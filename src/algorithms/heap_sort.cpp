#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {
namespace {

/// Heapify/Sift-Down operation, this restores the max-heap property of our vector
void sift_down(std::vector<Value>& data, std::size_t root, std::size_t count, Metrics& metrics) {
    for (;;) {
        const std::size_t left = 2 * root + 1;
        if (left >= count) {
            return;
        }

        std::size_t largest = left;
        const std::size_t right = left + 1;
        if (right < count && less(data[left], data[right], metrics)) {
            largest = right;
        }
        if (!less(data[root], data[largest], metrics)) {
            return;
        }
        swap_values(data[root], data[largest], metrics);
        root = largest;
    }
}

} // namespace

/// Heap sort builds a max-heap in O(n), then repeatedly swaps the maximum to the
/// end and shrinks the heap. Fully an in-place algorithm.
void heap_sort(std::vector<Value>& data, SortContext& ctx) {
    Metrics& metrics = ctx.metrics;
    const std::size_t n = data.size();

    // Heapify	
    for (std::size_t i = n / 2; i-- > 0;) {
        sift_down(data, i, n, metrics);
    }

    // Extract
    for (std::size_t end = n; end-- > 1;) {
        swap_values(data[0], data[end], metrics);
        sift_down(data, 0, end, metrics);
    }
}

} // namespace sortbench
