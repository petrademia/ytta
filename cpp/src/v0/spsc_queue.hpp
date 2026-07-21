#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace ytta::v0 {

// Bounded SPSC ring. Capacity must be a power of two.
template <typename T, std::size_t Capacity>
class SpscQueue {
  static_assert(Capacity >= 2, "capacity must be >= 2");
  static_assert((Capacity & (Capacity - 1)) == 0, "capacity must be power of two");

 public:
  static constexpr std::size_t capacity() { return Capacity; }

  bool try_push(const T& value) {
    const std::size_t w = write_.load(std::memory_order_relaxed);
    const std::size_t r = read_.load(std::memory_order_acquire);
    if (w - r >= Capacity) {
      return false;
    }
    buf_[w & (Capacity - 1)] = value;
    write_.store(w + 1, std::memory_order_release);
    return true;
  }

  bool try_push(T&& value) {
    const std::size_t w = write_.load(std::memory_order_relaxed);
    const std::size_t r = read_.load(std::memory_order_acquire);
    if (w - r >= Capacity) {
      return false;
    }
    buf_[w & (Capacity - 1)] = std::move(value);
    write_.store(w + 1, std::memory_order_release);
    return true;
  }

  bool try_pop(T& out) {
    const std::size_t r = read_.load(std::memory_order_relaxed);
    const std::size_t w = write_.load(std::memory_order_acquire);
    if (r == w) {
      return false;
    }
    out = std::move(buf_[r & (Capacity - 1)]);
    read_.store(r + 1, std::memory_order_release);
    return true;
  }

  std::size_t size_approx() const {
    const std::size_t w = write_.load(std::memory_order_acquire);
    const std::size_t r = read_.load(std::memory_order_acquire);
    return w - r;
  }

  bool empty_approx() const { return size_approx() == 0; }

 private:
  std::array<T, Capacity> buf_{};
  alignas(64) std::atomic<std::size_t> write_{0};
  alignas(64) std::atomic<std::size_t> read_{0};
};

}  // namespace ytta::v0
