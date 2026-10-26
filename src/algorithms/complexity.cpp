#include "algorithms/algorithm.hpp"

#include <cstddef>

namespace sortbench {

std::string_view to_string(Complexity complexity) noexcept {
    switch (complexity) {
    case Complexity::Quadratic:
        return "O(n^2)";
    case Complexity::Linearithmic:
        return "O(n log n)";
    case Complexity::Depends:
        return "depends";
    }
    return "unknown";
}

std::string_view to_string(AuxMemory memory) noexcept {
    switch (memory) {
    case AuxMemory::Constant:
        return "O(1)";
    case AuxMemory::Logarithmic:
        return "O(log n)";
    case AuxMemory::Linear:
        return "O(n)";
    }
    return "unknown";
}

std::size_t aux_bytes_estimate(const Algorithm& algorithm, std::size_t n) noexcept {
    switch (algorithm.aux_memory) {
    case AuxMemory::Constant:
        return 0;
    case AuxMemory::Linear:
        return n * sizeof(Value);
    case AuxMemory::Logarithmic: {
        if (n < 2) {
            return 0;
        }
        // Two frames per recursion level, expected depth ~log2(n).
        std::size_t levels = 0;
        for (std::size_t remaining = n; remaining > 1; remaining = (remaining + 1) / 2) {
            ++levels;
        }
        return 2 * levels * sizeof(std::size_t);
    }
    }
    return 0;
}

} // namespace sortbench
