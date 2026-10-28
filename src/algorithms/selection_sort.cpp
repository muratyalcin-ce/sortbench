#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {

/// Finds the minimum and moves it to the front 
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
