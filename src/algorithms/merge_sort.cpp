#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {
namespace {

/// Merges the two sorted halves data[lo, mid) and data[mid, hi) into the shared
/// scratch buffer, then copies the merged range back. Taking the right element
/// only when it is strictly smaller keeps the algorithm stable.
void merge_ranges(std::vector<Value>& data,
                  std::vector<Value>& buffer,
                  std::size_t lo,
                  std::size_t mid,
                  std::size_t hi,
                  Metrics& metrics) {
    std::size_t i = lo;
    std::size_t j = mid;
    std::size_t k = lo;

    while (i < mid && j < hi) {
        if (less(data[j], data[i], metrics)) {
            buffer[k++] = data[j++];
        } else {
            buffer[k++] = data[i++];
        }
        ++metrics.moves;
    }
    while (i < mid) {
        buffer[k++] = data[i++];
        ++metrics.moves;
    }
    while (j < hi) {
        buffer[k++] = data[j++];
        ++metrics.moves;
    }
    for (std::size_t t = lo; t < hi; ++t) {
        data[t] = buffer[t];
        ++metrics.moves;
    }
}

void merge_sort_range(std::vector<Value>& data,
                      std::vector<Value>& buffer,
                      std::size_t lo,
                      std::size_t hi,
                      Metrics& metrics) {
    if (hi - lo < 2) {
        return;
    }
    const std::size_t mid = lo + (hi - lo) / 2;
    merge_sort_range(data, buffer, lo, mid, metrics);
    merge_sort_range(data, buffer, mid, hi, metrics);
    merge_ranges(data, buffer, lo, mid, hi, metrics);
}

} // namespace

/// Top-down Merge Sort using SortContext::buffer as the only auxiliary array.
void merge_sort(std::vector<Value>& data, SortContext& ctx) {
    if (data.size() < 2) {
        return;
    }
    if (ctx.buffer.size() < data.size()) {
        ctx.buffer.resize(data.size());
    }
    merge_sort_range(data, ctx.buffer, 0, data.size(), ctx.metrics);
}

} // namespace sortbench
