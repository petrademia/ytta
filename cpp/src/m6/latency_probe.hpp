#pragma once

#include <cstdint>
#include <string>

namespace ytta::m6 {

class LatencyProbe {
 public:
  void record(std::uint64_t /*e2e_ns*/) {}

  std::uint64_t p50() const { return 0; }

  std::uint64_t p99() const { return 0; }

  std::string report() const { return {}; }
};

}  // namespace ytta::m6
