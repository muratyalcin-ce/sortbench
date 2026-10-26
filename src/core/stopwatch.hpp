#pragma once

#include <chrono>

namespace sortbench {

/// High-resolution wall-clock stopwatch.
///
/// std::chrono::steady_clock maps to QueryPerformanceCounter on Windows and to
/// CLOCK_MONOTONIC on POSIX, so measurements are monotonic and portable.
class Stopwatch {
public:
    Stopwatch() noexcept : start_(Clock::now()) {}

    void restart() noexcept { start_ = Clock::now(); }

    [[nodiscard]] double elapsed_ms() const noexcept {
        const auto end = Clock::now();
        return std::chrono::duration<double, std::milli>(end - start_).count();
    }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point start_;
};

} // namespace sortbench
