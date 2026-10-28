#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {
 
/// Swaps out of order elements next to each other. Also the
/// short circuit route ensures that an early finished sort does not
/// keep running.
void bubble_sort(std::vector<Value>& data, SortContext& ctx) {
    Metrics& metrics = ctx.metrics;

    for (std::size_t end = data.size(); end > 1; --end) {
        bool swapped = false;
        for (std::size_t j = 0; j + 1 < end; ++j) {
            if (less(data[j + 1], data[j], metrics)) {
                swap_values(data[j], data[j + 1], metrics);
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

} // namespace sortbench
