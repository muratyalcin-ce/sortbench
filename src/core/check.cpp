#include "core/check.hpp"

#include <cstddef>

namespace sortbench {

bool is_sorted(const std::vector<Value>& data) noexcept {
    for (std::size_t i = 1; i < data.size(); ++i) {
        if (data[i] < data[i - 1]) {
            return false;
        }
    }
    return true;
}

} // namespace sortbench
