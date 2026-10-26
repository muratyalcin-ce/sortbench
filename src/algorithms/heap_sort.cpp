#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {
namespace {

/// Restores the max-heap property for the subtree rooted at `root`, where the
/// heap occupies data[0, count).
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

/// Heap Sort: build a max-heap in O(n), then repeatedly swap the maximum to the
/// end and shrink the heap. Fully in-place.
void heap_sort(std::vector<Value>& data, SortContext& ctx) {
    Metrics& metrics = ctx.metrics;
    const std::size_t n = data.size();

    // Build phase: sift down every internal node, bottom-up.
    for (std::size_t i = n / 2; i-- > 0;) {
        sift_down(data, i, n, metrics);
    }

    // Extract phase: move the current maximum behind the heap.
    for (std::size_t end = n; end-- > 1;) {
        swap_values(data[0], data[end], metrics);
        sift_down(data, 0, end, metrics);
    }
}

} // namespace sortbench
