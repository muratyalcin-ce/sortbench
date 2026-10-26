#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

#include "algorithms/algorithm.hpp"
#include "cli/options.hpp"
#include "core/dataset.hpp"

namespace sortbench {

/// One aggregated CSV row: one algorithm on one distribution/size pair.
struct ResultRow {
    std::string algorithm;
    std::string distribution;
    std::size_t n = 0;
    unsigned repetitions = 0;
    double mean_ms = 0.0;
    double stddev_ms = 0.0;
    double min_ms = 0.0;
    double max_ms = 0.0;
    double mean_comparisons = 0.0;
    double mean_moves = 0.0;
    std::string aux_memory;
    std::size_t aux_bytes = 0;
    bool verified = false;
};

/// Resolved, validated experiment plan.
struct Plan {
    std::vector<const Algorithm*> algorithms;
    std::vector<Distribution> distributions;
    std::vector<std::size_t> sizes;

    /// Number of (distribution, size) pairs that will actually run, i.e. that
    /// have at least one algorithm not skipped by --quadratic-limit.
    std::size_t dataset_cases = 0;

    /// Number of CSV rows: dataset_cases x algorithms (minus skipped cases).
    std::size_t total_rows = 0;

    bool empty() const noexcept { return total_rows == 0; }
};

/// Resolves names from Options into concrete algorithms/distributions, removes
/// duplicates and validates sizes. Throws std::runtime_error on unknown names.
[[nodiscard]] Plan build_plan(const Options& options);

/// Runs the experiment and returns aggregated rows.
///
/// Progress and warnings are written to `progress` (stderr in the CLI); the
/// returned data is what gets serialised to CSV.
[[nodiscard]] std::vector<ResultRow> run_benchmark(const Plan& plan,
                                                   const Options& options,
                                                   std::ostream& progress);

/// Number of rows whose result failed the is_sorted / reference comparison.
[[nodiscard]] std::size_t count_failures(const std::vector<ResultRow>& rows) noexcept;

} // namespace sortbench
