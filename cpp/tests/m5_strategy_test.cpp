#include "strategy.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    ++failures;
  }
}

void expect_action(const ytta::m5::Action& actual,
                   const ytta::m5::Action& expected,
                   const std::string& label) {
  expect(actual.type == expected.type, label + " type");
  expect(actual.cl_ord_id == expected.cl_ord_id, label + " cl_ord_id");
  expect(actual.side == expected.side, label + " side");
  expect(actual.price == expected.price, label + " price");
  expect(actual.qty == expected.qty, label + " qty");
  expect(actual.reason == expected.reason, label + " reason");
}

}  // namespace

int main() {
  using namespace ytta::m5;

  // Default skeleton is a liquidity taker. Market-making is an alternate M5
  // exercise and is intentionally not covered by this deterministic fixture.
  const std::array<Tick, 2> ticks{{
      Tick{1000, Side::Buy, 101, 1},
      Tick{2000, Side::Buy, 100, 1},
  }};
  const std::array<MdSnapshot, 2> snapshots{{
      MdSnapshot{0, 100, 0, 5},
      MdSnapshot{},
  }};
  const std::array<Action, 2> expected{{
      Action{ActionType::NewOrder, 1, Side::Buy, 100, 1, "take_ask"},
      Action{ActionType::Cancel, 1, Side::Buy, 0, 0, "done"},
  }};

  Strategy strategy;
  for (std::size_t i = 0; i < ticks.size(); ++i) {
    expect_action(strategy.on_tick(ticks[i], snapshots[i]), expected[i],
                  "action " + std::to_string(i + 1));
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }

  std::cout << "ytta_m5_tests ok\n";
  return EXIT_SUCCESS;
}
