#pragma once

#include <cstddef>
#include <deque>
#include <utility>
#include <vector>

namespace ytta::v0 {

// Simple free-list pool. Nodes live in a deque so pointers stay stable.
template <typename T>
class ObjectPool {
 public:
  explicit ObjectPool(std::size_t prewarm = 0) {
    for (std::size_t i = 0; i < prewarm; ++i) {
      nodes_.emplace_back();
      free_.push_back(&nodes_.back());
    }
  }

  template <typename... Args>
  T* acquire(Args&&... args) {
    if (!free_.empty()) {
      T* p = free_.back();
      free_.pop_back();
      *p = T(std::forward<Args>(args)...);
      return p;
    }
    nodes_.emplace_back(std::forward<Args>(args)...);
    return &nodes_.back();
  }

  void release(T* p) {
    if (p == nullptr) {
      return;
    }
    free_.push_back(p);
  }

  std::size_t living() const { return nodes_.size(); }
  std::size_t free_count() const { return free_.size(); }

 private:
  std::deque<T> nodes_;
  std::vector<T*> free_;
};

}  // namespace ytta::v0
