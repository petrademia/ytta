#include "net/framing.hpp"

namespace ytta::m3 {

std::vector<std::uint8_t> frame(const std::string&) {
  return {};
}

bool try_deframe(std::string&, std::string&) {
  return false;
}

}  // namespace ytta::m3
