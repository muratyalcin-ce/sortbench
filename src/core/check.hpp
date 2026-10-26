#pragma once

#include <vector>

#include "core/types.hpp"

namespace sortbench {

/// Required correctness check: true when `data` is in non-decreasing order.
///
/// This function is intentionally free of instrumentation counters so it can be
/// used to validate the result of every algorithm without polluting the
/// measurements.
[[nodiscard]] bool is_sorted(const std::vector<Value>& data) noexcept;

} // namespace sortbench
