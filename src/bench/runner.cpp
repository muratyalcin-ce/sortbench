#include "bench/runner.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/check.hpp"
#include "core/stopwatch.hpp"

namespace sortbench {
namespace {

/// splitmix64 finaliser; used to derive independent, reproducible dataset seeds.
std::uint64_t mix64(std::uint64_t x) noexcept {
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}

std::uint64_t dataset_seed(std::uint64_t base,
                           Distribution distribution,
                           std::size_t n,
                           unsigned repetition) noexcept {
    std::uint64_t seed = mix64(base + 0x9E3779B97F4A7C15ull);
    seed = mix64(seed ^ (static_cast<std::uint64_t>(distribution) + 1));
    seed = mix64(seed ^ static_cast<std::uint64_t>(n));
    seed = mix64(seed ^ static_cast<std::uint64_t>(repetition));
    return seed;
}

bool is_skipped(const Algorithm& algorithm, std::size_t n, const Options& options) noexcept {
    return options.quadratic_limit != 0 && algorithm.average_complexity == Complexity::Quadratic &&
           n > options.quadratic_limit;
}

struct Accumulator {
    std::size_t count = 0;
    std::size_t failures = 0;
    double sum_ms = 0.0;
    double sum_sq_ms = 0.0;
    double min_ms = std::numeric_limits<double>::infinity();
    double max_ms = 0.0;
    double sum_comparisons = 0.0;
    double sum_moves = 0.0;

    void add(double elapsed_ms, const Metrics& metrics, bool verified) noexcept {
        ++count;
        sum_ms += elapsed_ms;
        sum_sq_ms += elapsed_ms * elapsed_ms;
        min_ms = std::min(min_ms, elapsed_ms);
        max_ms = std::max(max_ms, elapsed_ms);
        sum_comparisons += static_cast<double>(metrics.comparisons);
        sum_moves += static_cast<double>(metrics.moves);
        if (!verified) {
            ++failures;
        }
    }

    [[nodiscard]] double mean_ms() const noexcept {
        return count == 0 ? 0.0 : sum_ms / static_cast<double>(count);
    }

    [[nodiscard]] double stddev_ms() const noexcept {
        if (count < 2) {
            return 0.0;
        }
        const double n = static_cast<double>(count);
        const double variance = (sum_sq_ms - (sum_ms * sum_ms) / n) / (n - 1.0);
        return variance > 0.0 ? std::sqrt(variance) : 0.0;
    }

    [[nodiscard]] double mean_comparisons() const noexcept {
        return count == 0 ? 0.0 : sum_comparisons / static_cast<double>(count);
    }

    [[nodiscard]] double mean_moves() const noexcept {
        return count == 0 ? 0.0 : sum_moves / static_cast<double>(count);
    }
};

std::string format_case_summary(const Algorithm& algorithm,
                                const Accumulator& accumulator,
                                std::size_t aux_bytes) {
    std::ostringstream line;
    line.imbue(std::locale::classic());
    line << "    " << std::left << std::setw(10) << algorithm.id << std::right << std::fixed
         << std::setprecision(3) << std::setw(12) << accumulator.mean_ms() << " ms"
         << "  comp=" << std::setw(12) << std::setprecision(0)
         << accumulator.mean_comparisons() << "  moves=" << std::setw(12)
         << accumulator.mean_moves() << "  aux=" << aux_bytes << " B";

    if (accumulator.failures != 0) {
        line << "  [INCORRECT: " << accumulator.failures << "/" << accumulator.count << "]";
    }
    return line.str();
}

void append_unique(std::vector<const Algorithm*>& target, const Algorithm* algorithm) {
    if (std::find(target.begin(), target.end(), algorithm) == target.end()) {
        target.push_back(algorithm);
    }
}

void append_unique(std::vector<Distribution>& target, Distribution distribution) {
    if (std::find(target.begin(), target.end(), distribution) == target.end()) {
        target.push_back(distribution);
    }
}

std::string join_algorithm_ids(const std::vector<const Algorithm*>& algorithms) {
    std::string joined;
    for (const Algorithm* algorithm : algorithms) {
        if (!joined.empty()) {
            joined += ", ";
        }
        joined += algorithm->id;
    }
    return joined;
}

} // namespace

Plan build_plan(const Options& options) {
    Plan plan;

    // Algorithms: "all" expands to the full registry, in assignment order.
    for (const std::string& name : options.algorithms) {
        if (name == "all") {
            for (const Algorithm& algorithm : all_algorithms()) {
                append_unique(plan.algorithms, &algorithm);
            }
            continue;
        }
        const Algorithm* algorithm = find_algorithm(name);
        if (algorithm == nullptr) {
            std::string valid_ids;
            for (const Algorithm& entry : all_algorithms()) {
                if (!valid_ids.empty()) {
                    valid_ids += ", ";
                }
                valid_ids += entry.id;
            }
            throw std::runtime_error("unknown algorithm '" + name +
                                     "'. Valid ids: " + valid_ids + ", or \"all\"");
        }
        append_unique(plan.algorithms, algorithm);
    }

    // Distributions.
    for (const std::string& name : options.distributions) {
        if (name == "all") {
            for (Distribution distribution : all_distributions()) {
                append_unique(plan.distributions, distribution);
            }
            continue;
        }
        const std::optional<Distribution> distribution = parse_distribution(name);
        if (!distribution) {
            throw std::runtime_error("unknown distribution '" + name +
                                     "'. Valid names: random, sorted, reverse, nearly_sorted");
        }
        append_unique(plan.distributions, *distribution);
    }

    // Sizes, preserving user order but removing duplicates.
    for (std::size_t size : options.sizes) {
        if (size == 0) {
            throw std::runtime_error("dataset sizes must be positive");
        }
        if (std::find(plan.sizes.begin(), plan.sizes.end(), size) == plan.sizes.end()) {
            plan.sizes.push_back(size);
        }
    }

    if (plan.algorithms.empty()) {
        throw std::runtime_error("no algorithms selected");
    }
    if (plan.distributions.empty()) {
        throw std::runtime_error("no distributions selected");
    }
    if (plan.sizes.empty()) {
        throw std::runtime_error("no dataset sizes selected");
    }

    std::size_t cases_per_distribution = 0;
    for (std::size_t size : plan.sizes) {
        for (const Algorithm* algorithm : plan.algorithms) {
            if (!is_skipped(*algorithm, size, options)) {
                ++cases_per_distribution;
            }
        }
    }
    plan.total_cases = cases_per_distribution * plan.distributions.size();

    return plan;
}

std::vector<ResultRow> run_benchmark(const Plan& plan,
                                     const Options& options,
                                     std::ostream& progress) {
    std::vector<ResultRow> rows;
    rows.reserve(plan.total_cases);
    std::size_t case_index = 0;

    for (Distribution distribution : plan.distributions) {
        for (std::size_t n : plan.sizes) {
            std::vector<const Algorithm*> active;
            std::vector<const Algorithm*> skipped;
            for (const Algorithm* algorithm : plan.algorithms) {
                if (is_skipped(*algorithm, n, options)) {
                    skipped.push_back(algorithm);
                } else {
                    active.push_back(algorithm);
                }
            }

            if (!options.quiet && !skipped.empty()) {
                progress << "    skipping " << join_algorithm_ids(skipped) << " at n=" << n
                         << " (quadratic-limit=" << options.quadratic_limit << ")\n";
            }
            if (active.empty()) {
                continue;
            }

            ++case_index;
            if (!options.quiet) {
                progress << "[" << case_index << "/" << plan.total_cases << "] "
                         << to_string(distribution) << " n=" << n << " (" << active.size()
                         << " algorithms x " << options.repetitions << " reps)\n";
            }

            // Warm-up: same code path and same distribution, results discarded.
            if (options.warmup_runs > 0) {
                const std::vector<Value> warmup_base =
                    make_dataset(distribution, n, dataset_seed(options.seed, distribution, n, 0));
                for (const Algorithm* algorithm : active) {
                    for (unsigned run = 0; run < options.warmup_runs; ++run) {
                        std::vector<Value> data = warmup_base;
                        SortContext context;
                        context.buffer.resize(n);
                        algorithm->sort(data, context);
                    }
                }
            }

            std::vector<Accumulator> accumulators(active.size());

            for (unsigned repetition = 0; repetition < options.repetitions; ++repetition) {
                // One dataset per repetition, shared by every algorithm.
                const std::vector<Value> base = make_dataset(
                    distribution, n, dataset_seed(options.seed, distribution, n, repetition));

                // Reference result produced by the project's own merge sort.
                // It is used only for validation and is never timed.
                std::vector<Value> reference = base;
                {
                    SortContext reference_context;
                    reference_context.buffer.resize(n);
                    merge_sort(reference, reference_context);
                }

                for (std::size_t index = 0; index < active.size(); ++index) {
                    const Algorithm& algorithm = *active[index];

                    // Fresh copy outside the timed region: algorithms never see
                    // another algorithm's output (fair comparison).
                    std::vector<Value> data = base;

                    SortContext context;
                    context.buffer.resize(n);
                    context.metrics.reset();

                    Stopwatch watch;
                    watch.restart();
                    algorithm.sort(data, context);
                    const double elapsed_ms = watch.elapsed_ms();

                    const bool verified = is_sorted(data) && data == reference;
                    accumulators[index].add(elapsed_ms, context.metrics, verified);

                    if (!verified && !options.quiet) {
                        progress << "    ! " << algorithm.id << " produced an incorrect result at "
                                 << to_string(distribution) << " n=" << n
                                 << " (rep " << repetition + 1 << ")\n";
                    }
                }
            }

            for (std::size_t index = 0; index < active.size(); ++index) {
                const Algorithm& algorithm = *active[index];
                const Accumulator& accumulator = accumulators[index];

                ResultRow row;
                row.algorithm = std::string(algorithm.id);
                row.distribution = std::string(to_string(distribution));
                row.n = n;
                row.repetitions = static_cast<unsigned>(accumulator.count);
                row.mean_ms = accumulator.mean_ms();
                row.stddev_ms = accumulator.stddev_ms();
                row.min_ms = accumulator.min_ms;
                row.max_ms = accumulator.max_ms;
                row.mean_comparisons = accumulator.mean_comparisons();
                row.mean_moves = accumulator.mean_moves();
                row.aux_memory = std::string(to_string(algorithm.aux_memory));
                row.aux_bytes = aux_bytes_estimate(algorithm, n);
                row.verified = accumulator.failures == 0;

                if (!options.quiet) {
                    progress << format_case_summary(algorithm, accumulator, row.aux_bytes)
                             << "\n";
                }
                rows.push_back(std::move(row));
            }
        }
    }

    return rows;
}

std::size_t count_failures(const std::vector<ResultRow>& rows) noexcept {
    return static_cast<std::size_t>(
        std::count_if(rows.begin(), rows.end(), [](const ResultRow& row) { return !row.verified; }));
}

} // namespace sortbench
