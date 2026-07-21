#include "object_pool.hpp"
#include "spsc_queue.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool cond, const char* msg) {
  if (!cond) {
    std::cerr << "FAIL: " << msg << "\n";
    ++failures;
  }
}

}  // namespace

int main() {
  using ytta::v0::ObjectPool;
  using ytta::v0::SpscQueue;

  {
    SpscQueue<int, 4> q;
    expect(q.try_push(1), "push 1");
    expect(q.try_push(2), "push 2");
    expect(q.try_push(3), "push 3");
    expect(q.try_push(4), "push 4");
    expect(!q.try_push(5), "full rejects");
    int v = 0;
    expect(q.try_pop(v) && v == 1, "fifo 1");
    expect(q.try_pop(v) && v == 2, "fifo 2");
    expect(q.try_pop(v) && v == 3, "fifo 3");
    expect(q.try_pop(v) && v == 4, "fifo 4");
    expect(!q.try_pop(v), "empty rejects");
  }

  {
    SpscQueue<std::string, 2> q;
    expect(q.try_push("a"), "push a");
    expect(q.try_push("b"), "push b");
    expect(!q.try_push("c"), "cap 2 full");
    std::string s;
    expect(q.try_pop(s) && s == "a", "pop a");
    expect(q.try_push("c"), "reuse slot");
    expect(q.try_pop(s) && s == "b", "pop b");
    expect(q.try_pop(s) && s == "c", "pop c");
  }

  {
    ObjectPool<int> pool;
    int* a = pool.acquire(7);
    int* b = pool.acquire(8);
    expect(a && b && a != b && *a == 7 && *b == 8, "acquire distinct");
    pool.release(a);
    int* c = pool.acquire(9);
    expect(c == a && *c == 9, "reuse freed slot");
    pool.release(b);
    pool.release(c);
    expect(pool.free_count() == 2, "two free");
    for (int i = 0; i < 1000; ++i) {
      int* p = pool.acquire(i);
      expect(p != nullptr, "loop acquire");
      pool.release(p);
    }
    expect(pool.living() <= 2, "no unbounded growth on reuse loop");
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "ytta_v0_spsc_tests ok\n";
  return EXIT_SUCCESS;
}
