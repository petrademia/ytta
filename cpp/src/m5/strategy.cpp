#include "strategy.hpp"

namespace ytta::m5 {

Action Strategy::on_tick(const Tick&, const MdSnapshot&) {
  return Action{};
}

}  // namespace ytta::m5
