import argparse
import os
import statistics
import subprocess
import platform

import matplotlib.pyplot as plt

SEQUENTIAL = ["std", "candidate_v1", "candidate_v2", "candidate_v3", "candidate_v4", "hash_v1", "hash_v2"]
OPENMP = ["std_openmp", "candidate_openmp_v1", "candidate_openmp_v2", "hash_openmp"]
MPI = ["candidate_mpi"]
OPENCL = ["candidate_opencl_v1", "candidate_opencl_v2", "candidate_opencl_v3"]

OUTPUT_DIR = "doc"
CSV_DIR = os.path.join(OUTPUT_DIR, "csv")
PLOT_DIR = os.path.join(OUTPUT_DIR, "plots")


def run_single(implementation, executable, args, book_dir, query_file, mpi_processes=0):
    command = []

    if mpi_processes > 0:
        command += ["mpiexec", "-n", str(mpi_processes)]

        if platform.system() == "Linux":
            command += ["--use-hwthread-cpus"]

    command += [
        executable,
        "-i", implementation,
        "-d", book_dir,
        "-f", query_file,
        "--raw",
        *args
    ]

    print(f"Running {' '.join(command)}")

    result = subprocess.run(
        command,
        capture_output=True,
        text=True,
        check=True
    )

    return int(result.stdout.strip())


def benchmark_implementation(implementation, executable, args, book_dir, query_file, amounts, mpi_processes=0):
    print(f"Benchmarking {implementation}...")

    results = []

    for n in amounts:
        times = []

        for _ in range(20):
            times.append(run_single(implementation, executable, args + [str(n)], book_dir, query_file, mpi_processes))

        results.append(statistics.median(times))

    return results


def write_csv(results, amounts, query_file, mode):
    print("Writing csvs...")

    for implementation, times in results.items():
        file_path = os.path.join(CSV_DIR, f"{implementation}-{mode}-{os.path.splitext(query_file)[0]}.csv")

        with open(file_path, "w", encoding="utf-8") as f:
            f.write(implementation + "\n")

            for time, amount in zip(times, amounts):
                f.write(str(amount) + "," + str(time) + "\n")


def plot(results, amounts, x_label, query_file, mode, implementation_group):
    print("Creating plots...")

    plt.figure(figsize=(12, 7))

    for implementation, times in results.items():
        plt.plot(amounts, times, marker="o", label=implementation)

    plt.xlabel(x_label)
    plt.ylabel("ms")
    plt.grid(True, which="both", linestyle="--", alpha=0.6)
    plt.legend(fontsize=9)

    output_file = os.path.join(PLOT_DIR, f"{implementation_group}-{mode}-{os.path.splitext(query_file)[0]}.png")

    plt.tight_layout()
    plt.savefig(output_file)
    plt.close()

    print(f"Combined plot saved to {output_file}.")


def main():
    parser = argparse.ArgumentParser(description="Text Search Benchmarking")

    parser.add_argument(
        "-i", "--implementation",
        required=True,
        choices=["sequential", "openmp", "mpi", "opencl"],
        help="Implementation to use"
    )

    parser.add_argument(
        "-e", "--executable",
        required=True,
        help="Path to the executable (not validated)"
    )

    parser.add_argument(
        "-m", "--mode",
        required=True,
        choices=["queries", "books"],
        help="Mode: queries or books"
    )

    parser.add_argument(
        "-q", "--query-file",
        required=True,
        help="Query file (text file)"
    )

    parser.add_argument(
        "-b", "--book-dir",
        required=True,
        help="Book directory or book file"
    )

    parser.add_argument(
        "--book-amount",
        type=int,
        default=98,
        help="Maximum amount of books (default: 98)"
    )

    parser.add_argument(
        "--query-amount",
        type=int,
        default=100,
        help="Maximum amount of queries (default: 100)"
    )

    parser.add_argument(
        "--mpi-processes",
        type=int,
        default=0,
        help="Number of MPI processes (default: 0)"
    )

    args = parser.parse_args()

    os.makedirs(CSV_DIR, exist_ok=True)
    os.makedirs(PLOT_DIR, exist_ok=True)

    implementations = []

    match args.implementation:
        case "sequential":
            implementations = SEQUENTIAL
        case "openmp":
            implementations = OPENMP
        case "mpi":
            implementations = MPI
        case "opencl":
            implementations = OPENCL

    print(f"Benchmarking {", ".join(implementations)} and saving output to {OUTPUT_DIR}...")

    results = {}

    amounts = range(5, (args.query_amount if args.mode == "queries" else args.book_amount) + 1, 5)

    for implementation in implementations:
        results[implementation] = benchmark_implementation(implementation,
                                                           args.executable,
                                                           ["-n"] if args.mode == "queries" else ["-m"],
                                                           args.book_dir,
                                                           args.query_file,
                                                           amounts,
                                                           args.mpi_processes)

    write_csv(results, amounts, args.query_file, args.mode)

    plot(results, amounts, "Queries" if args.mode == "queries" else "Books", args.query_file, args.mode,
         args.implementation)


if __name__ == "__main__":
    main()
