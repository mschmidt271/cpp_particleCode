#include "version_info.hpp"

namespace utils {

std::string print_version_info() {
  std::ostringstream ss;
  ss << "  1D Particle Tracking With Random Walks and Mass Transfer\n";
  ss << "              Version 0.0.1--NOT FOR SCIENCE              \n";
  return ss.str();
}
}  // namespace utils
