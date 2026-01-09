import subprocess
import matplotlib.pyplot as plt
import numpy as np

# --- KONFIGURATION ---
# Note: Ensure the path is correct for your OS (Windows uses .exe)
EXECUTABLE = "./cmake-build-release/text-search-test.exe"
ITERATIONS = 10
QUERIES_FILE = "../common-words.txt"
DATA_DIR = "../data"
IMPLEMENTATIONS = [
    "candidate_v1", "candidate_v2", "candidate_v3", "candidate_v4",
    "hash_v1", "candidate_openmp_v1", "candidate_openmp_v2",
    "directComp_opencl_v1", "candidate_opencl_v2", "directComp_opencl_v3",
    "hash_openmp_v1"
]

def run_benchmark(impl):
    times = []
    print(f"Benchmarking {impl}: ", end="", flush=True)

    for i in range(ITERATIONS):
        cmd = [EXECUTABLE, "-i", impl, "-f", QUERIES_FILE, "-d", DATA_DIR, "--raw"]
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            time_ms = float(result.stdout.strip())
            times.append(time_ms)
            print(".", end="", flush=True) # Progress dots
        except Exception as e:
            print(f"\nFehler bei {impl}: {e}")
            return None, None

    avg_time = sum(times) / len(times)
    print(f" Done. Avg: {avg_time:.2f}ms")
    return avg_time, times

def main():
    avg_results = []
    all_runs = []
    labels = []

    for impl in IMPLEMENTATIONS:
        avg, times = run_benchmark(impl)
        if avg is not None:
            labels.append(impl)
            avg_results.append(avg)
            all_runs.append(times)

    if not labels:
        print("Keine Daten zum Plotten vorhanden.")
        return


    plt.figure(figsize=(14, 8))
    x_pos = np.arange(len(labels))

    bars = plt.bar(x_pos, avg_results, color='skyblue', alpha=0.5, label='Average Time', edgecolor='navy')

    for i in range(len(labels)):
        # We add a bit of random "jitter" to the X position so dots don't overlap perfectly
        x_jitter = np.random.normal(i, 0.05, size=len(all_runs[i]))
        plt.scatter(x_jitter, all_runs[i], color='crimson', alpha=0.6, s=20, zorder=3)

    plt.ylabel('Execution Time (ms)')
    plt.title(f'Performance Comparison: {ITERATIONS} Runs per Implementation')
    plt.xticks(x_pos, labels, rotation=45, ha='right')
    plt.legend(['Individual Runs', 'Average'])
    plt.grid(axis='y', linestyle='--', alpha=0.4)

    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval, f'{yval:.1f}',
                 va='bottom', ha='center', fontweight='bold')

    plt.tight_layout()
    plt.savefig('benchmark_detailed.png')
    print("\nPlot saved as 'benchmark_detailed.png'")
    plt.show()

if __name__ == "__main__":
    main()