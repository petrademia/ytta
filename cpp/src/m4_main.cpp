#include "pipeline.hpp"

#include <string>

int main(int argc, char* argv[]) {
  std::string fixture_path;
  std::string out_path;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--fixture" && i + 1 < argc) {
      fixture_path = argv[++i];
    } else if (arg == "--out" && i + 1 < argc) {
      out_path = argv[++i];
    } else {
      return 2;
    }
  }

  if (fixture_path.empty() || out_path.empty()) {
    return 2;
  }

  ytta::m4::Pipeline pipeline;
  return pipeline.run(fixture_path, out_path);
}
