#pragma once

#include <cstdint>

#if defined(__x86_64__) || defined(_M_X64)
#include <x86intrin.h>
#endif
#if defined(__APPLE__)
#include <mach/mach_time.h>
#endif

namespace Common {
    inline std::uint64_t readCycles() noexcept {
#if defined(__x86_64__) || defined(_M_X64)
        return __rdtsc();
#elif defined(__APPLE__) && defined(__aarch64__)
        return mach_absolute_time();
#elif defined(__aarch64__)
        std::uint64_t value = 0;
        asm volatile("isb; mrs %0, cntvct_el0" : "=r"(value));
        return value;
#else
        return 0;
#endif
    }

    inline std::uint64_t readCounterFrequency() noexcept {
#if defined(__APPLE__) && defined(__aarch64__)
        mach_timebase_info_data_t info{};
        if (mach_timebase_info(&info) != KERN_SUCCESS || info.numer == 0) {
            return 0;
        }
        return (1000000000ULL * info.denom) / info.numer;
#elif defined(__aarch64__)
        std::uint64_t value = 0;
        asm volatile("mrs %0, cntfrq_el0" : "=r"(value));
        return value;
#else
        return 0;
#endif
    }

    inline std::uint64_t cycleDelta(std::uint64_t start, std::uint64_t end) noexcept {
        if (end >= start) {
            return end - start;
        }
        return 0;
    }
}
