#ifndef PARPT_IO_HPP
#define PARPT_IO_HPP

#include <sstream>
#include <string>

#include "Kokkos_Core.hpp"
#include "containers.hpp"
#include "spdlog/formatter.h"
#include "type_defs.hpp"
#include "yaml-cpp/yaml.h"
// maybe add this back in once I get fancier with versioning
// #include "version_info.hpp"

namespace particles {

// class for writing particle output
class ParticleIO {
 private:
  FILE* outfile;

 public:
  ParticleIO(Params& params, const std::string& yaml_name);
  ParticleIO() = default;
  void set_positions(const Params& params, const std::string& yaml_name,
                            ko::View<Real**>& X);
  void enumerate_IC(Params& params, std::string& IC_str, const YAML::Node& yml,
                    const bool& space);
  void enumerate_seed_type(Params& params, std::string& seed_str,
                           const YAML::Node& yml);
  void set_seed_val(Params& params, const YAML::Node& yml);
  void print_params_summary(const Params& params);
  void write(const Params& params, const ko::View<Real**>& X,
             const ko::View<Real*>& mass, const int& i);
};

void print_version_info();

}  // namespace particles

#endif
