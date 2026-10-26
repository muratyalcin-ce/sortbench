#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace sortbench {

/// Command-line configuration. Defaults follow the assignment (section 5).
struct Options {
    std::vector<std::string> algorithms{"all"};
    std::vector<std::string> distributions{"all"};
    std::vector<std::size_t> sizes{100, 1000, 5000, 10000, 50000, 100000};

    unsigned repetitions = 5;    ///< measured runs per case
    unsigned warmup_runs = 1;    ///< discarded runs per case (startup / cache effects)
    std::uint64_t seed = 42;     ///< base seed; each repetition derives its own dataset
    std::size_t quadratic_limit = 0;  ///< skip O(n^2) algorithms when n > limit; 0 = off

    std::string output = "results.csv";
    bool quiet = false;
    bool dry_run = false;
};

enum class ParseStatus {
    Ok,
    HelpRequested,
    ListRequested,
    Error,
};

struct ParseResult {
    ParseStatus status = ParseStatus::Error;
    std::string message;
    Options options;
};

/// Pure parser: never exits the process and never writes to a stream.
[[nodiscard]] ParseResult parse_command_line(int argc, char** argv);

void print_usage(std::ostream& out);
void print_algorithm_table(std::ostream& out);

} // namespace sortbench
