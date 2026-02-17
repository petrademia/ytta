#include "thread_utils.h"
#include "thread.h"
#include "cycle_clock.h"
#include <chrono>
#include <cstdint>

namespace {
void dummyFunction(int a, int b, bool should_sleep) {
    std::cout << "dummyFunction(" << a << ", " << b << ")" << std::endl;
    std::cout << "dummyFunction output=" << a + b << std::endl;

    if (should_sleep) {
        std::cout << "dummyFunction sleeping..." << std::endl;
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(2s);
    }

    std::cout << "dummyFunction done." << std::endl;
}
}

int runThreadCreationDemo() {
    using namespace Common;
    using Clock = std::chrono::steady_clock;

    std::uint64_t t1_exec_ns = 0;
    std::uint64_t t2_exec_ns = 0;
    const auto demo_start_time = Clock::now();
    const auto demo_start_cycles = readCycles();
    const auto t1_start_time = Clock::now();
    const auto t1_start_cycles = readCycles();
    auto t1 = createAndStartThread(-1, "dummyFunction1", [&] {
        const auto exec_start = Clock::now();
        dummyFunction(12, 21, false);
        const auto exec_end = Clock::now();
        t1_exec_ns = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(exec_end - exec_start).count());
    });
    const auto t1_end_cycles = readCycles();
    const auto t1_end_time = Clock::now();
    const auto t2_start_time = Clock::now();
    const auto t2_start_cycles = readCycles();
    auto t2 = createAndStartThread(-1, "dummyFunction2", [&] {
        const auto exec_start = Clock::now();
        dummyFunction(15, 51, true);
        const auto exec_end = Clock::now();
        t2_exec_ns = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(exec_end - exec_start).count());
    });
    const auto t2_end_cycles = readCycles();
    const auto t2_end_time = Clock::now();

    if (!t1 || !t2) {
        std::cerr << "Failed to start one or more threads." << std::endl;
        if (t1 && t1->joinable()) {
            t1->join();
        }
        if (t2 && t2->joinable()) {
            t2->join();
        }
        return 1;
    }

    std::cout << "main waiting for threads to be done." << std::endl;
    t1->join();
    t2->join();
    const auto demo_end_time = Clock::now();
    const auto demo_end_cycles = readCycles();
    const auto counter_freq = readCounterFrequency();
    const auto t1_launch_ns = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(t1_end_time - t1_start_time).count());
    const auto t2_launch_ns = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(t2_end_time - t2_start_time).count());
    const auto total_wall_ns = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(demo_end_time - demo_start_time).count());
    const auto t1_launch_cycles = cycleDelta(t1_start_cycles, t1_end_cycles);
    const auto t2_launch_cycles = cycleDelta(t2_start_cycles, t2_end_cycles);
    const auto total_cycles = cycleDelta(demo_start_cycles, demo_end_cycles);
    std::cout << "t1 launch ns=" << t1_launch_ns << std::endl;
    std::cout << "t2 launch ns=" << t2_launch_ns << std::endl;
    std::cout << "t1 execution ns=" << t1_exec_ns << std::endl;
    std::cout << "t2 execution ns=" << t2_exec_ns << std::endl;
    std::cout << "total wall ns=" << total_wall_ns << std::endl;
    std::cout << "total wall us=" << (total_wall_ns / 1000ULL) << std::endl;
    std::cout << "t1 launch cycles=" << t1_launch_cycles << std::endl;
    std::cout << "t2 launch cycles=" << t2_launch_cycles << std::endl;
    std::cout << "total cycles=" << total_cycles << std::endl;
    if (counter_freq != 0) {
        const auto total_ns = total_cycles * 1000000000ULL / counter_freq;
        std::cout << "total approx ns=" << total_ns << std::endl;
    }
    std::cout << "main exiting." << std::endl;
    return 0;
}
