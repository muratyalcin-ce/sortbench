#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {

/// Selection Sort: repeatedly select the minimum of the unsorted suffix and
/// move it to the front. The number of comparisons is always n(n-1)/2, which
/// makes it a useful lower bound for comparison-count measurements.
void selection_sort(std::vector<Value>& data, SortContext& ctx) {
    Metrics& metrics = ctx.metrics;
    const std::size_t n = data.size();

    for (std::size_t i = 0; i + 1 < n; ++i) {
        std::size_t min_index = i;
        for (std::size_t j = i + 1; j < n; ++j) {
            if (less(data[j], data[min_index], metrics)) {
                min_index = j;
            }
        }
        if (min_index != i) {
            swap_values(data[i], data[min_index], metrics);
        }
    }
}

} // namespace sortbench
