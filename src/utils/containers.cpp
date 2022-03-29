#include "containers.hpp"

namespace particles {

Params::Params(const std::string& yaml_name) {
  ko::Profiling::pushRegion("in params constructor");
  YAML::Node file_params = YAML::LoadFile(yaml_name);

  Np = file_params["Np"].as<int>();
  IC_str_space =
      file_params["initial_condition"]["space"]["type"].as<std::string>();
  // true if enumerating spatial IC
  enumerate_IC(IC_str_space, file_params, true);
  IC_str_mass =
      file_params["initial_condition"]["mass"]["type"].as<std::string>();
  enumerate_IC(IC_str_mass, file_params, false);
  for (auto iter : file_params["omega"]) {
    omega.push_back(iter.as<Real>());
  }
  maxT = file_params["maxT"].as<Real>();
  dt = file_params["dt"].as<Real>();
  D = file_params["D"].as<Real>();
  pctRW = file_params["pctRW"].as<Real>();
  denom = 4 * D * (1.0 - pctRW) * dt;
  cdist_coeff = file_params["cdist_coeff"].as<Real>();
  cutdist = cdist_coeff * sqrt(denom);
  pFile = file_params["pFile"].as<std::string>();
  nSteps = ceil(maxT / dt);
  ko::Profiling::popRegion();
}

void Params::enumerate_IC(std::string& IC_str, const YAML::Node& yml,
                          const bool& space) {
  transform(IC_str.begin(), IC_str.end(), IC_str.begin(), ::tolower);
  if (space) {
    if (IC_str.compare("point") == 0) {
      IC_type_space = point_loc;
      X0_space = yml["initial_condition"]["space"]["X0"].as<Real>();
    } else if (IC_str.compare("uniform") == 0) {
      IC_type_space = uniform;
    } else if (IC_str.compare("equi") == 0) {
      IC_type_space = equi;
    } else if (IC_str.compare("hat") == 0) {
      IC_type_space = hat;
      hat_pct = yml["initial_condition"]["space"]["hat_pct"].as<Real>();
    } else {
      fmt::print(stderr, "ERROR: Unsupported initial condition type: {}\n",
                 &IC_str[0]);
      exit(1);
    }
  } else {
    if (IC_str.compare("point") == 0) {
      IC_type_mass = point_mass;
      X0_mass = yml["initial_condition"]["mass"]["X0"].as<Real>();
    } else if (IC_str.compare("heaviside") == 0) {
      IC_type_mass = heaviside;
      X0_mass = yml["initial_condition"]["mass"]["X0"].as<Real>();
    } else if (IC_str.compare("gaussian") == 0) {
      IC_type_mass = gaussian;
      X0_mass = yml["initial_condition"]["mass"]["X0"].as<Real>();
    } else {
      fmt::print(stderr, "ERROR: Unsupported initial condition type: {}\n",
                 &IC_str[0]);
      exit(1);
    }
  }
}

void Params::print_summary() {
  fmt::print("{}\n",
             "************************************************************");
  utils::print_version_info();
  fmt::print("{}\n",
             "************************************************************");
  fmt::print("Np = {}\n", Np);
  fmt::print("omega = [{}, {}]\n", omega[0], omega[1]);
  fmt::print("IC type (space) = {}\n", IC_str_space);
  fmt::print("IC type (mass) = {}\n", IC_str_mass);
  if (IC_type_space == point_loc) {
    fmt::print("X0 space = {}\n", X0_space);
  } else if (IC_type_space == hat) {
    fmt::print("hat_pct = {}\n", hat_pct);
  }
  fmt::print("X0 mass = {}\n", X0_mass);
  fmt::print("maxT = {}\n", maxT);
  fmt::print("dt = {}\n", dt);
  fmt::print("D = {}\n", D);
  fmt::print("pctRW = {}\n", pctRW);
  fmt::print("cdist_coeff = {}\n", cdist_coeff);
  fmt::print("cutdist = {}\n", cutdist);
  fmt::print("pFile = {}\n", pFile);
  fmt::print("nSteps = {}\n", nSteps);
  fmt::print("{}\n",
             "************************************************************");
}
}  // namespace particles
