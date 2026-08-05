#include "latency_probe.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    ++failures;
  }
}

}  // namespace

int main() {
  using ytta::m6::LatencyProbe;

  LatencyProbe probe;
  for (std::uint64_t ns = 10; ns <= 100; ns += 10) {
    probe.record(ns);
  }

  const auto p50 = probe.p50();
  const auto p99 = probe.p99();

  // Nearest-rank: p50 rank ceil(0.5*n), p99 rank ceil(0.99*n).
  expect(p50 >= 50 && p50 <= 60, "p50 in [50,60] for 10..100 ns");
  expect(p99 >= 90 && p99 <= 100, "p99 in [90,100] for 10..100 ns");

  const std::string report = probe.report();
  expect(report.find("p50_ns") != std::string::npos,
         "report contains p50_ns");
  expect(report.find("p99_ns") != std::string::npos,
         "report contains p99_ns");
  expect(report.find("count") != std::string::npos, "report contains count");

  LatencyProbe skew_probe;
  for (const std::uint64_t ns : {1ULL, 1ULL, 1ULL, 1000ULL}) {
    skew_probe.record(ns);
  }

  const auto skew_p50 = skew_probe.p50();
  const auto skew_p99 = skew_probe.p99();

  expect(skew_p50 <= 10, "p50 near 1 for skewed {1,1,1,1000}");
  expect(skew_p99 >= 500, "p99 near 1000 for skewed {1,1,1,1000}");

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "ytta_m6_tests ok\n";
  return EXIT_SUCCESS;
}
