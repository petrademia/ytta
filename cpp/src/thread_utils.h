#pragma once

#include <future>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#ifdef __linux__
#include <pthread.h>
#include <sched.h>
#endif

namespace Common {
    inline bool setThreadCore(int core_id) noexcept {
#ifdef __linux__
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(core_id, &cpu_set);
        return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set) == 0);
#else
        (void)core_id;
        return true;
#endif
    }

    template<typename T, typename... A>
    std::optional<std::thread> createAndStartThread(int core_id, const std::string &name, T &&func, A &&... args) {
        using Fn = std::decay_t<T>;
        using ArgsTuple = std::tuple<std::decay_t<A>...>;
        Fn fn(std::forward<T>(func));
        ArgsTuple bound_args(std::forward<A>(args)...);
        std::promise<bool> startup_promise;
        auto startup_future = startup_promise.get_future();

        auto thread_body = [core_id,
                            &name,
                            fn = std::move(fn),
                            bound_args = std::move(bound_args),
                            startup_promise = std::move(startup_promise)]() mutable {
            if (core_id >= 0) {
#ifdef __linux__
                if (!setThreadCore(core_id)) {
                    startup_promise.set_value(false);
                    std::cerr << "Failed to set core affinity for " << name << " to " << core_id << std::endl;
                    return;
                }
                std::cout << "Set core affinity for " << name << " to " << core_id << std::endl;
#else
                std::cout << "Thread pinning not supported on this OS. Skipping core: " << core_id << std::endl;
#endif
            }

            startup_promise.set_value(true);
            std::apply(
                [&]<typename... U>(U &&... unpacked_args) {
                    std::invoke(fn, std::forward<U>(unpacked_args)...);
                },
                std::move(bound_args));
        };

        std::thread t(std::move(thread_body));
        if (!startup_future.get()) {
            if (t.joinable()) {
                t.join();
            }
            return std::nullopt;
        }

        return std::move(t);
    }
}
