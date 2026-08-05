#include "pipeline.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr const char* kFixtureRel = "shared/fixtures/v0/ticks.ndjson";

std::string resolve_fixture_path(int argc, char* argv[]) {
  std::vector<std::filesystem::path> candidates;
  candidates.push_back(std::filesystem::current_path() / kFixtureRel);
  candidates.push_back(std::filesystem::current_path() / ".." / kFixtureRel);
  candidates.push_back(std::filesystem::current_path() / "../.." / kFixtureRel);

  if (argc > 0 && argv[0] != nullptr) {
    const auto exe_dir = std::filesystem::path(argv[0]).parent_path();
    candidates.push_back(std::filesystem::weakly_canonical(exe_dir / "../..") /
                           kFixtureRel);
  }

  const auto source_root =
      std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  candidates.push_back(source_root / kFixtureRel);

  for (const auto& candidate : candidates) {
    if (std::filesystem::exists(candidate)) {
      return candidate.string();
    }
  }

  return kFixtureRel;
}

}  // namespace

int main(int argc, char* argv[]) {
  const std::string fixture_path = resolve_fixture_path(argc, argv);
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
