#!/usr/bin/env bash
#
# Compile the BMT317 report.
#
#   ./compile.sh                 build main.tex (or the only .tex file here)
#   ./compile.sh report.tex      build an explicit file
#   ./compile.sh --clean         remove auxiliary files, keep the PDF
#   ./compile.sh --clean-all     remove auxiliary files and the PDF
#
# The script changes into its own directory first, so the relative includes
# (../plots/tables.tex, ../plots/figures.tex) always resolve.
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: ./compile.sh [options] [file.tex]

Options:
  -h, --help       Show this help and exit
  -c, --clean      Remove auxiliary files, keep the PDF
  -C, --clean-all  Remove auxiliary files and the PDF

Without a file argument, main.tex is used; if it does not exist and there is
exactly one .tex file in this directory, that file is used.
EOF
}

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"

mode="build"
root=""

for argument in "$@"; do
    case "$argument" in
        -h|--help)
            usage
            exit 0
            ;;
        -c|--clean)
            mode="clean"
            ;;
        -C|--clean-all)
            mode="clean-all"
            ;;
        -*)
            echo "error: unknown option '$argument'" >&2
            usage >&2
            exit 2
            ;;
        *)
            if [[ -n "$root" ]]; then
                echo "error: more than one input file given ('$root' and '$argument')" >&2
                exit 2
            fi
            root="$argument"
            ;;
    esac
done

# Resolve the source file.
if [[ -z "$root" ]]; then
    if [[ -f main.tex ]]; then
        root="main.tex"
    else
        shopt -s nullglob
        tex_files=(*.tex)
        shopt -u nullglob
        case "${#tex_files[@]}" in
            0)
                echo "error: no .tex file in $script_dir" >&2
                echo "       create main.tex or pass a file explicitly" >&2
                exit 2
                ;;
            1)
                root="${tex_files[0]}"
                ;;
            *)
                echo "error: multiple .tex files found; pass one explicitly:" >&2
                printf '       %s\n' "${tex_files[@]}" >&2
                exit 2
                ;;
        esac
    fi
fi

if [[ ! -f "$root" ]]; then
    echo "error: '$root' not found in $script_dir" >&2
    exit 2
fi

pdf="${root%.tex}.pdf"
base="${root%.tex}"

remove_aux() {
    rm -f \
        "$base.aux" "$base.log" "$base.out" "$base.toc" "$base.fls" \
        "$base.fdb_latexmk" "$base.synctex.gz" "$base.bbl" "$base.blg"
}

if [[ "$mode" != "build" ]]; then
    if command -v latexmk >/dev/null 2>&1; then
        if [[ "$mode" == "clean-all" ]]; then
            latexmk -C "$root" >/dev/null 2>&1 || true
        else
            latexmk -c "$root" >/dev/null 2>&1 || true
        fi
    fi
    remove_aux
    if [[ "$mode" == "clean-all" ]]; then
        rm -f "$pdf"
        echo "removed auxiliary files and $pdf"
    else
        echo "removed auxiliary files (kept $pdf)"
    fi
    exit 0
fi

# The table/figure include files are generated; warn but do not block compilation.
for asset in ../plots/tables.tex ../plots/figures.tex; do
    if [[ ! -f "$asset" ]]; then
        echo "warning: $asset is missing; run scripts/analyze_results.py first" >&2
    fi
done

if command -v latexmk >/dev/null 2>&1; then
    echo "==> latexmk -pdf $root"
    latexmk -pdf -interaction=nonstopmode -halt-on-error -file-line-error "$root"
elif command -v pdflatex >/dev/null 2>&1; then
    echo "==> pdflatex (twice; latexmk not found)"
    pdflatex -interaction=nonstopmode -halt-on-error -file-line-error "$root"
    pdflatex -interaction=nonstopmode -halt-on-error -file-line-error "$root"
else
    echo "error: neither latexmk nor pdflatex is installed" >&2
    exit 2
fi

if [[ ! -f "$pdf" ]]; then
    echo "error: compilation finished but $pdf was not produced" >&2
    exit 1
fi

if command -v pdfinfo >/dev/null 2>&1; then
    pages="$(pdfinfo "$pdf" | awk '/^Pages:/ {print $2}')"
    echo "OK: $script_dir/$pdf (${pages:-?} pages)"
else
    echo "OK: $script_dir/$pdf"
fi
