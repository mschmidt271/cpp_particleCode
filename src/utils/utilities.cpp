#include "utilities.hpp"

namespace particles {

// NOTE: uses pre-defined random seed
// FIXME: make a new constructor that seeds from clock-time or user-provided
// seed
Particles::Particles(const std::string& _input_file)
    : params(install_prefix + _input_file),
      particleIO(install_prefix + params.pFile, params) {
  ko::Profiling::pushRegion("constructor print");
  particleIO.print_params_summary();
  ko::Profiling::popRegion();
  rand_pool = init_random_seed();
  ko::Profiling::pushRegion("ctor initialize position");
  // initialize the X view
  X = ko::View<Real*>("X", params.Np);
  initialize_positions();
  // initialize the mass view
  ko::Profiling::popRegion();
  ko::Profiling::pushRegion("ctor initialize mass");
  mass = ko::View<Real*>("mass", params.Np);
  initialize_masses();
  ko::Profiling::popRegion();
  mass_trans = MassTransfer<CRSPolicy>(params, X, mass);
}

void Particles::initialize_positions() {
  switch (params.IC_type_space) {
    case point_loc: {
      // deep copy the params X0 (host) to device
      auto X0 = ko::View<Real>("X0");
      ko::deep_copy(X0, params.X0_space);
      auto lX = X;
      // fill the X view so particles are all located at X0
      ko::parallel_for(
          "InitX", params.Np, KOKKOS_LAMBDA(const int& i) { lX(i) = X0(); });
      break;
    }
    case equi: {
      auto hX = ko::create_mirror_view(X);
      Real dx = (params.omega[1] - params.omega[0]) /
                static_cast<Real>(params.Np - 1);
      for (auto i = 0; i < params.Np - 1; ++i) {
        hX(i) = static_cast<Real>(params.omega[0] + dx * i);
      }
      hX(params.Np - 1) = params.omega[1];
      ko::deep_copy(X, hX);
      break;
    }
    case uniform: {
      ko::parallel_for(
          params.Np, RandomUniform<RandPoolType>(X, rand_pool, params.omega[0],
                                                 params.omega[1]));
      break;
    }
    case hat: {
      ko::Profiling::pushRegion("space hat");
      auto hX = ko::create_mirror_view(X);
      Real L = params.omega[1] - params.omega[0];
      Real start = 0.4 * L;
      Real end = 0.6 * L;
      Real dx = (end - start) / static_cast<Real>(params.Np - 1);
      for (auto i = 0; i < params.Np - 1; ++i) {
        hX(i) = static_cast<Real>(start + dx * i);
      }
      hX(params.Np - 1) = end;
      ko::deep_copy(X, hX);
      break;
      ko::Profiling::popRegion();
    }
  }
}

void Particles::initialize_masses() {
  switch (params.IC_type_mass) {
    case point_mass: {
      auto hmass = ko::create_mirror_view(mass);
      int mid = (params.Np + 2 - 1) / 2 - 1;
      for (int i = 0; i < params.Np; ++i) {
        hmass(i) = 0.0;
      }
      hmass(mid) = 1.0;
      ko::deep_copy(mass, hmass);
    }
    case heaviside: {
      break;
    }
    case gaussian: {
      break;
    }
  }
}

}  // namespace particles
