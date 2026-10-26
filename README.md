# SortBench

A cross-platform C++17 benchmark harness for the BMT317 sorting-algorithm
assignment. It implements seven sorting algorithms from scratch, generates the
four required data distributions, measures wall-clock time plus comparison and
data-movement counts, verifies every result, and writes the aggregated results
as CSV.

> **Status:** the benchmark core, all seven algorithms, the CSV pipeline, tests,
> CLI and the table/figure generator are complete.

## Assignment coverage

| Assignment requirement | Where |
| --- | --- |
| 7 algorithms, no library sort | `src/algorithms/*_sort.cpp` |
| 4 data distributions (random, sorted, reverse, nearly sorted) | `src/core/dataset.cpp` |
| Sizes 100 … 100 000 | `src/cli/options.hpp` defaults |
| ≥ 5 repetitions, average reported | `--reps`, `src/bench/runner.cpp` |
| Warm-up / first-run effects | `--warmup`, results discarded |
| Time, comparisons, swaps/moves | `core/types.hpp`, CSV columns |
| Auxiliary memory analysis | `--list`, `aux_memory`, `aux_bytes` |
| `isSorted(array)` check | `src/core/check.cpp` |
| Same dataset copy for every algorithm | `src/bench/runner.cpp` |
| Raw results as CSV | `io/csv_writer.cpp` |
| Tables and plots from a results CSV | `scripts/analyze_results.py`, `assets/plots/` |
| Shell gap sequence / Quick pivot documented | `--list`, README below |

## Requirements

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

The full default matrix is 7 algorithms × 4 distributions × 6 sizes × 5
repetitions = 840 measured runs plus warm-ups. The O(n²) algorithms dominate the
runtime at large n; use `--quadratic-limit` and document the choice, as the
assignment explicitly allows.

Progress lines on stderr count **dataset cases**, i.e. one line per
(distribution, size) pair: `[4/24] sorted n=10000 (7 algorithms x 5 reps)`.
Each finished case produces one CSV row per algorithm, so the default matrix has
24 progress lines and 168 CSV rows.

Exit codes: `0` success, `1` nothing to run, `2` invalid input / I/O error,
`3` a result failed correctness verification.

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

**Datasets.** Every case uses a deterministic generator:

* *random*: uniform values in `[0, n)` from `std::mt19937_64`;
* *sorted*: `0, 1, …, n-1`;
* *reverse*: `n-1, …, 1, 0`;
* *nearly_sorted*: ascending, then `n/10` random pair swaps.

Each repetition derives its own seed (`seed → splitmix64(distribution, n, rep)`),
so runs are reproducible from `--seed` but different repetitions are not
identical clones. Results are averaged over repetitions.

**Fairness.** Within a repetition the base dataset is generated once; each
algorithm receives a fresh copy, and the copy is made *outside* the timed
region. No algorithm ever sorts another algorithm's output.

**Timing.** Only the `algorithm.sort(data, ctx)` call is inside
`std::chrono::steady_clock`. Dataset generation, validation, and the scratch
buffer allocation are outside. The scratch buffer (`SortContext::buffer`) is
pre-sized before timing so allocator noise does not distort Merge Sort.

**Warm-up.** `--warmup 1` runs each algorithm once per case on the same
distribution and discards the result; this reduces first-run page-fault and
cache effects.

**Counters.**

* `comparisons` — one increment per element-to-element comparison.
* `moves` — one increment per element assignment; a swap counts as 3 moves
  (the textbook convention). This is the "swap / data movement" column.

Counters are accumulated by the algorithms themselves through the `Metrics`
helpers, so they are exact (not sampled). All repetitions of a fixed dataset
size produce identical counter values unless the data generator changes; the
CSV stores the average so the format stays uniform.

**Memory.** Auxiliary memory is *not* measured at runtime (portable RSS
measurement is unreliable). Each algorithm declares its auxiliary-memory class,
and `aux_bytes` gives a documented estimate:

* Selection / Insertion / Bubble / Shell / Heap — `O(1)`, 0 bytes;
* Merge — one `n`-element scratch buffer (`n * sizeof(Value)`);
* Quick — expected `O(log n)` recursion stack (smaller-part recursion).

Report these as analytic values and explain them; do not present `aux_bytes` as
an OS measurement.

**Correctness.** Every measured run is validated with `is_sorted()` (the function
required by section 14) and by comparing against a reference sort of the same
input produced by the project's own merge sort. A mismatch is printed on stderr
and marks the row `verified = 0`; the process then exits with code 3. The
reference sort is never timed.

**Caveats when interpreting the results.** Wall-clock time includes cache and
memory effects; no CPU pinning or frequency scaling control is applied. Run on
an otherwise idle machine, use a release build, and prefer the median-spread
information (`stddev`, `min`, `max`) when discussing small differences.

## Implementation notes

| Algorithm | Implementation choices |
| --- | --- |
| Selection | Minimum selection, swap only when needed. Exactly `n(n-1)/2` comparisons. |
| Insertion | Shifting, early exit on sorted input; stable. |
| Bubble | Early-exit flag; stable. |
| Shell | Gap sequence `n/2, n/4, …, 1` (original Shell sequence). |
| Quick | **Median-of-three** pivot (first/middle/last), Lomuto partition, hand-rolled tail recursion on the larger partition so the stack is `O(log n)`. |
| Merge | Top-down, single shared scratch buffer, stable. |
| Heap | Binary max-heap: bottom-up build, then extract-max. In-place. |

All algorithms are hand-written; no `std::sort`, `std::stable_sort`,
`std::partial_sort`, `qsort`, or library heap helpers are used. `std::swap` is
only used by the dataset generator, not by the algorithms.

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

## Extending

**Add an algorithm:** write `src/algorithms/foo_sort.cpp` implementing
`void foo_sort(std::vector<Value>&, SortContext&)`; declare it in
`algorithm.hpp`; add it to the registry and the source list in
`src/meson.build`; add a case to the test matrix (the generic loop picks it up
automatically).

**Bonus 1 — Quick Sort pivot comparison:** the pivot choice is isolated in
`quick_sort_range`. Add a second entry (e.g. `quick_first`) that uses the first
element, or select the pivot from `SortContext::rng_state` for a randomized
variant, then run `--algos quick,quick_first`.

**Bonus 2 — Shell gap comparison:** make the gap sequence a parameter and
register `shell_knuth` (`1, 4, 13, 40, …`) alongside the original sequence;
compare with `--algos shell,shell_knuth`.

**Bonus 3+** (large n, parallel merge, visualization): not implemented in the
skeleton; the runner and metric types are ready for a parallel variant.

## Next steps

1. If per-run variance is needed rather than aggregates, add a `--raw PATH`
   option that writes one row per repetition.
2. Explore the optional bonus items in [Extending](#extending).

## Notes

* The project deliberately keeps all identifiers, column names and output in
  ASCII so CSV files and console output behave identically on Windows and POSIX.
* Values are `int32`, so dataset sizes must stay below 2³¹ (the assignment uses
  at most 100 000; `--sizes` is capped at 2³⁰).
