#pragma once

#include <cstdint>
#include <utility>

namespace sortbench {

/// Element type used by every algorithm.
///
/// It is fixed for the whole project so that timings, comparison counts and
/// memory estimates are directly comparable between algorithms.
using Value = std::int32_t;

/// Instrumentation counters shared by all algorithms.
///
/// Definitions used by this project (quote them in the report):
///   * comparisons: one increment per element-to-element comparison.
///   * moves:       one increment per element assignment / data movement.
///                  A swap is counted as 3 moves (temp, a, b), which is the
///                  classical convention used in algorithm analysis.
///
/// Reading an element into a local variable is not counted as a move, except
/// inside swaps where the temporary copy is part of the 3-move convention.
struct Metrics {
    std::uint64_t comparisons = 0;
    std::uint64_t moves = 0;

    void reset() noexcept {
        comparisons = 0;
        moves = 0;
    }
};

/// Counted `a < b` comparison.
inline bool less(Value a, Value b, Metrics& metrics) noexcept {
    ++metrics.comparisons;
    return a < b;
}

/// Counted `a <= b` comparison.
inline bool less_equal(Value a, Value b, Metrics& metrics) noexcept {
    ++metrics.comparisons;
    return a <= b;
}

/// Swaps two elements and records the swap as three data moves.
inline void swap_values(Value& a, Value& b, Metrics& metrics) noexcept {
    if (&a == &b) {
        return;
    }
    const Value tmp = a;
    a = b;
    b = tmp;
    metrics.moves += 3;
}

} // namespace sortbench
