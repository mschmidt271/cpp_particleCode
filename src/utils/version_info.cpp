#include "version_info.hpp"

namespace utils {

void print_version_info() {
  // note: the format statement means the text will be centered in 60 total spaces
  fmt::print("{:^60}\n", "1D Particle Tracking With Random Walks and Mass Transfer");
  fmt::print("{:^60}\n", "Version 0.0.1--NOT FOR SCIENCE");
}
}  // namespace utils
