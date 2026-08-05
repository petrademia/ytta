#pragma once
#include <cstdint>

namespace ytta::m1 {

struct MonoClock {
  // Stub: returns 0 always (breaks monotonicity / usefulness checks).
  static std::uint64_t now_ns() { return 0; }
};

}  // namespace ytta::m1
