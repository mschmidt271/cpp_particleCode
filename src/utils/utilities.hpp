#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <cmath>

#include "ArborX_LinearBVH.hpp"
#include "Kokkos_Core.hpp"
#include "Kokkos_Random.hpp"
#include "containers.hpp"
#include "mass_transfer.hpp"
// #include "random.hpp"
#include "random_walk.hpp"
#include "type_defs.hpp"
#include "version_info.hpp"

#include "brute_force_crs_policy.hpp"
#include "tree_crs_policy.hpp"

namespace particles {

// this comes from the parPT_config.hpp via type_defs.hpp
using CRSPolicy = crs_policy_name;

// class for writing particle output
class ParticleIO {
 private:
  FILE* outfile;

 public:
  ParticleIO(std::string f);
  void write(const ko::View<Real*>& X, const ko::View<Real*>& mass,
             const Params& pars, int i);
};

// class for particles and associated methods
class Particles {
 public:
  // real-valued position
  ko::View<Real*> X;
  // FIXME: keep a mirror (private?) of these for writing out every xx time
  // steps mass carried by particles (FIXME: units, )
  ko::View<Real*> mass;
  // host version of params
  // FIXME(?): create a device version?
  Params params;
  MassTransfer<CRSPolicy> mass_trans;
  RandPoolType rand_pool;
  ParticleIO particleIO;
  Particles(const std::string& input_file);
  // constructor that specifies the random number seed
  Particles(const std::string& input_file, const int& rand_seed);
  void initialize_positions();
  void initialize_masses();
};

}  // namespace particles

#endif
