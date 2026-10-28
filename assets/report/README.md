# BMT317 report

This directory holds the assignment PDF, the report sources, and the build
script. All tables and figures are generated from `assets/plots/results.csv` by
`scripts/analyze_results.py`; do not edit the generated files by hand.

## Contents

```text
assets/report/
├── BMT317_...pdf   # assignment
├── README.md       # this file
├── compile.sh      # compile the report
├── main.tex        # report source (placeholders at the top)
└── main.pdf        # generated output
```

## Using the generated assets

`assets/plots/` contains:

* `tables/tab_time.tex`, `tab_comparisons.tex`, `tab_moves.tex` — one table per
  metric, 28 rows (algorithm × distribution) × 6 sizes;
* `tables/tab_summary.tex` — theory vs measured growth exponent, auxiliary
  memory, stability;
* `tables/tab_environment.tex` — CPU, RAM, OS, compiler, seed, repetitions, run
  date;
* `figures/fig_*.pdf` (vector) and `.png` (preview);
* `tables.tex` and `figures.tex` — ready-to-use `\input` files.

Add these packages to the preamble:

```latex
\usepackage{booktabs}
\usepackage{graphicx}
```

Then include everything:

```latex
\input{../plots/tables.tex}
\input{../plots/figures.tex}
```

Or pick individual pieces:

```latex
\input{../plots/tables/tab_summary}
\includegraphics[width=\linewidth]{../plots/figures/fig_time_vs_n.pdf}
```

The paths are relative to this directory, so compile from `assets/report/`
(`compile.sh` does this automatically). Available labels:

* tables: `tab:time`, `tab:comparisons`, `tab:moves`, `tab:summary`,
  `tab:environment`
* figures: `fig:time`, `fig:comparisons`, `fig:moves`, `fig:distribution`

All captions and axis labels are in Turkish. They are defined in
`scripts/analyze_results.py` (constants at the top and the caption strings in
the rendering functions) if you want to change the wording.

`main.tex` is a complete report draft. Fill in the three placeholders at the top
of the file:

```latex
\newcommand{\OgrenciAdi}{[Adınız Soyadınız]}
\newcommand{\OgrenciNumarasi}{[Öğrenci Numaranız]}
\newcommand{\TeslimTarihi}{[.... / .... / ........]}
```

The section layout follows the assignment: Kapak, Özet, Giriş, Teorik Bilgi,
Algoritmaların Gerçekleştirilmesi, Deneysel Çalışma, Performans Sonuçları,
Karşılaştırma ve Tartışma, Sonuç ve Kaynakça. The measured values are already
written into the text; if you regenerate the results, check the numbers in
Section 5 and Section 6 for consistency.

## Generating the tables and figures

From the repository root:

```bash
python3 scripts/analyze_results.py \
    --csv assets/plots/results.csv \
    --outdir assets/plots
```

This needs Python 3 with numpy and matplotlib. It rewrites everything under
`assets/plots/tables/` and `assets/plots/figures/`, plus
`assets/plots/growth_exponents.csv` and the two include files.

To start from a fresh benchmark run:

```bash
./build-release/src/sortbench --reps 5 --warmup 1 --out assets/plots/results.csv
python3 scripts/analyze_results.py --csv assets/plots/results.csv --outdir assets/plots
```

The environment table takes its date from the CSV modification time, so a fresh
run updates it automatically. More detail: `assets/plots/README.md`.

## Compiling the report

```bash
./compile.sh                 # use main.tex (or the only .tex file here)
./compile.sh report.tex      # explicit file
./compile.sh --clean         # remove auxiliary files, keep the PDF
./compile.sh --clean-all     # remove auxiliary files and the PDF
```

`compile.sh` changes into this directory, checks that the plot includes exist,
and runs `latexmk -pdf` (or two `pdflatex` passes if `latexmk` is unavailable)
with `-halt-on-error`, so a failed build returns a non-zero exit code. The PDF
is written next to the source, e.g. `main.pdf`; the script prints its page count
when `pdfinfo` is installed.

Requirements: a TeX distribution with `pdflatex`, plus `latexmk` (recommended)
and `pdfinfo` (optional). The generated tables need `booktabs`; the figures need
`graphicx`.

## Note about version control

The repository `.gitignore` currently contains `/assets/report`, so this
directory (including the report sources and `compile.sh`) is ignored by git.
Remove that line if the report should be committed.
