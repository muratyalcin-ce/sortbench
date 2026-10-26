#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "core/types.hpp"

namespace sortbench {

/// Data distributions required by the assignment (section 6).
enum class Distribution {
    Random = 0,        ///< A. uniformly random values
    Sorted,            ///< B. ascending, 0..n-1
    ReverseSorted,     ///< C. descending, n-1..0
    NearlySorted,      ///< D. ascending with ~10% of the elements displaced
};

/// All four distributions, in assignment order.
[[nodiscard]] const std::vector<Distribution>& all_distributions();

/// Stable, CSV-friendly identifier ("random", "sorted", "reverse", "nearly_sorted").
[[nodiscard]] std::string_view to_string(Distribution distribution) noexcept;

/// Parses an identifier or a common alias, e.g. "reversed", "descending", "nearly".
[[nodiscard]] std::optional<Distribution> parse_distribution(std::string_view text) noexcept;

/// Builds a dataset deterministically from `seed`.
///
/// * Random:       values drawn uniformly from [0, n).
/// * Sorted:       0, 1, ..., n-1.
/// * ReverseSorted:n-1, ..., 1, 0.
/// * NearlySorted: ascending, then n/10 random pair swaps are applied.
///
/// Data generation never uses the standard library sort. Values are 32-bit, so
/// n must stay well below 2^31 (the assignment uses n <= 100.000).
[[nodiscard]] std::vector<Value> make_dataset(Distribution distribution,
                                              std::size_t n,
                                              std::uint64_t seed);

} // namespace sortbench
