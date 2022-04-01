#include "parPT_io.hpp"

namespace particles {

// constructor for WriteParticles object that takes in the filename f and
// creates an outfile object
ParticleIO::ParticleIO(std::string f) { outfile = fopen(f.c_str(), "w"); }

void ParticleIO::write(const ko::View<Real*>& X, const ko::View<Real*>& mass,
                       const Params& pars, int tStep) {
  auto hX = ko::create_mirror_view(X);
  auto hmass = ko::create_mirror_view(mass);
  ko::deep_copy(hX, X);
  ko::deep_copy(hmass, mass);
  if (tStep == 0) {
    fmt::print(outfile, "{} {}\n", pars.Np, pars.nSteps);
    fmt::print(outfile, "{} ", pars.IC_type_space);
    fmt::print(outfile, "{} ", pars.IC_type_mass);
    for (auto i : pars.omega) {
      fmt::print(outfile, "{} ", i);
    }
    if (pars.IC_type_space == point_loc) {
      fmt::print(outfile, "{} -999 ", pars.X0_space);
    } else if (pars.IC_type_space == hat) {
      fmt::print(outfile, "{} -999 ", pars.hat_pct);
    } else if (pars.IC_type_space == equi) {
      fmt::print(outfile, "-999 -999 ");
    }
    fmt::print(outfile, "{} {} {} {} {} {} {}\n", pars.X0_mass, pars.maxT,
               pars.dt, pars.D, pars.pctRW, pars.cdist_coeff, pars.cutdist);
    for (size_t i = 0; i < hX.extent(0); ++i) {
      fmt::print(outfile, "{} {}\n", hX(i), hmass(i));
    }
  } else {
    for (size_t i = 0; i < hX.extent(0); ++i) {
      fmt::print(outfile, "{} {}\n", hX(i), hmass(i));
    }
  }
}

}  // namespace particles
