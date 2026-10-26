# Tables and figures

LaTeX tables and figures generated from a SortBench results CSV.

**Source data:** `results.csv` is the completed benchmark run (7 algorithms × 4
distributions × 6 sizes × 5 repetitions = 168 rows, seed 42, all rows verified).
It was copied from the run at the repository root; its original timestamp
(2026-10-24 17:59) is preserved so the environment table records the actual run
date.

## Contents

```text
assets/plots/
├── results.csv                 # aggregated benchmark data (input for everything)
├── growth_exponents.csv        # measured log-log slopes, all (algorithm, distribution)
├── tables/
│   ├── tab_time.tex            # mean runtime (ms), algorithm × distribution × n
│   ├── tab_comparisons.tex     # mean comparison counts
│   ├── tab_moves.tex           # mean data movements (swap = 3 moves)
│   ├── tab_summary.tex         # theory, auxiliary memory, measured exponent b, stability
│   └── tab_environment.tex     # CPU, RAM, OS, compiler, seed, repetitions, run date
├── figures/
│   ├── fig_time_vs_n.pdf/.png
│   ├── fig_comparisons_vs_n.pdf/.png
│   ├── fig_moves_vs_n.pdf/.png
│   └── fig_distribution_effect.pdf/.png
├── figures.tex                 # ready-to-use figure environments
└── tables.tex                  # inputs all five tables
```

`tab_time.tex` and the other two metric tables have 28 rows each (algorithm ×
distribution) and six columns (one per dataset size), so all measured data is
included without a 168-row table. See `assets/report/README.md` for how to
include the files in the report.

## Regenerating

```bash
# from the repository root
python3 scripts/analyze_results.py \
    --csv assets/plots/results.csv \
    --outdir assets/plots
```

Python 3 with numpy and matplotlib is required. If you rerun the benchmark,
overwrite `assets/plots/results.csv` first (or pass `--csv`); the environment
table's date comes from the CSV modification time.

Done after a fresh run:

```bash
./build-release/src/sortbench --reps 5 --warmup 1 --out assets/plots/results.csv
python3 scripts/analyze_results.py --csv assets/plots/results.csv --outdir assets/plots
```

## Notes

* Tables require the `booktabs` package; no `multirow`, `siunitx` or other extra
  packages are needed.
* Figures are vector PDFs with embedded TrueType fonts (no Type 3) and 200 dpi
  PNG previews. Both axes of the "vs n" figures are logarithmic; the movement
  figure uses a symmetric log y-axis so that zero-move runs remain visible.
* `growth_exponents.csv` contains the slope `b` of `log(time)` vs `log(n)` for
  every algorithm and distribution. Values near 2 indicate quadratic growth,
  values near 1.1 indicate `n log n` growth (for `n ≥ 1000`).
* Captions and axis labels are Turkish. They are defined as constants at the top
  of `scripts/analyze_results.py` (and in the table strings) if you want to
  change the wording.
* The generated files were validated with `pdflatex` (A4, 6 pages, no overfull or
  underfull boxes).
