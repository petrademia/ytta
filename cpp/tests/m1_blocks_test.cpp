#include "spsc_queue.hpp"
#include "object_pool.hpp"
#include "mono_clock.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

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
  using ytta::m1::ObjectPool;
  using ytta::m1::SpscQueue;
  using ytta::m1::MonoClock;

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

  {
    auto t0 = MonoClock::now_ns();
    auto t1 = MonoClock::now_ns();
    expect(t1 >= t0, "monotonic non-decreasing");
    expect(t0 != 0 || t1 != 0, "clock not stuck at zero forever");
    // Allow equal values; require at least one non-zero after a tiny sleep.
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    auto t2 = MonoClock::now_ns();
    expect(t2 >= t1 && t2 > 0, "clock advances past zero");
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "ytta_m1_tests ok\n";
  return EXIT_SUCCESS;
}
