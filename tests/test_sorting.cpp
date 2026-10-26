#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/algorithm.hpp"
#include "core/check.hpp"
#include "core/dataset.hpp"

namespace {

using sortbench::Algorithm;
using sortbench::Distribution;
using sortbench::SortContext;
using sortbench::Value;

int g_checks = 0;
int g_failures = 0;

void expect(bool condition, const std::string& message) {
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::cerr << "FAIL: " << message << "\n";
    }
}

std::string label(std::string_view algorithm, Distribution distribution, std::size_t n) {
    return std::string(algorithm) + " / " + std::string(sortbench::to_string(distribution)) +
           " / n=" + std::to_string(n);
}

/// Sorts a copy with the algorithm under test and validates it against a
/// reference result produced by the project's own merge sort.
void check_algorithm_all_distributions(const Algorithm& algorithm,
                                       Distribution distribution,
                                       std::size_t n) {
    const std::vector<Value> input = sortbench::make_dataset(distribution, n, 0xC0FFEEu + n);

    std::vector<Value> expected = input;
    {
        SortContext reference;
        reference.buffer.resize(n);
        sortbench::merge_sort(expected, reference);
    }

    std::vector<Value> actual = input;
    SortContext context;
    context.buffer.resize(n);
    algorithm.sort(actual, context);

    const std::string name = label(algorithm.id, distribution, n);
    expect(sortbench::is_sorted(actual), name + ": result is not sorted");
    expect(actual == expected, name + ": result is not a permutation of the input");

    if (n < 2) {
        expect(context.metrics.comparisons == 0 && context.metrics.moves == 0,
               name + ": n<2 must not perform comparisons or moves");
    }
}

void test_is_sorted() {
    expect(sortbench::is_sorted({}), "empty array is sorted");
    expect(sortbench::is_sorted(std::vector<Value>{42}), "single element is sorted");
    expect(sortbench::is_sorted(std::vector<Value>{1, 2, 2, 3}), "non-decreasing array is sorted");
    expect(!sortbench::is_sorted(std::vector<Value>{2, 1}), "descending pair is not sorted");
    expect(!sortbench::is_sorted(std::vector<Value>{1, 3, 2}), "unsorted triple is not sorted");
}

void test_dataset_generation() {
    const std::size_t n = 1000;

    const std::vector<Value> sorted = sortbench::make_dataset(Distribution::Sorted, n, 7);
    expect(sorted.size() == n, "sorted dataset has n elements");
    expect(sortbench::is_sorted(sorted), "sorted dataset is ascending");

    const std::vector<Value> reverse = sortbench::make_dataset(Distribution::ReverseSorted, n, 7);
    bool descending = true;
    for (std::size_t i = 1; i < reverse.size(); ++i) {
        descending = descending && reverse[i] < reverse[i - 1];
    }
    expect(descending, "reverse dataset is strictly descending");

    const std::vector<Value> nearly_a = sortbench::make_dataset(Distribution::NearlySorted, n, 7);
    const std::vector<Value> nearly_b = sortbench::make_dataset(Distribution::NearlySorted, n, 7);
    expect(nearly_a == nearly_b, "dataset generation is deterministic for a fixed seed");
    expect(nearly_a != sorted, "nearly sorted dataset is not fully sorted");

    std::size_t displaced = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (nearly_a[i] != static_cast<Value>(i)) {
            ++displaced;
        }
    }
    expect(displaced > 0 && displaced < n, "nearly sorted dataset displaces only some elements");

    const std::vector<Value> random_a = sortbench::make_dataset(Distribution::Random, n, 123);
    const std::vector<Value> random_b = sortbench::make_dataset(Distribution::Random, n, 123);
    const std::vector<Value> random_c = sortbench::make_dataset(Distribution::Random, n, 124);
    expect(random_a == random_b, "random dataset is reproducible for a fixed seed");
    expect(random_a != random_c, "different seeds produce different random datasets");

    bool in_range = true;
    for (Value value : random_a) {
        in_range = in_range && value >= 0 && value < static_cast<Value>(n);
    }
    expect(in_range, "random values are in [0, n)");

    expect(sortbench::make_dataset(Distribution::Random, 0, 1).empty(),
           "empty dataset can be generated");
}

void test_all_algorithms() {
    const std::vector<std::size_t> sizes{0, 1, 2, 3, 4, 5, 8, 17, 64, 257, 1023};

    for (const Algorithm& algorithm : sortbench::all_algorithms()) {
        for (Distribution distribution : sortbench::all_distributions()) {
            for (std::size_t n : sizes) {
                check_algorithm_all_distributions(algorithm, distribution, n);
            }
        }
    }
}

void test_duplicate_heavy_input() {
    const std::vector<std::size_t> sizes{2, 3, 10, 100, 1000};

    for (const Algorithm& algorithm : sortbench::all_algorithms()) {
        for (std::size_t n : sizes) {
            // All elements equal: the classic worst case for Lomuto partitioning.
            std::vector<Value> data(n, 7);
            SortContext context;
            context.buffer.resize(n);
            algorithm.sort(data, context);
            expect(sortbench::is_sorted(data), std::string(algorithm.id) +
                                                   ": all-equal input is not sorted");

            // Only two distinct values: many duplicates, unstable algorithms may
            // permute equals, so only sortedness is validated here.
            std::vector<Value> binary(n);
            for (std::size_t i = 0; i < n; ++i) {
                binary[i] = (i % 3 == 0) ? static_cast<Value>(1) : static_cast<Value>(2);
            }
            SortContext binary_context;
            binary_context.buffer.resize(n);
            algorithm.sort(binary, binary_context);
            expect(sortbench::is_sorted(binary), std::string(algorithm.id) +
                                                      ": two-value input is not sorted");
        }
    }
}

/// Randomized differential test: every algorithm is compared against the
/// reference merge sort on arrays with small value ranges (many duplicates),
/// which is the most error-prone case for partitioning and gap-based sorts.
void test_randomized_fuzz() {
    std::mt19937_64 rng(2024);

    for (const Algorithm& algorithm : sortbench::all_algorithms()) {
        for (int trial = 0; trial < 100; ++trial) {
            const std::size_t n = static_cast<std::size_t>(rng() % 400);
            const unsigned range = 1 + static_cast<unsigned>(rng() % 20);

            std::vector<Value> input(n);
            for (Value& value : input) {
                value = static_cast<Value>(rng() % range);
            }

            std::vector<Value> expected = input;
            {
                SortContext reference;
                reference.buffer.resize(n);
                sortbench::merge_sort(expected, reference);
            }

            std::vector<Value> actual = input;
            SortContext context;
            context.buffer.resize(n);
            algorithm.sort(actual, context);

            expect(actual == expected, std::string(algorithm.id) +
                                            ": fuzz trial " + std::to_string(trial) +
                                            " (n=" + std::to_string(n) + ", range=" +
                                            std::to_string(range) + ") is incorrect");
        }
    }
}

void test_large_sorted_input() {    // Guards against quadratic blow-ups on already sorted input (middle pivot).
    const std::size_t n = 20000;
    for (const Algorithm& algorithm : sortbench::all_algorithms()) {
        if (algorithm.average_complexity == sortbench::Complexity::Quadratic) {
            continue; // O(n^2) is expected to be slow; covered by the small sizes above
        }
        std::vector<Value> data = sortbench::make_dataset(Distribution::Sorted, n, 99);
        SortContext context;
        context.buffer.resize(n);
        algorithm.sort(data, context);
        expect(sortbench::is_sorted(data),
               std::string(algorithm.id) + ": large sorted input is not sorted");
    }
}

void test_measurement_counters() {
    // Selection Sort always performs exactly n(n-1)/2 comparisons.
    {
        const std::size_t n = 100;
        std::vector<Value> data = sortbench::make_dataset(Distribution::ReverseSorted, n, 1);
        SortContext context;
        context.buffer.resize(n);
        sortbench::selection_sort(data, context);
        expect(context.metrics.comparisons == n * (n - 1) / 2,
               "selection sort comparison count matches n(n-1)/2");
        expect(context.metrics.moves == 3 * (n / 2),
               "selection sort performs floor(n/2) swaps on reversed input");
    }

    // Insertion Sort is linear on already sorted input and performs no moves.
    {
        const std::size_t n = 1000;
        std::vector<Value> data = sortbench::make_dataset(Distribution::Sorted, n, 1);
        SortContext context;
        context.buffer.resize(n);
        sortbench::insertion_sort(data, context);
        expect(context.metrics.comparisons == n - 1,
               "insertion sort performs n-1 comparisons on sorted input");
        expect(context.metrics.moves == 0, "insertion sort performs no moves on sorted input");
    }

    // Bubble Sort exits after the first pass on sorted input.
    {
        const std::size_t n = 100;
        std::vector<Value> data = sortbench::make_dataset(Distribution::Sorted, n, 1);
        SortContext context;
        context.buffer.resize(n);
        sortbench::bubble_sort(data, context);
        expect(context.metrics.comparisons == n - 1,
               "bubble sort performs n-1 comparisons on sorted input");
        expect(context.metrics.moves == 0, "bubble sort performs no moves on sorted input");
    }

    // Merge Sort: for n = 16 (power of two) each of the 4 levels merges n
    // elements (n writes to the buffer + n writes back) and performs n/2
    // comparisons when the input is already sorted.
    {
        const std::size_t n = 16;
        std::vector<Value> data = sortbench::make_dataset(Distribution::Sorted, n, 1);
        SortContext context;
        context.buffer.resize(n);
        sortbench::merge_sort(data, context);
        expect(context.metrics.comparisons == (n / 2) * 4,
               "merge sort comparison count on sorted input");
        expect(context.metrics.moves == 2 * n * 4, "merge sort move count on sorted input");
    }
}

void test_aux_memory_estimates() {
    const std::size_t n = 1000;
    const Algorithm* merge = sortbench::find_algorithm("merge");
    const Algorithm* selection = sortbench::find_algorithm("selection");
    const Algorithm* quick = sortbench::find_algorithm("quick");

    expect(merge != nullptr && selection != nullptr && quick != nullptr,
           "registry contains merge, selection and quick");
    if (merge != nullptr) {
        expect(sortbench::aux_bytes_estimate(*merge, n) == n * sizeof(Value),
               "merge sort auxiliary memory is n * sizeof(Value)");
    }
    if (selection != nullptr) {
        expect(sortbench::aux_bytes_estimate(*selection, n) == 0,
               "selection sort auxiliary memory is O(1)");
    }
    if (quick != nullptr) {
        expect(sortbench::aux_bytes_estimate(*quick, n) > 0,
               "quick sort reports a non-zero stack estimate");
    }
}

} // namespace

int main() {
    test_is_sorted();
    test_dataset_generation();
    test_all_algorithms();
    test_duplicate_heavy_input();
    test_randomized_fuzz();
    test_large_sorted_input();
    test_measurement_counters();
    test_aux_memory_estimates();

    if (g_failures == 0) {
        std::cout << "All " << g_checks << " checks passed.\n";
        return 0;
    }
    std::cerr << g_failures << " of " << g_checks << " checks failed.\n";
    return 1;
}
