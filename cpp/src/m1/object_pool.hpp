#pragma once
#include <cstddef>

namespace ytta::m1 {

template <typename T>
class ObjectPool {
 public:
  T* acquire(const T&) { return nullptr; }  // stub
  void release(T*) {}
  std::size_t free_count() const { return 0; }
  std::size_t living() const { return 0; }
};

}  // namespace ytta::m1
