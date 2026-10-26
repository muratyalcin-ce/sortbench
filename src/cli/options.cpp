#include "cli/options.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <ostream>
#include <string_view>
#include <system_error>

#include "algorithms/algorithm.hpp"

namespace sortbench {
namespace {

std::string_view trim(std::string_view text) {
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        return {};
    }
    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::vector<std::string> split_list(std::string_view text) {
    std::vector<std::string> items;
    std::size_t start = 0;

    while (start <= text.size()) {
        const std::size_t comma = text.find(',', start);
        const std::size_t end = (comma == std::string_view::npos) ? text.size() : comma;
        const std::string_view item = trim(text.substr(start, end - start));
        if (!item.empty()) {
            items.emplace_back(item);
        }
        if (comma == std::string_view::npos) {
            break;
        }
        start = comma + 1;
    }
    return items;
}

bool parse_unsigned(std::string_view text, std::uint64_t& value) {
    if (text.empty()) {
        return false;
    }
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const std::from_chars_result result = std::from_chars(begin, end, value);
    return result.ec == std::errc() && result.ptr == end;
}

std::string option_label(std::string_view name) {
    return "'" + std::string(name) + "'";
}

} // namespace

ParseResult parse_command_line(int argc, char** argv) {
    ParseResult result;
    result.status = ParseStatus::Ok;
    Options& options = result.options;

    auto fail = [&result](std::string message) -> ParseResult {
        result.status = ParseStatus::Error;
        result.message = std::move(message);
        return result;
    };

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg.empty()) {
            continue;
        }

        std::string_view name = arg;
        std::string_view inline_value;
        bool has_inline_value = false;

        if (arg.rfind("--", 0) == 0) {
            const std::size_t equals = arg.find('=');
            if (equals != std::string_view::npos) {
                name = arg.substr(0, equals);
                inline_value = arg.substr(equals + 1);
                has_inline_value = true;
            }
        }

        auto take_value = [&]() -> std::optional<std::string_view> {
            if (has_inline_value) {
                return inline_value;
            }
            if (i + 1 >= argc) {
                return std::nullopt;
            }
            ++i;
            return std::string_view(argv[i]);
        };

        auto reject_value = [&]() -> bool {
            return !has_inline_value;
        };

        if (name == "-h" || name == "--help") {
            if (!reject_value()) {
                return fail(option_label(name) + " does not take a value");
            }
            result.status = ParseStatus::HelpRequested;
            return result;
        }
        if (name == "--list") {
            if (!reject_value()) {
                return fail(option_label(name) + " does not take a value");
            }
            result.status = ParseStatus::ListRequested;
            return result;
        }
        if (name == "--algos" || name == "--algo" || name == "--algorithms") {
            const std::optional<std::string_view> value = take_value();
            if (!value) {
                return fail("missing value for " + option_label(name));
            }
            options.algorithms = split_list(*value);
            if (options.algorithms.empty()) {
                return fail(option_label(name) + " needs at least one identifier");
            }
            continue;
        }
        if (name == "--dist" || name == "--distribution" || name == "--distributions") {
            const std::optional<std::string_view> value = take_value();
            if (!value) {
                return fail("missing value for " + option_label(name));
            }
            options.distributions = split_list(*value);
            if (options.distributions.empty()) {
                return fail(option_label(name) + " needs at least one identifier");
            }
            continue;
        }
        if (name == "--sizes" || name == "--size") {
            const std::optional<std::string_view> value = take_value();
            if (!value) {
                return fail("missing value for " + option_label(name));
            }
            constexpr std::uint64_t kMaxSize = 1ull << 30;
            std::vector<std::size_t> sizes;
            std::vector<std::string> items = split_list(*value);
            if (items.empty()) {
                return fail(option_label(name) + " needs at least one value");
            }
            for (const std::string& item : items) {
                std::uint64_t size = 0;
                if (!parse_unsigned(item, size) || size == 0 || size > kMaxSize) {
                    return fail("invalid dataset size '" + item +
                                "' (expected an integer in [1, 1073741824])");
                }
                sizes.push_back(static_cast<std::size_t>(size));
            }
            options.sizes = std::move(sizes);
            continue;
        }
        if (name == "--reps" || name == "--repetitions") {
            const std::optional<std::string_view> value = take_value();
            if (!value) {
                return fail("missing value for " + option_label(name));
            }
            std::uint64_t repetitions = 0;
            if (!parse_unsigned(*value, repetitions) || repetitions < 1 || repetitions > 1000000) {
                return fail(option_label(name) + " expects an integer in [1, 1000000]");
            }
            options.repetitions = static_cast<unsigned>(repetitions);
            continue;
        }
        if (name == "--warmup") {
            const std::optional<std::string_view> value = take_value();
            if (!value) {
                return fail("missing value for " + option_label(name));
            }
            std::uint64_t warmup = 0;
            if (!parse_unsigned(*value, warmup) || warmup > 1000) {
                return fail(option_label(name) + " expects an integer in [0, 1000]");
            }
            options.warmup_runs = static_cast<unsigned>(warmup);
            continue;
        }
        if (name == "--seed") {
            const std::optional<std::string_view> value = take_value();
            if (!value) {
                return fail("missing value for " + option_label(name));
            }
            std::uint64_t seed = 0;
            if (!parse_unsigned(*value, seed)) {
                return fail(option_label(name) + " expects a non-negative integer");
            }
            options.seed = seed;
            continue;
        }
        if (name == "--quadratic-limit") {
            const std::optional<std::string_view> value = take_value();
            if (!value) {
                return fail("missing value for " + option_label(name));
            }
            std::uint64_t limit = 0;
            if (!parse_unsigned(*value, limit)) {
                return fail(option_label(name) + " expects a non-negative integer");
            }
            options.quadratic_limit = static_cast<std::size_t>(limit);
            continue;
        }
        if (name == "--out" || name == "--output") {
            const std::optional<std::string_view> value = take_value();
            if (!value) {
                return fail("missing value for " + option_label(name));
            }
            options.output = std::string(*value);
            if (options.output.empty()) {
                return fail(option_label(name) + " needs a non-empty path");
            }
            continue;
        }
        if (name == "--quiet") {
            if (!reject_value()) {
                return fail(option_label(name) + " does not take a value");
            }
            options.quiet = true;
            continue;
        }
        if (name == "--dry-run") {
            if (!reject_value()) {
                return fail(option_label(name) + " does not take a value");
            }
            options.dry_run = true;
            continue;
        }
        if (!arg.empty() && arg.front() == '-') {
            return fail("unknown option " + option_label(arg));
        }
        return fail("unexpected argument " + option_label(arg));
    }

    return result;
}

void print_usage(std::ostream& out) {
    out << "SortBench - sorting algorithm benchmark (BMT317)\n"
        << "\n"
        << "Usage:\n"
        << "  sortbench [options]\n"
        << "\n"
        << "Datasets are generated deterministically from --seed:\n"
        << "  random, sorted, reverse, nearly_sorted\n"
        << "\n"
        << "Options:\n"
        << "  -h, --help             Show this help and exit\n"
        << "      --list             List the implemented algorithms and exit\n"
        << "      --algos LIST       Comma separated ids, or \"all\" (default: all)\n"
        << "      --dist LIST        Comma separated distributions, or \"all\" (default: all)\n"
        << "      --sizes LIST       Comma separated sizes (default: "
           "100,1000,5000,10000,50000,100000)\n"
        << "      --reps N           Measured repetitions per case, >= 1 (default: 5)\n"
        << "      --warmup N         Discarded warm-up runs per case, >= 0 (default: 1)\n"
        << "      --seed N           Base RNG seed for dataset generation (default: 42)\n"
        << "      --quadratic-limit N\n"
        << "                         Skip O(n^2) algorithms when n > N; 0 disables\n"
        << "                         the limit (default: 0 = run everything)\n"
        << "      --out PATH         CSV output path, \"-\" for stdout (default: results.csv)\n"
        << "      --quiet            Do not print progress to stderr\n"
        << "      --dry-run          Print the experiment plan and exit\n"
        << "\n"
        << "Measurement rules:\n"
        << "  * Time is measured with std::chrono::steady_clock around the sort call\n"
        << "    only; dataset generation and copies are outside the timed region.\n"
        << "  * Within one repetition every algorithm sorts a fresh copy of the same\n"
        << "    dataset, so algorithms never see another algorithm's output.\n"
        << "  * Comparisons and data moves are counted inside the algorithms.\n"
        << "  * --warmup runs are discarded (startup and cache warm-up effects).\n"
        << "\n"
        << "Examples:\n"
        << "  sortbench --dry-run\n"
        << "  sortbench --algos quick,merge,heap --sizes 10000,100000 --reps 3\n"
        << "  sortbench --algos all --quadratic-limit 10000 --out results.csv\n";
}

void print_algorithm_table(std::ostream& out) {
    out << "Implemented algorithms:\n\n";
    out << "  " << std::left << std::setw(12) << "id" << std::setw(16) << "name"
        << std::setw(12) << "average" << std::setw(10) << "memory"
        << "notes\n";
    out << "  " << std::left << std::setw(12) << std::string(11, '-') << std::setw(16)
        << std::string(15, '-') << std::setw(12) << std::string(11, '-') << std::setw(10)
        << std::string(9, '-') << std::string(40, '-') << "\n";

    for (const Algorithm& algorithm : all_algorithms()) {
        out << "  " << std::left << std::setw(12) << algorithm.id << std::setw(16)
            << algorithm.label << std::setw(12) << to_string(algorithm.average_complexity)
            << std::setw(10) << to_string(algorithm.aux_memory) << algorithm.note << "\n";
    }
    out << std::right;
}

} // namespace sortbench
