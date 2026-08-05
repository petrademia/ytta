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

  expect(p50 >= 40 && p50 <= 70, "p50 in plausible range for 10..100 ns");
  expect(p99 >= 80 && p99 <= 100, "p99 in plausible range for 10..100 ns");

  const std::string report = probe.report();
  expect(report.find("p50") != std::string::npos, "report contains p50");
  expect(report.find("p99") != std::string::npos, "report contains p99");

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "ytta_m6_tests ok\n";
  return EXIT_SUCCESS;
}
