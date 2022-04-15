#ifndef PARPT_IO_HPP
#define PARPT_IO_HPP

#include <string>
#include <sstream>

#include "Kokkos_Core.hpp"
#include "containers.hpp"
#include "spdlog/formatter.h"
#include "type_defs.hpp"
// maybe add this back in once I get fancier with versioning
// #include "version_info.hpp"

namespace particles {

// class for writing particle output
class ParticleIO {
 private:
  FILE* outfile;
  Params params;

 public:
  ParticleIO(std::string f, const Params& params);
  void print_params_summary();
  void write(const ko::View<Real**>& X, const ko::View<Real*>& mass,
             const int& i);
};

void print_version_info();

void read_params_input(const std::string& yaml_name);

}  // namespace particles

#endif
