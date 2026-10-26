# SortBench

A cross-platform C++17 benchmark harness for my BMT317 - Algotithms class 
assignment about sorting algorithms.

It implements seven sorting algorithms from scratch, generates the
four required data distributions, measures time, comparison and
data-movement counts, verifies every result, and writes the results
as CSV.

## Features

The table below shows which of the assignment requirements can be found in this project.

| Assignment requirement | Where |
| --- | --- |
| 7 algorithms, implemented by hand | `src/algorithms/*_sort.cpp` |
| 4 data distributions (random, sorted, reverse, nearly sorted) | `src/core/dataset.cpp` |
| Sizes 100 … 100 000 | `src/cli/options.hpp` defaults |
| ≥ 5 repetitions, reports averages of them | `--reps`, `src/bench/runner.cpp` |
| Warm-up / first-run effects | `--warmup`, results discarded |
| Time, comparisons, swaps/moves | `core/types.hpp`, CSV columns |
| Auxiliary memory analysis | `--list`, `aux_memory`, `aux_bytes` |
| `isSorted(array)` check | `src/core/check.cpp` |
| Same dataset copy for every algorithm | `src/bench/runner.cpp` |
| Raw results as CSV | `io/csv_writer.cpp` |
| Tables and plots from a results CSV | `scripts/analyze_results.py`, `assets/plots/` |
| Shell gap sequence / Quick pivot documented | `--list`, README below |

## Dependencies

* A C++17 compiler: GCC ≥ 9, Clang ≥ 10, MSVC 2019+ (or MinGW-w64)
* Meson ≥ 1.1 and Ninja
* Python 3 (Meson dependency)
* Optional, only to regenerate tables and figures: numpy and matplotlib

## Build

Linux / macOS:

```bash
meson setup build --buildtype=release
meson compile -C build
meson test -C build --print-errorlogs
./build/src/sortbench --help
```

Windows (MSVC, from an *x64 Native Tools Command Prompt for VS*):

```bat
meson setup build --buildtype=release --vsenv
meson compile -C build
meson test -C build --print-errorlogs
build\src\sortbench.exe --help
```

Windows (MinGW-w64 / MSYS2) works with the Linux commands above. No
platform-specific headers or APIs are used; `std::chrono::steady_clock` maps to
`QueryPerformanceCounter` on Windows.

Build types:

* `debugoptimized` (default) — fast enough to iterate, keeps assertions.
* `release` — **use this for measurements**. `b_ndebug=if-release` removes
  assertions, and the compiler can fully optimise the algorithms.

Disable the test target with `meson setup build -Dbuild_tests=false` if needed.

## Usage

```text
sortbench [options]
```

| Option | Meaning | Default |
| --- | --- | --- |
| `-h`, `--help` | Show help | |
| `--list` | Print algorithms, complexity, memory class, gap/pivot notes | |
| `--algos LIST` | Comma-separated ids or `all` | `all` |
| `--dist LIST` | `random`, `sorted`, `reverse`, `nearly_sorted`, or `all` | `all` |
| `--sizes LIST` | Dataset sizes | `100,1000,5000,10000,50000,100000` |
| `--reps N` | Measured repetitions per case | `5` |
| `--warmup N` | Discarded runs per case | `1` |
| `--seed N` | Base seed for dataset generation | `42` |
| `--quadratic-limit N` | Skip O(n²) algorithms when n > N; `0` disables | `0` |
| `--out PATH` | CSV path, `-` for stdout | `results.csv` |
| `--quiet` | No progress output on stderr | off |
| `--dry-run` | Print the plan and exit | off |

Examples:

```bash
# Inspect the plan first (cheap).
./build/src/sortbench --dry-run

# Quick iteration while developing.
./build/src/sortbench --algos quick,merge,heap --sizes 1000,10000 --reps 3

# Full experiment; O(n^2) algorithms are skipped above n = 10 000.
mkdir -p results
./build/src/sortbench --quadratic-limit 10000 --out results/results.csv

# Stream CSV to stdout (progress stays on stderr).
./build/src/sortbench --quiet --out - > results.csv
```

Algorithms and ids: `selection`, `insertion`, `bubble`, `shell`, `quick`,
`merge`, `heap`.

Running the benchmark prints each of the dataset cases and the algorithms the user chose to test (all by default).
So if there are 24 different cases, we will have 24 x (algorithm count) results.

## Output CSV

```text
algorithm,distribution,n,repetitions,mean_time_ms,stddev_time_ms,
min_time_ms,max_time_ms,mean_comparisons,mean_moves,aux_memory,aux_bytes,verified
```

One row per algorithm × distribution × size. Times are milliseconds; comparison
and move columns are averages over the measured repetitions; `aux_memory` is a
complexity class (`O(1)`, `O(log n)`, `O(n)`) and `aux_bytes` is the analytic
estimate for that n. `verified = 1` means the result was sorted and identical to
a reference sort of the same input.

## Tables and figures

`assets/plots/` contains the generated tables and figures:

```bash
python3 scripts/analyze_results.py \
    --csv assets/plots/results.csv \
    --outdir assets/plots
```

It produces five `booktabs` tables (`tab_time`, `tab_comparisons`, `tab_moves`,
`tab_summary`, `tab_environment`), four figures as vector PDF + PNG
(`fig_time_vs_n`, `fig_comparisons_vs_n`, `fig_moves_vs_n`,
`fig_distribution_effect`), the measured growth exponents, and ready-to-use
`tables.tex` / `figures.tex` include files. See `assets/plots/README.md` for the
full file list and regeneration details.

## Measurement methodology

More information about the measurement strategy can be found in the report.

Each dataset case generates a seperate dataset. There are 4 distributions I implemented to be used here:

- `random`: random uniform distribution using `std::mt19937` engine.
- `sorted`: from 0 to n-1.
- `reversesorted`: from n-1 to 0.
- `nearlysorted`: we start with a sorted list and do n/10 moves to shift some values around.

For improving measurement accuracy, as it is required in the assignment, each dataset case runs 6 times by default (1 warmup + 5 for measurement). 
I used std::steady_timer for measurements related to time.

## Implementation notes

| Algorithm | Implementation choices |
| --- | --- |
| Selection | Minimum selection, swap only when needed. Exactly `n(n-1)/2` comparisons. |
| Insertion | Shifting, early exit on sorted input; stable. |
| Bubble | Early-exit flag; stable. |
| Shell | Gap sequence `n/2, n/4, …, 1` (original Shell sequence). |
| Quick | **Median-of-three** pivot (first/middle/last), Lomuto partition. |
| Merge | Top-down, single shared scratch buffer, stable. |
| Heap | Uses a max heap. In-place. |

I have hand-written all of the sorting algorithms to satisfy the requirements of my assignment.

## Project layout

```text
sortbench/
├── meson.build              # project + cross-platform flags
├── meson_options.txt
├── src/
│   ├── main.cpp             # CLI entry point, banner, CSV output
│   ├── core/
│   │   ├── types.hpp        # Value, Metrics, counted less/swap helpers
│   │   ├── dataset.{hpp,cpp}# 4 distributions, deterministic seeds
│   │   ├── check.{hpp,cpp}  # is_sorted()
│   │   └── stopwatch.hpp    # steady_clock wrapper
│   ├── algorithms/
│   │   ├── algorithm.hpp    # SortContext, Algorithm descriptor, registry API
│   │   ├── complexity.cpp   # complexity/memory enum text + estimates
│   │   ├── registry.cpp     # the 7 algorithms, in assignment order
│   │   └── *_sort.cpp       # one translation unit per algorithm
│   ├── cli/options.{hpp,cpp}
│   ├── bench/runner.{hpp,cpp}  # plan, dataset loop, warm-up, aggregation
│   └── io/csv_writer.{hpp,cpp}
├── scripts/
│   └── analyze_results.py     # CSV -> LaTeX tables + PDF/PNG figures
├── assets/
│   └── plots/                 # results.csv, tables/, figures/ (generated)
└── tests/
    ├── meson.build
    └── test_sorting.cpp     # correctness, counters, parser, end-to-end run
```

## Tests

```bash
meson test -C build --print-errorlogs
# or directly for the full output:
./build/tests/sortbench_tests
```

The suite covers:

* all 7 algorithms × 4 distributions × sizes 0…1023 (including 0, 1, 2 and
  duplicate-heavy inputs) against a reference sort;
* a randomized differential fuzz loop (100 arrays per algorithm, small value
  ranges to force many duplicates);
* `is_sorted` edge cases and dataset-generator properties (ordering,
  determinism, value range, ~10 % displacement);
* exact counter expectations (e.g. Selection Sort = `n(n-1)/2` comparisons,
  Insertion Sort = 0 moves on sorted input, Merge Sort move counts);
* CLI parsing (inline `--opt=value`, invalid values, unknown options, flags
  with values), plan resolution/deduplication and the quadratic limit;
* an end-to-end 4-case benchmark run with CSV row invariants.

## License

Released under the MIT License. See [LICENSE](LICENSE) for the full text.
