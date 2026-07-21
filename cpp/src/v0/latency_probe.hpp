#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

namespace ytta::v0 {

class LatencyProbe {
 public:
  struct Summary {
    std::uint64_t count{0};
    std::int64_t p50_ns{0};
    std::int64_t p99_ns{0};
    std::int64_t ingest_p50_ns{0};
    std::int64_t ingest_p99_ns{0};
    std::int64_t decide_p50_ns{0};
    std::int64_t decide_p99_ns{0};
    std::int64_t execute_p50_ns{0};
    std::int64_t execute_p99_ns{0};
    std::uint64_t drops{0};
  };

  void set_drops(std::uint64_t drops) { drops_ = drops; }
  void add_drop() { ++drops_; }

  // Back-compat: record e2e only.
  void record_ns(std::int64_t delta_ns) { e2e_.push_back(delta_ns); }

  void record_stages(std::int64_t e2e_ns, std::int64_t ingest_ns,
                     std::int64_t decide_ns, std::int64_t execute_ns) {
    e2e_.push_back(e2e_ns);
    ingest_.push_back(ingest_ns);
    decide_.push_back(decide_ns);
    execute_.push_back(execute_ns);
  }

  Summary summarize() const {
    Summary s;
    s.count = e2e_.size();
    s.drops = drops_;
    percentile(e2e_, s.p50_ns, s.p99_ns);
    percentile(ingest_, s.ingest_p50_ns, s.ingest_p99_ns);
    percentile(decide_, s.decide_p50_ns, s.decide_p99_ns);
    percentile(execute_, s.execute_p50_ns, s.execute_p99_ns);
    return s;
  }

 private:
  static void percentile(const std::vector<std::int64_t>& samples,
                         std::int64_t& p50, std::int64_t& p99) {
    if (samples.empty()) {
      p50 = 0;
      p99 = 0;
      return;
    }
    auto sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    const auto idx = [&](double pct) -> std::size_t {
      if (sorted.size() == 1) {
        return 0;
      }
      return static_cast<std::size_t>(pct *
                                      static_cast<double>(sorted.size() - 1));
    };
    p50 = sorted[idx(0.50)];
    p99 = sorted[idx(0.99)];
  }

  std::vector<std::int64_t> e2e_;
  std::vector<std::int64_t> ingest_;
  std::vector<std::int64_t> decide_;
  std::vector<std::int64_t> execute_;
  std::uint64_t drops_{0};
};

inline std::int64_t steady_ns_since(
    std::chrono::steady_clock::time_point start) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::steady_clock::now() - start)
      .count();
}

inline std::int64_t steady_ns_between(
    std::chrono::steady_clock::time_point a,
    std::chrono::steady_clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}

}  // namespace ytta::v0
