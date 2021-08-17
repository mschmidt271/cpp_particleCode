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

// this gives the install directory
#include "parPT_config.h"
#include "yaml-cpp/yaml.h"

namespace particles {

// alias the kokkos namespace to make life easier
namespace ko = Kokkos;

typedef double Real;

std::string toy_problem_intro();

// class for generating random normal variables
class RandyNorm {
 private:
  Real mean, var;
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine gen;
  std::normal_distribution<Real> dist;

 public:
  RandyNorm(Real m, Real v);
  Real get_num();
};

// A Functor for generating uint64_t random numbers templated on the
// GeneratorPool type
template <class RandPool>
struct GenRandom {
  // Output View for the random numbers
  ko::View<Real*> vals;

  // The GeneratorPool
  RandPool rand_pool;

  typedef Real Scalar;
  typedef typename RandPool::generator_type gen_type;

  // mean and variance of the normal random variable
  Scalar mean, var;

  KOKKOS_INLINE_FUNCTION
  void operator()(int i) const {
    // Get a random number state from the pool for the active thread
    gen_type rgen = rand_pool.get_state();

    // draw random normal numbers, with mean and variance provided
    vals(i) = rgen.normal(mean, var);

    // Give the state back, which will allow another thread to acquire it
    rand_pool.free_state(rgen);
  }

  // Constructor, Initialize all members
  GenRandom(ko::View<Real*> vals_, RandPool rand_pool_, Scalar mean_,
            Scalar var_)
      : vals(vals_), rand_pool(rand_pool_), mean(mean_), var(var_) {}

};  // end GenRandom struct

// class holding simulation parameters
class Params {
 public:
  int Np, nSteps;
  Real L, X0, maxT, dt, D;
  std::string pFile;
  void set_values(const std::string& yaml_name);
  void print_summary();
  Params() = default;
  Params(const std::string& yaml_name);
};

// class for writing particle output
class ParticleIO {
 private:
  std::ofstream outFile;
 public:
  ParticleIO(std::string f);
  void write(const std::vector<Real>& p, const Params& pars, int i);
  void write(const ko::View<Real*>& p, const Params& pars, int i);
};

// class for particles and associated methods
class Particles {
 public:
  std::string input_file;
  // real-valued position
  ko::View<Real*> X;
  // parameter views
  ko::View<Real> D, dt, Np;
  Params params;
  // typedef and variable for the random pool, used by the kokkos RNG
  typedef typename ko::Random_XorShift64_Pool<> RandPoolType;
  ParticleIO particleIO;
  RandPoolType rand_pool;
  Particles(std::string input_file);
  // constructor that specifies the random number seed
  Particles(std::string input_file, int rand_seed);
  void random_walk(Real D, Real dt, RandyNorm& rn);
  void random_walk();
};

}  // namespace particles

#endif
