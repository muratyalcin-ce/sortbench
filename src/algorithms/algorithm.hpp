#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "core/types.hpp"

namespace sortbench {

/// Context passed to every algorithm.
///
/// `buffer` is pre-sized by the benchmark runner *before* the stopwatch starts,
/// so the allocation is not part of the measured sorting time. Only merge sort
/// uses it; the other algorithms are in-place.
struct SortContext {
    Metrics metrics;
    std::vector<Value> buffer;

    /// Reserved for randomized variants (for example "random pivot" quick
    /// sort). Deterministic algorithms must not read it, otherwise repetitions
    /// would no longer be reproducible.
    std::uint64_t rng_state = 0x9E3779B97F4A7C15ull;
};

using SortFn = void (*)(std::vector<Value>&, SortContext&);

enum class Complexity {
    Quadratic,      ///< O(n^2)
    Linearithmic,   ///< O(n log n)
    Depends,        ///< depends on gap sequence / pivot strategy
};

enum class AuxMemory {
    Constant,      ///< O(1) extra memory
    Logarithmic,   ///< O(log n) expected recursion stack
    Linear,        ///< O(n) auxiliary buffer
};

[[nodiscard]] std::string_view to_string(Complexity complexity) noexcept;
[[nodiscard]] std::string_view to_string(AuxMemory memory) noexcept;

struct Algorithm {
    std::string_view id;       ///< stable identifier used by --algos and the CSV
    std::string_view label;    ///< human readable name
    Complexity average_complexity;
    AuxMemory aux_memory;
    SortFn sort;
    std::string_view note;     ///< pivot choice, gap sequence, stability, ...
};

/// Analytic (not measured) auxiliary memory estimate for a dataset of size n.
///
/// Constant -> 0, Linear -> n * sizeof(Value), Logarithmic -> expected stack.
[[nodiscard]] std::size_t aux_bytes_estimate(const Algorithm& algorithm,
                                             std::size_t n) noexcept;

/// The seven sorting algorithms required by the assignment, in assignment order.
[[nodiscard]] const std::vector<Algorithm>& all_algorithms();

/// Returns nullptr when the identifier is unknown.
[[nodiscard]] const Algorithm* find_algorithm(std::string_view id) noexcept;

// ---------------------------------------------------------------------------
// Implementations (one translation unit each, no library sort functions).
// ---------------------------------------------------------------------------
void selection_sort(std::vector<Value>& data, SortContext& ctx);
void insertion_sort(std::vector<Value>& data, SortContext& ctx);
void bubble_sort(std::vector<Value>& data, SortContext& ctx);
void shell_sort(std::vector<Value>& data, SortContext& ctx);
void quick_sort(std::vector<Value>& data, SortContext& ctx);
void merge_sort(std::vector<Value>& data, SortContext& ctx);
void heap_sort(std::vector<Value>& data, SortContext& ctx);

} // namespace sortbench
