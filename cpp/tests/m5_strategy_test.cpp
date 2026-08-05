#include "strategy.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr const char* kTicksFixture =
    "shared/fixtures/m5/ticks_taker.ndjson";
constexpr const char* kActionsFixture =
    "shared/fixtures/m5/expected_actions_taker.ndjson";

int failures = 0;

void expect(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    ++failures;
  }
}

std::filesystem::path resolve_fixture(const std::string& relative_path) {
  const auto source_root =
      std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  const std::vector<std::filesystem::path> candidates{
      std::filesystem::current_path() / relative_path,
      std::filesystem::current_path() / ".." / relative_path,
      std::filesystem::current_path() / "../.." / relative_path,
      source_root / relative_path,
  };

  for (const auto& candidate : candidates) {
    if (std::filesystem::exists(candidate)) {
      return candidate;
    }
  }
  throw std::runtime_error("fixture not found: " + relative_path);
}

std::optional<std::size_t> field_start(const std::string& line,
                                       const std::string& key) {
  const std::string token = "\"" + key + "\"";
  std::size_t search_from = 0;
  while (true) {
    const auto key_pos = line.find(token, search_from);
    if (key_pos == std::string::npos) {
      return std::nullopt;
    }
    const auto colon =
        line.find_first_not_of(" \t", key_pos + token.size());
    if (colon != std::string::npos && line[colon] == ':') {
      return line.find_first_not_of(" \t", colon + 1);
    }
    search_from = key_pos + token.size();
  }
}

std::optional<std::string> string_field(const std::string& line,
                                        const std::string& key) {
  const auto start = field_start(line, key);
  if (!start) {
    return std::nullopt;
  }
  if (*start == std::string::npos || line[*start] != '"') {
    throw std::runtime_error("field is not a string: " + key);
  }
  const auto end = line.find('"', *start + 1);
  if (end == std::string::npos) {
    throw std::runtime_error("unterminated string field: " + key);
  }
  return line.substr(*start + 1, end - *start - 1);
}

std::optional<std::int64_t> int_field(const std::string& line,
                                      const std::string& key) {
  const auto start = field_start(line, key);
  if (!start) {
    return std::nullopt;
  }
  if (*start == std::string::npos) {
    throw std::runtime_error("missing integer value: " + key);
  }
  return std::stoll(line.substr(*start));
}

std::string require_string(const std::string& line, const std::string& key) {
  const auto value = string_field(line, key);
  if (!value) {
    throw std::runtime_error("missing string field: " + key);
  }
  return *value;
}

std::int64_t require_int(const std::string& line, const std::string& key) {
  const auto value = int_field(line, key);
  if (!value) {
    throw std::runtime_error("missing integer field: " + key);
  }
  return *value;
}

ytta::m2::Side parse_side(const std::string& value) {
  if (value == "BUY") {
    return ytta::m2::Side::Buy;
  }
  if (value == "SELL") {
    return ytta::m2::Side::Sell;
  }
  throw std::runtime_error("unknown side: " + value);
}

std::vector<ytta::m5::Tick> load_ticks(
    const std::filesystem::path& fixture_path) {
  std::ifstream fixture(fixture_path);
  if (!fixture) {
    throw std::runtime_error("cannot open fixture: " + fixture_path.string());
  }

  std::vector<ytta::m5::Tick> ticks;
  std::string line;
  while (std::getline(fixture, line)) {
    if (line.empty()) {
      continue;
    }
    if (require_string(line, "type") != "tick") {
      throw std::runtime_error("unexpected record in tick fixture");
    }
    ticks.push_back(ytta::m5::Tick{
        static_cast<std::uint64_t>(require_int(line, "ts_ns")),
        parse_side(require_string(line, "side")),
        require_int(line, "price"),
        require_int(line, "qty"),
    });
  }
  return ticks;
}

std::vector<ytta::m5::Action> load_actions(
    const std::filesystem::path& fixture_path) {
  std::ifstream fixture(fixture_path);
  if (!fixture) {
    throw std::runtime_error("cannot open fixture: " + fixture_path.string());
  }

  std::vector<ytta::m5::Action> actions;
  std::string line;
  while (std::getline(fixture, line)) {
    if (line.empty()) {
      continue;
    }
    if (require_string(line, "type") != "action") {
      throw std::runtime_error("unexpected record in action fixture");
    }

    ytta::m5::Action action;
    action.ts_ns = static_cast<std::uint64_t>(require_int(line, "ts_ns"));
    action.cl_ord_id =
        static_cast<std::uint64_t>(require_int(line, "cl_ord_id"));
    action.reason = require_string(line, "reason");

    const auto action_type = require_string(line, "action");
    if (action_type == "NEW_ORDER") {
      action.type = ytta::m5::ActionType::NewOrder;
      action.side = parse_side(require_string(line, "side"));
      action.price = require_int(line, "price");
      action.qty = require_int(line, "qty");
    } else if (action_type == "CANCEL") {
      action.type = ytta::m5::ActionType::Cancel;
    } else {
      throw std::runtime_error("unknown action: " + action_type);
    }
    actions.push_back(action);
  }
  return actions;
}

void expect_action(const ytta::m5::Action& actual,
                   const ytta::m5::Action& expected,
                   const std::string& label) {
  expect(actual.type == expected.type, label + " type");
  expect(actual.ts_ns == expected.ts_ns, label + " ts_ns");
  expect(actual.cl_ord_id == expected.cl_ord_id, label + " cl_ord_id");
  if (expected.type == ytta::m5::ActionType::NewOrder) {
    expect(actual.side == expected.side, label + " side");
    expect(actual.price == expected.price, label + " price");
    expect(actual.qty == expected.qty, label + " qty");
  }
  expect(actual.reason == expected.reason, label + " reason");
}

}  // namespace

int main() {
  using namespace ytta::m5;

  try {
    // Default skeleton is a liquidity taker. Market-making is an alternate M5
    // exercise and is intentionally not covered by this deterministic fixture.
    const auto ticks = load_ticks(resolve_fixture(kTicksFixture));
    const auto expected = load_actions(resolve_fixture(kActionsFixture));
    const std::vector<MdSnapshot> snapshots{
      MdSnapshot{0, 100, 0, 5},
      MdSnapshot{},
    };

    expect(ticks.size() == snapshots.size(), "tick fixture size");
    expect(expected.size() == ticks.size(), "action fixture size");
    if (ticks.size() == snapshots.size() && expected.size() == ticks.size()) {
      Strategy strategy;
      for (std::size_t i = 0; i < ticks.size(); ++i) {
        expect_action(strategy.on_tick(ticks[i], snapshots[i]), expected[i],
                      "action " + std::to_string(i + 1));
      }
    }
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << "\n";
    return EXIT_FAILURE;
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }

  std::cout << "ytta_m5_tests ok\n";
  return EXIT_SUCCESS;
}
