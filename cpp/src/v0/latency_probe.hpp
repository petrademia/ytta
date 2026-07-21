#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

namespace ytta::v0 {

class LatencyProbe {
 public:
  void record_ns(std::int64_t delta_ns) { samples_.push_back(delta_ns); }

  struct Summary {
    std::uint64_t count{0};
    std::int64_t p50_ns{0};
    std::int64_t p99_ns{0};
  };

  Summary summarize() const {
    Summary s;
    s.count = samples_.size();
    if (samples_.empty()) {
      return s;
    }
    auto sorted = samples_;
    std::sort(sorted.begin(), sorted.end());
    const auto idx = [&](double pct) -> std::size_t {
      if (sorted.size() == 1) {
        return 0;
      }
      auto i = static_cast<std::size_t>(pct * static_cast<double>(sorted.size() - 1));
      return i;
    };
    s.p50_ns = sorted[idx(0.50)];
    s.p99_ns = sorted[idx(0.99)];
    return s;
  }

 private:
  std::vector<std::int64_t> samples_;
};

inline std::int64_t steady_ns_since(
    std::chrono::steady_clock::time_point start) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::steady_clock::now() - start)
      .count();
}

}  // namespace ytta::v0
