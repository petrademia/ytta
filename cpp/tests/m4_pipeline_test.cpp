#include "pipeline.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

int main() {
  const std::string fixture_path = "shared/fixtures/v0/ticks.ndjson";
  const auto out_path =
      std::filesystem::temp_directory_path() / "ytta_m4_pipeline_test.ndjson";
  std::filesystem::remove(out_path);

  ytta::m4::Pipeline pipeline;
  const int status = pipeline.run(fixture_path, out_path.string());

  if (status != 0) {
    std::cerr << "FAIL: pipeline should return 0\n";
    return EXIT_FAILURE;
  }
  if (!std::filesystem::exists(out_path)) {
    std::cerr << "FAIL: pipeline should create output file\n";
    return EXIT_FAILURE;
  }

  std::filesystem::remove(out_path);
  std::cout << "ytta_m4_tests ok\n";
  return EXIT_SUCCESS;
}
