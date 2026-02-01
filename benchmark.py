import argparse
import os
import platform
import statistics
import subprocess

import matplotlib.pyplot as plt

SEQUENTIAL = ["std", "candidate_v1", "candidate_v2", "candidate_v3", "candidate_v4", "hash_v1", "hash_v2"]
OPENMP = ["std_openmp", "candidate_openmp_v1", "candidate_openmp_v2", "hash_openmp"]
MPI = ["candidate_mpi"]
OPENCL = ["candidate_opencl_v1", "candidate_opencl_v2", "candidate_opencl_v3"]

OUTPUT_DIR = "doc"
CSV_DIR = os.path.join(OUTPUT_DIR, "csv")
PLOT_DIR = os.path.join(OUTPUT_DIR, "plots")

MAPPINGS = {
    "candidate_v3": ["candidate_openmp_v1", "candidate_mpi"],
    "candidate_v4": ["candidate_openmp_v2", "candidate_opencl_v1", "candidate_opencl_v2", "candidate_opencl_v3"],
    "std": ["std_openmp"],
    "hash_v2": ["hash_openmp"]
}

MODES = ["queries", "books"]


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


def get_csv_file_path(implementation, mode, query_file):
    return os.path.join(CSV_DIR, f"{implementation}-{mode}-{os.path.splitext(query_file)[0]}.csv")


def write_csv(results, amounts, query_file, mode):
    print("Writing csvs...")

    for implementation, times in results.items():
        with open(get_csv_file_path(implementation, mode, query_file), "w", encoding="utf-8") as f:
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


def parse_csv(file):
    times = []

    with open(file) as f:
        next(f)
        for line in f:
            line = line.strip()

            if not line:
                continue

            parts = line.split(",")

            if len(parts) < 2:
                continue

            times.append(float(parts[1]))

    return times


def compare(query_files):
    for sequential_implementation, parallel_implementations in MAPPINGS.items():
        all_speedups = {p: [] for p in parallel_implementations}

        for mode in MODES:
            for query_file in query_files:
                sequential_times = parse_csv(get_csv_file_path(sequential_implementation, mode, query_file))

                for parallel_implementation in parallel_implementations:
                    parallel_times = parse_csv(get_csv_file_path(parallel_implementation, mode, query_file))

                    speedups = [s / p for s, p in zip(sequential_times, parallel_times)]

                    all_speedups[parallel_implementation].extend(speedups)

        for parallel_implementation, speedup_list in all_speedups.items():
            print(
                f"{sequential_implementation} -> {parallel_implementation}: {(statistics.mean(speedup_list) - 1) * 100}%")


def main():
    parser = argparse.ArgumentParser(description="Text Search Benchmarking")

    subparsers = parser.add_subparsers(dest="command", required=True)

    benchmark_parser = subparsers.add_parser(
        "benchmark",
        help="Run benchmarks and generate CSVs and plots"
    )

    benchmark_parser.add_argument(
        "-i", "--implementation",
        required=True,
        choices=["sequential", "openmp", "mpi", "opencl"],
        help="Implementation to use"
    )

    benchmark_parser.add_argument(
        "-e", "--executable",
        required=True,
        help="Path to the executable (not validated)"
    )

    benchmark_parser.add_argument(
        "-m", "--mode",
        required=True,
        choices=MODES,
        help="Mode: queries or books"
    )

    benchmark_parser.add_argument(
        "-q", "--query-file",
        required=True,
        help="Query file"
    )

    benchmark_parser.add_argument(
        "-b", "--book-dir",
        required=True,
        help="Book directory or book file"
    )

    benchmark_parser.add_argument(
        "--book-amount",
        type=int,
        default=98,
        help="Maximum amount of books (default: 98)"
    )

    benchmark_parser.add_argument(
        "--query-amount",
        type=int,
        default=100,
        help="Maximum amount of queries (default: 100)"
    )

    benchmark_parser.add_argument(
        "--mpi-processes",
        type=int,
        default=0,
        help="Number of MPI processes (default: 0)"
    )

    compare_parser = subparsers.add_parser(
        "compare",
        help="Show speed-up factors"
    )

    compare_parser.add_argument(
        "-q", "--query-file",
        action="append",
        required=True,
        help="Query files"
    )

    args = parser.parse_args()

    os.makedirs(CSV_DIR, exist_ok=True)
    os.makedirs(PLOT_DIR, exist_ok=True)

    if args.command == "benchmark":
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
    else:
        compare(args.query_file)


if __name__ == "__main__":
    main()
