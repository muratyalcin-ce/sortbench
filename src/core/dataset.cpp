#include "core/dataset.hpp"

#include <algorithm>
#include <array>
#include <random>
#include <stdexcept>

namespace sortbench {
namespace {

const std::array<Distribution, 4> kDistributions{
    Distribution::Random,
    Distribution::Sorted,
    Distribution::ReverseSorted,
    Distribution::NearlySorted,
};

} // namespace

const std::vector<Distribution>& all_distributions() {
    static const std::vector<Distribution> distributions(kDistributions.begin(),
                                                         kDistributions.end());
    return distributions;
}

std::string_view to_string(Distribution distribution) noexcept {
    switch (distribution) {
    case Distribution::Random:
        return "random";
    case Distribution::Sorted:
        return "sorted";
    case Distribution::ReverseSorted:
        return "reverse";
    case Distribution::NearlySorted:
        return "nearly_sorted";
    }
    return "unknown";
}

std::optional<Distribution> parse_distribution(std::string_view text) noexcept {
    if (text == "random" || text == "rand" || text == "rnd") {
        return Distribution::Random;
    }
    if (text == "sorted" || text == "ascending" || text == "asc") {
        return Distribution::Sorted;
    }
    if (text == "reverse" || text == "reversed" || text == "descending" || text == "desc") {
        return Distribution::ReverseSorted;
    }
    if (text == "nearly" || text == "nearly_sorted" || text == "partially_sorted" ||
        text == "partial") {
        return Distribution::NearlySorted;
    }
    return std::nullopt;
}

std::vector<Value> make_dataset(Distribution distribution, std::size_t n, std::uint64_t seed) {
    std::vector<Value> data(n);

    switch (distribution) {
    case Distribution::Random: {
        std::mt19937_64 rng(seed);
        if (n > 1) {
            std::uniform_int_distribution<std::int64_t> pick(0, static_cast<std::int64_t>(n) - 1);
            for (Value& value : data) {
                value = static_cast<Value>(pick(rng));
            }
        }
        break;
    }
    case Distribution::Sorted:
        for (std::size_t i = 0; i < n; ++i) {
            data[i] = static_cast<Value>(i);
        }
        break;
    case Distribution::ReverseSorted:
        for (std::size_t i = 0; i < n; ++i) {
            data[i] = static_cast<Value>(n - 1 - i);
        }
        break;
    case Distribution::NearlySorted: {
        for (std::size_t i = 0; i < n; ++i) {
            data[i] = static_cast<Value>(i);
        }
        if (n > 1) {
            std::mt19937_64 rng(seed);
            std::uniform_int_distribution<std::size_t> pick(0, n - 1);
            const std::size_t swaps = std::max<std::size_t>(1, n / 10);
            for (std::size_t k = 0; k < swaps; ++k) {
                const std::size_t i = pick(rng);
                const std::size_t j = pick(rng);
                std::swap(data[i], data[j]);
            }
        }
        break;
    }
    }

    return data;
}

} // namespace sortbench
