import subprocess
import csv
import statistics
import matplotlib.pyplot as plt
from pathlib import Path

# ================== KONFIGURATION ==================

EXECUTABLE = "./cmake-build-release/text-search-test.exe"
QUERIES_FILE = "common-words.txt"
DATA_DIR = "data"

ITERATIONS = 3                      # Läufe pro n
QUERY_COUNTS = list(range(5, 101, 5))  # 5,10,...,100

IMPLEMENTATIONS = [
    "candidate_v1", "candidate_v2", "candidate_v3", "candidate_v4",
    "candidate_openmp_v1", "candidate_openmp_v2","directComp_opencl_v1", "candidate_opencl_v2",
    "directComp_opencl_v3", "hash_v1", "hash_v2", "hash_openmp_v1"]

OUTPUT_DIR = Path("benchmark_results")
OUTPUT_DIR.mkdir(exist_ok=True)

ALL_MEDIANS = {}  # Speichert Median-Kurven für den Plot

ALL_ROWS = []     # Alle Daten für die gemeinsame CSV

# ===================================================

def run_single(impl, n_queries):
    cmd = [
        EXECUTABLE,
        "-i", impl,
        "-f", QUERIES_FILE,
        "-d", DATA_DIR,
        "-n", str(n_queries),
        "--raw"
    ]

    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        check=True
    )

    return float(result.stdout.strip())


def benchmark_implementation(impl):
    print(f"\n=== Benchmarking {impl} ===")

    rows = []

    for n in QUERY_COUNTS:
        times = []

        for run in range(ITERATIONS):
            print(f"[{impl}] n={n} (run {run+1}/{ITERATIONS})", flush=True)
            t = run_single(impl, n)
            times.append(t)

        median_time = statistics.median(times)
        median_time_s = median_time / 1000

        row = {
            "implementation": impl,
            "n_queries": n,
            "median_s": median_time_s
        }
        for idx, t in enumerate(times, start=1):
            row[f"run_{idx}_ms"] = t / 1000

        rows.append(row)
        ALL_ROWS.append(row)

    # Für den Gesamtplot speichern
    ALL_MEDIANS[impl] = [(row["n_queries"], row["median_s"]) for row in rows]


def write_csv():
    out_file = OUTPUT_DIR / "benchmark_all.csv"
    print(f"\n→ Writing combined CSV: {out_file}")

    # Feldnamen dynamisch für ITERATIONS
    fieldnames = ["implementation", "n_queries"]
    for i in range(1, ITERATIONS+1):
        fieldnames.append(f"run_{i}_ms")
    fieldnames.append("median_s")

    with out_file.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(ALL_ROWS)


def plot_all_models():
    plt.figure(figsize=(12, 7))

    for impl, data in ALL_MEDIANS.items():
        n_vals = [n for n, _ in data]
        medians = [m for _, m in data]
        plt.plot(n_vals, medians, marker="o", label=impl)

    plt.xlabel("Number of Queries (n)")
    plt.ylabel("Median Execution Time (ms)")
    plt.title("Median Runtime vs. Queries (All Implementations)")
    plt.grid(True, which="both", linestyle="--", alpha=0.6)
    plt.legend(fontsize=9)

    out_file = OUTPUT_DIR / "benchmark_all_models.png"
    plt.tight_layout()
    plt.savefig(out_file)
    plt.close()
    print(f"📊 Combined plot saved to {out_file}")


def main():
    for impl in IMPLEMENTATIONS:
        benchmark_implementation(impl)

    write_csv()
    plot_all_models()
    print("\n✅ All benchmarks completed.")


if __name__ == "__main__":
    main()
