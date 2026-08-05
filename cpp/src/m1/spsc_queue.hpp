#pragma once
#include <cstddef>
#include <optional>
#include <array>

namespace ytta::m1 {

template <typename T, std::size_t N>
class SpscQueue {
 public:
  bool try_push(const T&) { return false; }  // stub: always full
  bool try_pop(T&) { return false; }         // stub: always empty
};

}  // namespace ytta::m1
