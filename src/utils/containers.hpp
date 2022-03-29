#ifndef CONTAINERS_HPP
#define CONTAINERS_HPP

#include "Kokkos_Core.hpp"
#include "type_defs.hpp"
#include "version_info.hpp"
#include "yaml-cpp/yaml.h"

namespace particles {

enum IC_enum_space { point_loc, uniform, equi, hat };
enum IC_enum_mass { point_mass, heaviside, gaussian };

// class holding simulation parameters
class Params {
 public:
  int Np, nSteps;
  Real X0_mass, X0_space, maxT, dt, D, pctRW, denom, cdist_coeff, cutdist,
      hat_pct;
  std::string IC_str_space, IC_str_mass;
  std::vector<Real> omega;
  std::string pFile;
  IC_enum_space IC_type_space;
  IC_enum_mass IC_type_mass;
  void set_values(const std::string& yaml_name);
  void print_summary();
  Params() = default;
  Params(const std::string& yaml_name);
  void enumerate_IC(std::string& IC_str, const YAML::Node& yml,
                    const bool& space);
};

}  // namespace particles

#endif
