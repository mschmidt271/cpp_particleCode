#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "ArborX.hpp"
#include "KokkosKernels_default_types.hpp"
#include "KokkosSparse_CrsMatrix.hpp"
#include "KokkosSparse_spgemm.hpp"
#include "KokkosSparse_spmv.hpp"
#include "type_defs.hpp"
#include "yaml-cpp/yaml.h"

namespace particles {

std::string toy_problem_intro();

// functor for generating uniformly-distributed random doubles
// in the range [start, end]
// Note: RandPoolType is currently hard-coded in the particle class
// GeneratorPool type
template <class RandPool>
struct RandomUniform {
  // Output View for the random numbers
  ko::View<Real*> vals;

  // The GeneratorPool
  RandPool rand_pool;

  typedef Real Scalar;
  typedef typename RandPool::generator_type gen_type;

  // mean and variance of the normal random variable
  Scalar start, end;

  KOKKOS_INLINE_FUNCTION
  void operator()(int i) const {
    // Get a random number state from the pool for the active thread
    gen_type rgen = rand_pool.get_state();

    // draw random normal numbers, with mean and variance provided
    vals(i) = rgen.drand(start, end);

    // Give the state back, which will allow another thread to acquire it
    rand_pool.free_state(rgen);
  }

  // Constructor, Initialize all members
  RandomUniform(ko::View<Real*> vals_, RandPool rand_pool_, Scalar start_,
                Scalar end_)
      : vals(vals_), rand_pool(rand_pool_), start(start_), end(end_) {}

};  // end RandomUniform functor

// functor for random-walking particles, templated on the GeneratorPool type
// Note: RandPoolType is currently hard-coded in the particle class
// GeneratorPool type
template <class RandPool>
struct RandomWalk {
  // Output View for the random numbers
  ko::View<Real*> pvec;

  // The GeneratorPool
  RandPool rand_pool;

  typedef Real Scalar;
  typedef typename RandPool::generator_type gen_type;

  // mean and std deviation of the normal random variable
  Scalar mean, stddev;

  KOKKOS_INLINE_FUNCTION
  void operator()(int i) const {
    // Get a random number state from the pool for the active thread
    gen_type rgen = rand_pool.get_state();

    // draw random normal numbers, with mean and std deviation provided
    pvec(i) = pvec(i) + rgen.normal(mean, stddev);

    // Give the state back, which will allow another thread to acquire it
    rand_pool.free_state(rgen);
  }

  // Constructor, Initialize all members
  RandomWalk(ko::View<Real*> pvec_, RandPool rand_pool_, Scalar mean_,
             Scalar stddev_)
      : pvec(pvec_), rand_pool(rand_pool_), mean(mean_), stddev(stddev_) {}

};  // end RandomWalk functor

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
  void enumerate_IC(std::string IC_str, YAML::Node yml, bool space);
};

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
  // mask for distmat reduction
  // FIXME: make this int/boolean
  ko::View<Real*> mask;
  // parameter views
  ko::View<Real> D, pctRW, dt, cutdist;
  ko::View<int> Np;
  Params params;
  // typedef and variable for the random pool, used by the kokkos RNG
  // Note: there's also a 1024-bit generator, but that is probably overkill
  typedef typename ko::Random_XorShift64_Pool<> RandPoolType;
  ParticleIO particleIO;
  RandPoolType rand_pool;
  Particles(std::string input_file);
  // constructor that specifies the random number seed
  Particles(std::string input_file, int rand_seed);
  void initialize_positions(Params params);
  void initialize_masses(Params params);
  void random_walk();
  void mass_transfer();
  SpmatType get_transfer_mat();
  SpmatType sparse_kernel_mat(const int& nnz, const ko::View<Real*>& mask);
};

}  // namespace particles

#endif
