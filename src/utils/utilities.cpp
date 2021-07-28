#include "utilities.hpp"

namespace particles {

std::string toy_problem_intro() {
  std::ostringstream ss;
  ss << "1D Pure Diffusion Random Walks\n";
  return ss.str();
}

void Params::set_values(const std::string& yaml_name) {
  YAML::Node file_params = YAML::LoadFile(yaml_name);

  Np = file_params["Np"].as<int>();
  L = file_params["L"].as<Real>();
  X0 = file_params["X0"].as<Real>();
  maxT = file_params["maxT"].as<Real>();
  dt = file_params["dt"].as<Real>();
  D = file_params["D"].as<Real>();
  pFile = file_params["pFile"].as<std::string>();
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

void random_walk(std::vector<Real>& p, Real D, Real dt, RandyNorm& rn) {
  for (int i = 0; i < p.size(); ++i) {
    p[i] = p[i] + rn.get_num() * sqrt(2.0 * D * dt);
  }
}

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

// constructor for WriteParticles object that takes in the filename f and
// creates an outFile object
ParticleIO::ParticleIO(std::string f) : outFile(f) {}

}  // namespace particles
