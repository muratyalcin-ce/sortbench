#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {

/// n/2, n/4, n/8... sequence
void shell_sort(std::vector<Value>& data, SortContext& ctx) {
    Metrics& metrics = ctx.metrics;
    const std::size_t n = data.size();

    for (std::size_t gap = n / 2; gap > 0; gap /= 2) {
        for (std::size_t i = gap; i < n; ++i) {
            const Value key = data[i];
            std::size_t j = i;

            while (j >= gap && less(key, data[j - gap], metrics)) {
                data[j] = data[j - gap];
                ++metrics.moves;
                j -= gap;
            }
            if (j != i) {
                data[j] = key;
                ++metrics.moves;
            }
        }
    }
}

} // namespace sortbench
