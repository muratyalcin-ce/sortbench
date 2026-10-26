#pragma once

#include <iosfwd>
#include <vector>

#include "bench/runner.hpp"

namespace sortbench {

/// Serialises aggregated results as RFC 4180-ish CSV.
///
/// Columns:
///   algorithm,distribution,n,repetitions,mean_time_ms,stddev_time_ms,
///   min_time_ms,max_time_ms,mean_comparisons,mean_moves,aux_memory,aux_bytes,verified
void write_csv(std::ostream& out, const std::vector<ResultRow>& rows);

} // namespace sortbench
