#include <cstddef>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "bench/runner.hpp"
#include "cli/options.hpp"
#include "io/csv_writer.hpp"
#include "version.hpp"

namespace {

std::string compiler_description() {
#if defined(_MSC_VER)
    std::ostringstream text;
    text << "MSVC " << _MSC_VER;
    return text.str();
#elif defined(__clang__)
    std::ostringstream text;
    text << "Clang " << __clang_major__ << '.' << __clang_minor__ << '.' << __clang_patchlevel__;
    return text.str();
#elif defined(__GNUC__)
    std::ostringstream text;
    text << "GCC " << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
    return text.str();
#else
    return "unknown compiler";
#endif
}

void print_environment(std::ostream& out, const sortbench::Options& options) {
    out << "SortBench " << SORTBENCH_VERSION << " | " << compiler_description() << " | C++"
        << (__cplusplus / 100 % 100) << " | seed=" << options.seed
        << " | reps=" << options.repetitions << " | warmup=" << options.warmup_runs << "\n";
}

void print_plan(std::ostream& out, const sortbench::Plan& plan, const sortbench::Options& options) {
    using sortbench::to_string;

    out << "\nExperiment plan\n";
    out << "  algorithms   : ";
    for (std::size_t i = 0; i < plan.algorithms.size(); ++i) {
        out << (i == 0 ? "" : ", ") << plan.algorithms[i]->id;
    }
    out << "\n  distributions: ";
    for (std::size_t i = 0; i < plan.distributions.size(); ++i) {
        out << (i == 0 ? "" : ", ") << to_string(plan.distributions[i]);
    }
    out << "\n  sizes        : ";
    for (std::size_t i = 0; i < plan.sizes.size(); ++i) {
        out << (i == 0 ? "" : ", ") << plan.sizes[i];
    }
    out << "\n  repetitions  : " << options.repetitions << " (+" << options.warmup_runs
        << " warm-up)\n"
        << "  seed         : " << options.seed << "\n"
        << "  quadratic    : "
        << (options.quadratic_limit == 0
                ? std::string("no limit (O(n^2) algorithms may be very slow at large n)")
                : "skip O(n^2) algorithms above n=" + std::to_string(options.quadratic_limit))
        << "\n"
        << "  output       : " << options.output << "\n"
        << "  dataset cases: " << plan.dataset_cases << "\n"
        << "  result rows  : " << plan.total_rows << "\n"
        << "  measured runs: " << plan.total_rows * options.repetitions << "\n\n";
}

} // namespace

int main(int argc, char** argv) {
    using namespace sortbench;

    const ParseResult parsed = parse_command_line(argc, argv);
    switch (parsed.status) {
    case ParseStatus::HelpRequested:
        print_usage(std::cout);
        return 0;
    case ParseStatus::ListRequested:
        print_algorithm_table(std::cout);
        return 0;
    case ParseStatus::Error:
        std::cerr << "error: " << parsed.message << "\n\n";
        print_usage(std::cerr);
        return 2;
    case ParseStatus::Ok:
        break;
    }

    const Options& options = parsed.options;

    try {
        const Plan plan = build_plan(options);
        if (!options.quiet) {
            print_environment(std::cerr, options);
        }

        if (options.dry_run) {
            print_plan(std::cout, plan, options);
            return 0;
        }
        if (plan.empty()) {
            std::cerr << "warning: the selected options produced no runnable cases\n";
            return 1;
        }

        const std::vector<ResultRow> rows = run_benchmark(plan, options, std::cerr);

        if (options.output == "-") {
            write_csv(std::cout, rows);
            std::cout.flush();
        } else {
            std::ofstream file(options.output, std::ios::binary | std::ios::trunc);
            if (!file) {
                std::cerr << "error: cannot open output file '" << options.output << "'\n";
                return 2;
            }
            write_csv(file, rows);
            file.flush();
            if (!file) {
                std::cerr << "error: failed while writing '" << options.output << "'\n";
                return 2;
            }
            if (!options.quiet) {
                std::cerr << "wrote " << rows.size() << " rows to " << options.output << "\n";
            }
        }

        const std::size_t failures = count_failures(rows);
        if (failures != 0) {
            std::cerr << "error: " << failures
                      << " aggregated case(s) failed correctness verification\n";
            return 3;
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 2;
    }
}
