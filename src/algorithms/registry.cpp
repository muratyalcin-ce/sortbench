#include "algorithms/algorithm.hpp"

namespace sortbench {

const std::vector<Algorithm>& all_algorithms() {
    static const std::vector<Algorithm> algorithms{
        Algorithm{
            "selection", "Selection Sort", Complexity::Quadratic, AuxMemory::Constant,
            &selection_sort,
            "in-place, not stable; always n(n-1)/2 comparisons"},
        Algorithm{
            "insertion", "Insertion Sort", Complexity::Quadratic, AuxMemory::Constant,
            &insertion_sort,
            "in-place, stable; O(n) on already sorted data"},
        Algorithm{
            "bubble", "Bubble Sort", Complexity::Quadratic, AuxMemory::Constant,
            &bubble_sort,
            "in-place, stable; early exit when a pass performs no swap"},
        Algorithm{
            "shell", "Shell Sort", Complexity::Depends, AuxMemory::Constant,
            &shell_sort,
            "in-place, not stable; gap sequence n/2, n/4, ..., 1"},
        Algorithm{
            "quick", "Quick Sort", Complexity::Linearithmic, AuxMemory::Logarithmic,
            &quick_sort,
            "in-place, not stable; median-of-three pivot, tail recursion on larger part"},
        Algorithm{
            "merge", "Merge Sort", Complexity::Linearithmic, AuxMemory::Linear,
            &merge_sort,
            "stable; single n-element scratch buffer (SortContext::buffer)"},
        Algorithm{
            "heap", "Heap Sort", Complexity::Linearithmic, AuxMemory::Constant,
            &heap_sort,
            "in-place, not stable; binary max-heap (build + extract)"},
    };
    return algorithms;
}

const Algorithm* find_algorithm(std::string_view id) noexcept {
    for (const Algorithm& algorithm : all_algorithms()) {
        if (algorithm.id == id) {
            return &algorithm;
        }
    }
    return nullptr;
}

} // namespace sortbench
