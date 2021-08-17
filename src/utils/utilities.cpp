#include "utilities.hpp"

namespace particles {

std::string toy_problem_intro() {
  std::ostringstream ss;
  ss << "1D Pure Diffusion Random Walks\n";
  return ss.str();
}

Params::Params(const std::string& yaml_name) {
  YAML::Node file_params = YAML::LoadFile(yaml_name);

  Np = file_params["Np"].as<int>();
  L = file_params["L"].as<Real>();
  X0 = file_params["X0"].as<Real>();
  maxT = file_params["maxT"].as<Real>();
  dt = file_params["dt"].as<Real>();
  D = file_params["D"].as<Real>();
  pFile = file_params["pFile"].as<std::string>();
  nSteps = ceil(maxT / dt);
}

void Params::print_summary() {
  std::cout << "**********************************\n";
  std::cout << "  " << particles::toy_problem_intro();
  std::cout << "**********************************\n";
  std::cout << "Np = " << Np << "\n";
  std::cout << "L = " << L << "\n";
  std::cout << "X0 = " << X0 << "\n";
  std::cout << "maxT = " << maxT << "\n";
  std::cout << "dt = " << dt << "\n";
  std::cout << "D = " << D << "\n";
  std::cout << "pFile = " << pFile << "\n";
  std::cout << "nSteps = " << nSteps << "\n";
  std::cout << "***********************************\n";
}

// constructor for random N(mean, var) generator that seeds with clock time
// SOURCE:
// http://www.cplusplus.com/reference/random/normal_distribution/normal_distirbution/
// AND:  http://www.cpp.re/forum/general/223250/
RandyNorm::RandyNorm(Real mean, Real var) : gen(seed), dist(mean, var) {}

// method for getting a random normal number from RandyNorm
Real RandyNorm::get_num() {
  Real num = dist(gen);
  return num;
}

// constructor that places all particles in the same place (X0)
// NOTE: uses pre-defined random seed
// FIXME: make a new constructor that seeds from clock-time or user-provided seed
Particles::Particles(std::string _input_file)
    : input_file(_input_file),
      params(install_prefix + _input_file),
      particleIO(install_prefix + std::string("/data/") +
                 std::string(params.pFile)),
      rand_pool(5374857) {
  params.print_summary();

  D = ko::View<Real>("D");
  ko::deep_copy(D, params.D);
  dt = ko::View<Real>("dt");
  ko::deep_copy(dt, params.dt);
  Np = ko::View<Real>("Np");
  ko::deep_copy(Np, params.Np);

  X = ko::View<Real*>("X", params.Np);
  auto X0 = ko::View<Real>("X0");
  // deep copy the params X0 (host) to device
  ko::deep_copy(X0, params.X0);
  // fill the position view so they are all located at X0
  ko::parallel_for(
      "InitX", params.Np, KOKKOS_LAMBDA(const int& i) { X(i) = X0(); });
}

void Particles::random_walk(Real D, Real dt, RandyNorm& rn) {
  for (size_t i = 0; i < X.size(); ++i) {
    X[i] = X[i] + rn.get_num() * sqrt(2.0 * D * dt);
  }
}

void Particles::random_walk() {
  auto rand_vals = ko::View<Real*>("rand_vals", params.Np);
  ko::parallel_for(params.Np, GenRandom<RandPoolType>(rand_vals, rand_pool, 0.0,
                                                      2.0 * D() * dt()));
  ko::parallel_for(
      params.Np, KOKKOS_LAMBDA(const int& i) { X(i) = X(i) + rand_vals(i); });
}

// constructor for WriteParticles object that takes in the filename f and
// creates an outFile object
ParticleIO::ParticleIO(std::string f) : outFile(f) {}

void ParticleIO::write(const std::vector<Real>& p, const Params& pars,
                       int tStep) {
  if (tStep == 1) {
    outFile << pars.Np << " " << pars.nSteps << "\n";
    for (std::vector<Real>::const_iterator i = p.begin(); i != p.end(); ++i) {
      outFile << *i << "\n";
    }
  } else {
    for (std::vector<Real>::const_iterator i = p.begin(); i != p.end(); ++i) {
      outFile << *i << "\n";
    }
  }
}

void ParticleIO::write(const ko::View<Real*>& X, const Params& pars,
                       int tStep) {
  auto hX = ko::create_mirror_view(X);
  ko::deep_copy(hX, X);
  if (tStep == 1) {
    outFile << pars.Np << " " << pars.nSteps << "\n";
    for (size_t i = 0; i < hX.extent(0); ++i) {
      outFile << hX(i) << "\n";
    }
  } else {
    for (size_t i = 0; i < hX.extent(0); ++i) {
      outFile << hX(i) << "\n";
    }
  }
}

}  // namespace particles
