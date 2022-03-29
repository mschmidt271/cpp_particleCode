#ifndef UTILITIES_HPP
#define UTILITIES_HPP

// ***TESTING***
// #include <chrono>
// #include <fstream>
// #include <random>
// #include <sstream>

#include <cmath>
// #include <iostream>
// #include <string>

// #include "ArborX.hpp"
#include "ArborX_LinearBVH.hpp"
#include "containers.hpp"
#include "Kokkos_Core.hpp"
#include "Kokkos_Random.hpp"
#include "mass_transfer.hpp"
// #include "KokkosSparse_spmv.hpp"
#include "type_defs.hpp"
#include "version_info.hpp"
// #include "yaml-cpp/yaml.h"

// using namespace utils;

namespace particles {

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
  RandomUniform(ko::View<Real*> vals_, const RandPool& rand_pool_, const Scalar& start_,
                const Scalar& end_)
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
  RandomWalk(ko::View<Real*> pvec_, const RandPool& rand_pool_, const Scalar& mean_,
             const Scalar& stddev_)
      : pvec(pvec_), rand_pool(rand_pool_), mean(mean_), stddev(stddev_) {}

};  // end RandomWalk functor

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
  // parameter views
  // ko::View<Real> D, pctRW, dt, cutdist;
  // ko::View<int> Np;
  // host version of params and the device view below
  Params params;
  MassTransfer mass_trans;
  // ko::View<Params> params;
  // typedef and variable for the random pool, used by the kokkos RNG
  // Note: there's also a 1024-bit generator, but that is probably overkill
  typedef typename ko::Random_XorShift64_Pool<> RandPoolType;
  ParticleIO particleIO;
  RandPoolType rand_pool;
  Particles(const std::string& input_file);
  // constructor that specifies the random number seed
  Particles(const std::string& input_file, const int& rand_seed);
  void initialize_positions();
  void initialize_masses();
  void random_walk();
  // void mass_transfer();
};

}  // namespace particles

#endif
