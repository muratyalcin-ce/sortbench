#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {

/// Insertion Sort: grows a sorted prefix by inserting the next element into its
/// place. Shifting instead of swapping keeps the move count low and makes the
/// algorithm linear on already sorted input.
void insertion_sort(std::vector<Value>& data, SortContext& ctx) {
    Metrics& metrics = ctx.metrics;
    const std::size_t n = data.size();

    for (std::size_t i = 1; i < n; ++i) {
        const Value key = data[i];
        std::size_t j = i;

        while (j > 0 && less(key, data[j - 1], metrics)) {
            data[j] = data[j - 1];
            ++metrics.moves;
            --j;
        }
        if (j != i) {
            data[j] = key;
            ++metrics.moves;
        }
    }
}

} // namespace sortbench
