#include "utilities.hpp"

namespace particles {

std::string toy_problem_intro() {
  std::ostringstream ss;
  ss << "1D Pure Diffusion Random Walks\n";
  return ss.str();
}

// anonymous namespace for utility fxns used below
namespace {

IC_enum enumerate_IC(std::string IC_str) {
  IC_enum IC_type;
  transform(IC_str.begin(), IC_str.end(), IC_str.begin(), ::tolower);
  if (IC_str.compare("point") == 0) {
    IC_type = point;
  } else if (IC_str.compare("uniform") == 0) {
    IC_type = uniform;
  } else if (IC_str.compare("equi") == 0) {
    IC_type = equi;
  } else {
    fprintf(stderr, "ERROR: Unsupported initial condition type: %s\n",
            &IC_str[0]);
    exit(1);
  }
  return IC_type;
}

} // end anonymous namespace

Params::Params(const std::string& yaml_name) {
  YAML::Node file_params = YAML::LoadFile(yaml_name);

  Np = file_params["Np"].as<int>();
  IC_str = file_params["IC_type"].as<std::string>();
  IC_type = enumerate_IC(IC_str);
  X0 = file_params["X0"].as<Real>();
  for (auto iter : file_params["omega"])
  {
    omega.push_back(iter.as<Real>());
  }
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
  std::cout << "IC type = " << IC_str << "\n";
  std::cout << "IC enum = " << IC_type << "\n";
  std::cout << "omega = [";
  for (auto i : omega) {std::cout << i << " ";}
  std::cout << "] \n";
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
    : params(install_prefix + _input_file),
      particleIO(install_prefix + std::string("/data/") +
                 std::string(params.pFile)),
      rand_pool(5374857) {
  params.print_summary();

  // create views of a few parameters on device and deep copy the corresponding
  // values from params
  D = ko::View<Real>("D");
  ko::deep_copy(D, params.D);
  dt = ko::View<Real>("dt");
  ko::deep_copy(dt, params.dt);
  Np = ko::View<Real>("Np");
  ko::deep_copy(Np, params.Np);

  // initialize the X view
  X = ko::View<Real*>("X", params.Np);
  initialize_positions(params);
}

void Particles::initialize_positions(Params params) {
  switch (params.IC_type) {
    case point: {
      std::cout << "params.IC_type (point) = " << params.IC_type << "\n";
      // deep copy the params X0 (host) to device
      auto X0 = ko::View<Real>("X0");
      ko::deep_copy(X0, params.X0);
      // fill the X view so particles are all located at X0
      ko::parallel_for(
          "InitX", params.Np, KOKKOS_LAMBDA(const int& i) { X(i) = X0(); });
      break;
    }
    case equi: {
      std::cout << "params.IC_type (equi) = " << params.IC_type << "\n";
      auto hX = ko::create_mirror_view(X);
      Real dx = (params.omega[1] - params.omega[0]) /
                static_cast<Real>(params.Np - 1);
      for (auto i = 0; i < params.Np - 1; ++i){
        hX(i) = static_cast<Real>(params.omega[0] + dx * i);
      }
      hX(params.Np - 1) = params.omega[1];
      ko::deep_copy(X, hX);
      break;
    }
    case uniform: {
      std::cout << "params.IC_type (uniform) = " << params.IC_type << "\n";
      ko::parallel_for(params.Np, RandomUniform<RandPoolType>(X, rand_pool,
                                                              params.omega[0],
                                                              params.omega[1]));
      break;
    }
  }
}

void Particles::random_walk(Real D, Real dt, RandyNorm& rn) {
  for (size_t i = 0; i < X.size(); ++i) {
    X[i] = X[i] + rn.get_num() * sqrt(2.0 * D * dt);
  }
}

// parallelized random walk function that calls the RandomWalk functor
void Particles::random_walk() {
  ko::parallel_for(params.Np, RandomWalk<RandPoolType>(X, rand_pool, 0.0,
                                                       2.0 * D() * dt()));
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
  if (tStep == 0) {
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
