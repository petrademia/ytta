#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ytta::m3 {

std::vector<std::uint8_t> frame(const std::string& payload);
bool try_deframe(std::string& buffer, std::string& payload);

}  // namespace ytta::m3
