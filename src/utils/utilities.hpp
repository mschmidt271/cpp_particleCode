#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "yaml-cpp/yaml.h"

namespace particles {

typedef double Real;

std::string toy_problem_intro();

// class holding simulation parameters
class Params {
 public:
  int Np, nSteps;
  Real L, X0, maxT, dt, D;
  std::string pFile;
  void set_values(const std::string& yaml_name);
  void print_summary();
};

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

// class for writing particle output
class ParticleIO {
 private:
  std::string fName;
  std::ofstream outFile;

 public:
  ParticleIO(std::string f);
  void write(const std::vector<Real>& p, const Params& pars, int i);
};

void random_walk(std::vector<Real>& p, Real D, Real dt, RandyNorm& rn);

}  // namespace particles

#endif
