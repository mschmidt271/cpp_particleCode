#include "containers.hpp"

namespace particles {

// TODO: add error-checking/default values here
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
  std::string rand_seed_str;
  // the parser will crash is they type string is empty, so check first
  // Note: for whatever reason, IsScalar() seems to do the best job of
  // determining whether or not the value is empty when the key is in the yaml
  if (file_params["rand_seed_type"] and
      file_params["rand_seed_type"].IsScalar()) {
    rand_seed_str = file_params["rand_seed_type"].as<std::string>();
  } else {
    rand_seed_str = "missing";
  }
  enumerate_seed_type(rand_seed_str, file_params);
  maxT = file_params["maxT"].as<Real>();
  dt = file_params["dt"].as<Real>();
  D = file_params["D"].as<Real>();
  pctRW = file_params["pctRW"].as<Real>();
  denom = 4 * D * (1.0 - pctRW) * dt;
  cdist_coeff = file_params["cdist_coeff"].as<Real>();
  cutdist = cdist_coeff * sqrt(denom);
  pFile = file_params["pFile"].as<std::string>();
  if (file_params["write_plot"])
  {
    write_plot = file_params["write_plot"].as<bool>();
  } else {
    write_plot = false;
  }
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

void Params::enumerate_seed_type(std::string& seed_str, const YAML::Node& yml) {
  if (!seed_str.empty()) {
    transform(seed_str.begin(), seed_str.end(), seed_str.begin(), ::tolower);
  }
  if (seed_str.compare("default") == 0) {
    seed_type = default_rand;
  } else if (seed_str.compare("clock") == 0) {
    seed_type = clock_rand;
  } else if (seed_str.compare("specified") == 0) {
    seed_type = specified_rand;
    if (yml["rand_seed_value"] and yml["rand_seed_value"].IsScalar()) {
      seed_val = yml["rand_seed_value"].as<uint64_t>();
    } else {
      seed_type = missing;
      seed_val = 5374857;
    }
  } else if (seed_str.compare("missing") == 0) {
    seed_type = missing;
    seed_val = 5374857;
  } else {
    seed_type = default_rand;
  }
}

}  // namespace particles
