#pragma once

#include <string>

namespace ytta::m4 {

class Pipeline {
 public:
  int run(const std::string& fixture_path, const std::string& out_path);
};

}  // namespace ytta::m4
