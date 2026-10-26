#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {
namespace {

Value& at(std::vector<Value>& data, std::ptrdiff_t index) {
    return data[static_cast<std::size_t>(index)];
}

/// Recursive part of Quick Sort.
///
/// Pivot strategy: median-of-three (first, middle, last element). The median is
/// moved to the last position and partitioning is Lomuto's scheme.
///
/// To guarantee an O(log n) recursion stack, only the smaller partition is
/// handled recursively; the larger one is handled by the loop (tail-call
/// elimination by hand).
void quick_sort_range(std::vector<Value>& data,
                      std::ptrdiff_t lo,
                      std::ptrdiff_t hi,
                      Metrics& metrics) {
    while (lo < hi) {
        // Median-of-three needs at least three elements. For two elements a
        // single comparison is enough (and mid would alias lo).
        if (hi - lo == 1) {
            if (less(at(data, hi), at(data, lo), metrics)) {
                swap_values(at(data, lo), at(data, hi), metrics);
            }
            return;
        }

        const std::ptrdiff_t mid = lo + (hi - lo) / 2;

        // Sort data[lo] <= data[mid] <= data[hi].
        if (less(at(data, mid), at(data, lo), metrics)) {
            swap_values(at(data, mid), at(data, lo), metrics);
        }
        if (less(at(data, hi), at(data, lo), metrics)) {
            swap_values(at(data, hi), at(data, lo), metrics);
        }
        if (less(at(data, hi), at(data, mid), metrics)) {
            swap_values(at(data, hi), at(data, mid), metrics);
        }

        // Move the median (data[mid]) to the end and use it as pivot.
        swap_values(at(data, mid), at(data, hi), metrics);
        const Value pivot = at(data, hi);

        std::ptrdiff_t store = lo;
        for (std::ptrdiff_t j = lo; j < hi; ++j) {
            if (less(at(data, j), pivot, metrics)) {
                if (store != j) {
                    swap_values(at(data, store), at(data, j), metrics);
                }
                ++store;
            }
        }
        swap_values(at(data, store), at(data, hi), metrics);

        const std::ptrdiff_t left_size = store - lo;
        const std::ptrdiff_t right_size = hi - store;

        if (left_size < right_size) {
            if (left_size > 0) {
                quick_sort_range(data, lo, store - 1, metrics);
            }
            lo = store + 1;
        } else {
            if (right_size > 0) {
                quick_sort_range(data, store + 1, hi, metrics);
            }
            hi = store - 1;
        }
    }
}

} // namespace

void quick_sort(std::vector<Value>& data, SortContext& ctx) {
    if (data.size() < 2) {
        return;
    }
    quick_sort_range(data, 0, static_cast<std::ptrdiff_t>(data.size()) - 1, ctx.metrics);
}

} // namespace sortbench
