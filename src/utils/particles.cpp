#include "particles.hpp"

// TODO: make this particles.xpp

namespace particles {

Particles::Particles(const std::string& _input_file)
{
  ko::Profiling::pushRegion("construct particleIO");
  const std::string yaml_name = install_prefix + _input_file;
  particleIO = ParticleIO(params, yaml_name);
  ko::Profiling::popRegion();
  // ko::Profiling::pushRegion("read params from file");
  // std::string yaml_name = install_prefix + _input_file;
  // particleIO.read_params_input(yaml_name);
  // // ko::Profiling::popRegion();
  ko::Profiling::pushRegion("constructor print");
  particleIO.print_params_summary(params);
  ko::Profiling::popRegion();


  // // uint64_t seed;
  // // switch (params.seed_type) {
  // //   case clock_rand: {
  // //     seed =
  // //         std::chrono::high_resolution_clock::now().time_since_epoch().count();
  // //     break;
  // //   }
  // //   case specified_rand: {
  // //     seed = params.seed_val;
  // //     break;
  // //   }
  // //   case default_rand: {
  // //     seed = 5374857;
  // //     break;
  // //   } case missing: {
  // //     fmt::print("Seed type or value not provided--using default seed 5374857.\n");
  // //     seed = 5374857;
  // //     break;
  // //   }
  // //   default: {
  // //     // this should not be possible
  // //     fmt::print("Seed type or value is invalid--using default seed 5374857.\n");
  // //     seed = 5374857;
  // //     break;
  // //   }
  // // }
  // // // rand_pool = init_random_seed(seed);


  rand_pool = RandPoolType(params.seed_val);
  ko::Profiling::pushRegion("ctor initialize position");
  // initialize the X view
  X = ko::View<Real**>("X", params.dim, params.Np);
  particleIO.initialize_positions(params, yaml_name, X);
  auto hX = ko::create_mirror_view(X);
  ko::deep_copy(hX, X);
  for (int i = 0; i < params.Np; ++i)
  {
    std::cout << "hX(0, i) = " << hX(0, i) << "\n";
  }
  // initialize the mass view
  ko::Profiling::popRegion();
  ko::Profiling::pushRegion("ctor initialize mass");
  mass = ko::View<Real*>("mass", params.Np);
  std::cout << "mass.extent(0) = " << mass.extent(0) << "\n";
  initialize_masses();
  ko::Profiling::popRegion();
  mass_trans = MassTransfer<CRSPolicy>(params, X, mass);
}

// void Particles::initialize_positions() {
//   switch (params.IC_type_space) {
//     case point_loc: {
//       // deep copy the params X0 (host) to device
//       auto X0 = ko::View<Real>("X0");
//       ko::deep_copy(X0, params.X0_space);
//       auto lX = X;
//       // fill the X view so particles are all located at X0
//       // FIXME: profile this to figure out which way the loops should go
//       ko::parallel_for(
//           "InitX", ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {params.dim, params.Np}),
//           KOKKOS_LAMBDA(const int& j, const int& i) { lX(i, j) = X0(); });
//       break;
//     }
//     case equi: {
//       // Note: this may be a good spot for an OffsetView
//       // FIXME: I may just read in a file containing points for anything other
//       // than point and uniform auto hX = ko::create_mirror_view(X); Real dx =
//       // (params.omega[1] - params.omega[0]) /
//       //           static_cast<Real>(params.Np - 1);
//       // for (int i = 0; i < params.dim; ++i)
//       // {
//       //   for (int j = 0; j < params.Np - 1; ++j)
//       //   {
//       //     hX(i, j) = static_cast<Real>(params.omega[0] + dx * i);
//       //   }
//       // }
//       //   for (auto i = 0; i < params.Np - 1; ++i) {

//       //   }
//       // hX(params.Np - 1) = params.omega[1];
//       // ko::deep_copy(X, hX);
//       break;
//     }
//     case uniform: {
//       ko::parallel_for("space uniform",
//                        ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {params.dim, params.Np}),
//                        RandomUniform<RandPoolType>(
//                            X, rand_pool, params.omega[0], params.omega[1]));
//       break;
//     }
//     case hat: {
//       //   ko::Profiling::pushRegion("space hat");
//       //   auto hX = ko::create_mirror_view(X);
//       //   Real L = params.omega[1] - params.omega[0];
//       //   Real start = 0.4 * L;
//       //   Real end = 0.6 * L;
//       //   Real dx = (end - start) / static_cast<Real>(params.Np - 1);
//       //   for (auto i = 0; i < params.Np - 1; ++i) {
//       //     hX(i) = static_cast<Real>(start + dx * i);
//       //   }
//       //   hX(params.Np - 1) = end;
//       //   ko::deep_copy(X, hX);
//       //   break;
//       //   ko::Profiling::popRegion();
//       }
//     }
//   }

void Particles::initialize_masses() {
  // switch (params.IC_type_mass) {
  //   case point_mass: {
  //     auto hmass = ko::create_mirror_view(mass);
  //     int mid = (params.Np + 2 - 1) / 2 - 1;
  //     std::cout << "mid = " << mid << "\n";
  //     for (int i = 0; i < params.Np; ++i) {
  //       hmass(i) = 0.0;
  //     }
  //     hmass(mid) = 1.0;
  //     std::cout << "hmass(mid) = " << hmass(mid) << "\n";
  //     ko::deep_copy(mass, hmass);
  //   }
  //   case heaviside: {
  //     break;
  //   }
  //   case gaussian: {
  //     break;
  //   }
  // }
}

}  // namespace particles
