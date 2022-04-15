#include "parPT_io.hpp"

namespace particles {

// constructor for WriteParticles object that takes in the filename f and
// creates an outfile object
ParticleIO::ParticleIO(std::string f, const Params& _params) : params(_params) {
  if(params.write_plot) {
    outfile = fopen(f.c_str(), "w");
  }
}

void print_version_info() {
  // note: the format statement means the text will be centered in 60 total
  // spaces
  fmt::print("{:^60}\n",
             "1D Particle Tracking With Random Walks and Mass Transfer");
  fmt::print("{:^60}\n", "Version 0.0.1--NOT FOR SCIENCE");
}

void ParticleIO::print_params_summary() {
  fmt::print("{}\n",
             "************************************************************");
  print_version_info();
  fmt::print("{}\n",
             "************************************************************");
  fmt::print("Np = {}\n", params.Np);
  fmt::print("maxT = {}\n", params.maxT);
  fmt::print("dt = {}\n", params.dt);
  fmt::print("nSteps = {}\n", params.nSteps);
  fmt::print("dimension = {}\n", params.dim);
  fmt::print("omega = [{}, {}]\n", params.omega[0], params.omega[1]);
  fmt::print("IC type (space) = {}\n", params.IC_str_space);
  fmt::print("IC type (mass) = {}\n", params.IC_str_mass);
  if (params.IC_type_space == point_loc) {
    fmt::print("X0 space = {}\n", params.X0_space);
  } else if (params.IC_type_space == hat) {
    fmt::print("hat_pct = {}\n", params.hat_pct);
  }
  fmt::print("X0 mass = {}\n", params.X0_mass);
  fmt::print("D = {}\n", params.D);
  fmt::print("pctRW = {}\n", params.pctRW);
  fmt::print("cdist_coeff = {}\n", params.cdist_coeff);
  fmt::print("cutdist = {}\n", params.cutdist);
  fmt::print("pFile = {}\n", params.pFile);
  fmt::print("write plot info (position/mass) = {}\n", params.write_plot);
  fmt::print("{}\n\n",
             "************************************************************");
}

void ParticleIO::write(const ko::View<Real**>& X, const ko::View<Real*>& mass,
                       const int& tStep) {
  auto hX = ko::create_mirror_view(X);
  auto hmass = ko::create_mirror_view(mass);
  ko::deep_copy(hX, X);
  ko::deep_copy(hmass, mass);
  if (tStep == 0) {
    fmt::print(outfile, "{} {}\n", params.Np, params.nSteps);
    fmt::print(outfile, "{} ", params.IC_type_space);
    fmt::print(outfile, "{} ", params.IC_type_mass);
    for (auto i : params.omega) {
      fmt::print(outfile, "{} ", i);
    }
    if (params.IC_type_space == point_loc) {
      fmt::print(outfile, "{} -999 ", params.X0_space);
    } else if (params.IC_type_space == hat) {
      fmt::print(outfile, "{} -999 ", params.hat_pct);
    } else if (params.IC_type_space == equi) {
      fmt::print(outfile, "-999 -999 ");
    }
    fmt::print(outfile, "{} {} {} {} {} {} {}\n", params.X0_mass, params.maxT,
               params.dt, params.D, params.pctRW, params.cdist_coeff, params.cutdist);
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
