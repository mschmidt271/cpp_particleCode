#include "utilities.hpp"

namespace particles {

std::string toy_problem_intro() {
  std::ostringstream ss;
  ss << "1D Particle Tracking With Random Walks and Mass Transfer\n";
  return ss.str();
}

// anonymous namespace for utility fxns used below
namespace {

static constexpr double pi = 3.14159265358979323846264;

}  // end anonymous namespace

Params::Params(const std::string& yaml_name) {
  YAML::Node file_params = YAML::LoadFile(yaml_name);

  Np = file_params["Np"].as<int>();
  IC_str_space = file_params["initial_condition"]["space"]["type"].as<std::string>();
  // true if enumerating spatial IC
  enumerate_IC(IC_str_space, file_params, true);
  IC_str_mass = file_params["initial_condition"]["mass"]["type"].as<std::string>();
  enumerate_IC(IC_str_mass, file_params, false);
  for (auto iter : file_params["omega"]) {
    omega.push_back(iter.as<Real>());
  }
  maxT = file_params["maxT"].as<Real>();
  dt = file_params["dt"].as<Real>();
  D = file_params["D"].as<Real>();
  pctRW = file_params["pctRW"].as<Real>();
  denom = 4 * D * (1.0 - pctRW) * dt;
  cdist_coeff = file_params["cdist_coeff"].as<Real>();
  cutdist = cdist_coeff * sqrt(denom);
  pFile = file_params["pFile"].as<std::string>();
  nSteps = ceil(maxT / dt);
}

void Params::enumerate_IC(std::string IC_str, YAML::Node yml, bool space) {
  transform(IC_str.begin(), IC_str.end(), IC_str.begin(), ::tolower);
  if (space) {
    if (IC_str.compare("point") == 0) {
      IC_type_space = point_loc;
      X0_space = yml["initial_condition"]["space"]["X0"].as<Real>();
    } else if (IC_str.compare("uniform") == 0) {
      IC_type_space = uniform;
    } else if (IC_str.compare("equi") == 0) {
      IC_type_space = equi;
    } else if (IC_str.compare("hat") == 0) {
      IC_type_space = hat;
      hat_pct = yml["initial_condition"]["space"]["hat_pct"].as<Real>();
    } else {
      fprintf(stderr, "ERROR: Unsupported initial condition type: %s\n",
              &IC_str[0]);
      exit(1);
    }
  } else {
    if (IC_str.compare("point") == 0) {
      IC_type_mass = point_mass;
      X0_mass = yml["initial_condition"]["mass"]["X0"].as<Real>();
    } else if (IC_str.compare("heaviside") == 0) {
      IC_type_mass = heaviside;
      X0_mass = yml["initial_condition"]["mass"]["X0"].as<Real>();
    } else if (IC_str.compare("gaussian") == 0) {
      IC_type_mass = gaussian;
      X0_mass = yml["initial_condition"]["mass"]["X0"].as<Real>();
    } else {
      fprintf(stderr, "ERROR: Unsupported initial condition type: %s\n",
              &IC_str[0]);
      exit(1);
    }
  }

}

void Params::print_summary() {
  std::cout << "************************************************************\n";
  std::cout << "  " << particles::toy_problem_intro();
  std::cout << "************************************************************\n";
  std::cout << "Np = " << Np << "\n";
  std::cout << "omega = [";
  for (auto i : omega) {
    std::cout << i << " ";
  }
  std::cout << "\b \b] \n";
  std::cout << "IC type (space) = " << IC_str_space << "\n";
  std::cout << "IC type (mass) = " << IC_str_mass << "\n";
  if (IC_type_space == point_loc) {
    std::cout << "X0 space = " << X0_space << "\n";
  } else if (IC_type_space == hat) {
    std::cout << "hat_pct = " << hat_pct << "\n";
  }
  std::cout << "X0 mass = " << X0_mass << "\n";
  std::cout << "maxT = " << maxT << "\n";
  std::cout << "dt = " << dt << "\n";
  std::cout << "D = " << D << "\n";
  std::cout << "pctRW = " << pctRW << "\n";
  std::cout << "cdist_coeff = " << cdist_coeff << "\n";
  std::cout << "cutdist = " << cutdist << "\n";
  std::cout << "pFile = " << pFile << "\n";
  std::cout << "nSteps = " << nSteps << "\n";
  std::cout
      << "*************************************************************\n";
}

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
    fprintf(outfile, "%i %i\n", pars.Np, pars.nSteps);
    fprintf(outfile, "%i ", pars.IC_type_space);
    fprintf(outfile, "%i ", pars.IC_type_mass);
    for (auto i : pars.omega) {
      fprintf(outfile, "%g ", i);
    }
    if (pars.IC_type_space == point_loc) {
      fprintf(outfile, "%g -999 ", pars.X0_space);
    } else if (pars.IC_type_space == hat) {
      fprintf(outfile, "%g -999 ", pars.hat_pct);
    }
    else if (pars.IC_type_space == equi) {
      fprintf(outfile, "-999 -999 ");
    }
    fprintf(outfile, "%g %g %g %g %g %g %g\n", pars.X0_mass, pars.maxT, pars.dt,
            pars.D, pars.pctRW, pars.cdist_coeff, pars.cutdist);
      for (size_t i = 0; i < hX.extent(0); ++i) {
        fprintf(outfile, "%g %g\n", hX(i), hmass(i));
      }
    } else {
      for (size_t i = 0; i < hX.extent(0); ++i) {
        fprintf(outfile, "%g %g\n", hX(i), hmass(i));
      }
  }
}

// constructor that places all particles in the same place (X0)
// NOTE: uses pre-defined random seed
// FIXME: make a new constructor that seeds from clock-time or user-provided
// seed
Particles::Particles(std::string _input_file)
    : params(install_prefix + _input_file),
      particleIO(install_prefix + std::string(params.pFile)),
      rand_pool(5374857) {
  params.print_summary();

  // create views of a few parameters on device and deep copy the corresponding
  // values from params
  D = ko::View<Real>("D");
  ko::deep_copy(D, params.D);
  pctRW = ko::View<Real>("pctRW");
  ko::deep_copy(pctRW, params.pctRW);
  dt = ko::View<Real>("dt");
  ko::deep_copy(dt, params.dt);
  Np = ko::View<Real>("Np");
  ko::deep_copy(Np, params.Np);

  // initialize the X view
  X = ko::View<Real*>("X", params.Np);
  initialize_positions(params);
  // initialize the mass view
  mass = ko::View<Real*>("mass", params.Np);
  initialize_masses(params);
  mask = ko::View<Real*>("idx_mask", pow(params.Np, 2));
}

void Particles::initialize_positions(Params params) {
  switch (params.IC_type_space) {
    case point_loc: {
      // deep copy the params X0 (host) to device
      auto X0 = ko::View<Real>("X0");
      ko::deep_copy(X0, params.X0_space);
      // fill the X view so particles are all located at X0
      ko::parallel_for(
          "InitX", params.Np, KOKKOS_LAMBDA(const int& i) { X(i) = X0(); });
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
    }
  }
}

void Particles::initialize_masses(Params params) {
  switch (params.IC_type_mass) {
    case point_mass: {
      auto hmass = ko::create_mirror_view(mass);
      int mid = (params.Np + 2 - 1) / 2 - 1;
      for (int i = 0; i < params.Np; ++i)
      {
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

// parallelized random walk function that calls the RandomWalk functor
void Particles::random_walk() {
  if (pctRW() > 0.0) {
    ko::parallel_for(params.Np,
                     RandomWalk<RandPoolType>(X, rand_pool, 0.0,
                                              sqrt(2.0 * pctRW() * D() * dt())));
  }
}

void Particles::mass_transfer() {
  if (pctRW() < 1.0)
  {
    ko::Profiling::pushRegion("build_Tmat");
    spmat_type mat = get_transfer_mat();
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("matvec");
    auto tmpmass = ko::View<Real*>("tmpmass", params.Np);
    ko::deep_copy(tmpmass, mass);
    KokkosSparse::spmv("N", 1.0, mat, tmpmass, 0.0, mass);
    ko::Profiling::popRegion();
  }
}

spmat_type Particles::get_transfer_mat() {
  Real Np2 = pow(params.Np, 2);
  // auto dmat = ko::View<Real**>("dist_mat", params.Np, params.Np);
  Real cutdist = params.cutdist;
  int nnz = 0;
  // auto mask = ko::View<Real*>("idx_mask", Np2);
  ko::parallel_reduce(
      "sum nnz", ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {params.Np, params.Np}),
      KOKKOS_LAMBDA(const int& i, const int& j, int& val) {
        Real dist = fabs(X(i) - X(j));
        int idx = j + (i * params.Np);
        mask(idx)= 0;
        if (dist <= cutdist) {
          val += 1;
          mask(idx) = 1;
        }
      },
      nnz);
  ko::parallel_scan(
      "create_reduction_maskmat", Np2,
      KOKKOS_LAMBDA(const int& i, Real& update, const bool final) {
        const Real val_i = mask(i);
        if (final) {
          mask(i) = update;
        }
        update += val_i;
      });
  auto kmat = sparse_kernel_mat(nnz, mask);
  return kmat;
}

spmat_type Particles::sparse_kernel_mat(const int& nnz,
                                        const ko::View<Real*>& mask) {
  auto row = ko::View<int*>("row", nnz);
  auto col = ko::View<int*>("col", nnz);
  auto val = ko::View<Real*>("val", nnz);
  auto rowmap = ko::View<int*>("rowmap", params.Np + 1);
  ko::deep_copy(rowmap, 0.0);
  ko::deep_copy(col, 0.0);
  ko::deep_copy(val, 0.0);
  Real cutdist = params.cutdist;
  int Np = params.Np;
  Real denom = params.denom;
  Real c = sqrt(denom * pi);
  ko::parallel_for(
      "form_sparse_kmat",
      ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {Np, Np}),
      KOKKOS_LAMBDA(const int& i, const int& j) {
        int idx = mask(j + (i * Np));
        Real d = fabs(X(i) - X(j));
        if (d <= cutdist) {
          row(idx) = i;
          col(idx) = j;
          val(idx) =
              exp(pow(d, 2) / -denom) / c;
        }
      });
  // ***NOTE: possibly (probably) sort by row here?***
  ko::parallel_for(
      "compute_rowmap", nnz,
      KOKKOS_LAMBDA(const int& i) { rowmap(row(i) + 1)++; });
  ko::parallel_scan(
      "finalize_rowmap", params.Np + 1,
      KOKKOS_LAMBDA(const int& i, int& update, const bool final) {
        const int val_i = rowmap(i);
        update += val_i;
        if (final) {
          rowmap(i) = update;
        }
      });
  auto rowcolsum = ko::View<Real*>("rowcolsum", params.Np);
  // NOTE: possibly try a team policy here, a la sec. 8.1 in the Kokkos v3 paper
  ko::parallel_for(
      "compute_rowcolsum", params.Np, KOKKOS_LAMBDA(const int& i) {
        for (int j = rowmap(i); j < rowmap(i + 1); ++j) {
          rowcolsum(i) += val(j);
        }
      });
  // NOTE: we may have to do this more than once if we run into issues.
  // I suspect we should be ok, though, since we still guarantee symmetry
  ko::parallel_for(
      "normalize_mat", nnz, KOKKOS_LAMBDA(const int& i) {
        val(i) = 2.0 * val(i) / (rowcolsum(row(i)) + rowcolsum(col(i)));
      });
  auto diagmap = ko::View<int*>("diagmap", params.Np);
  // compute the rowsum again for the transfer mat construction
  ko::parallel_for(
      "compute_rowcolsum", params.Np, KOKKOS_LAMBDA(const int& i) {
        rowcolsum(i) = 0.0;
        for (int j = rowmap(i); j < rowmap(i + 1); ++j) {
          rowcolsum(i) += val(j);
          if (col(j) == i) {
            diagmap(i) = j;
          }
        }
      });
  ko::parallel_for(
      "create_transfer_mat", params.Np, KOKKOS_LAMBDA(const int& i) {
        val(diagmap(i)) = val(diagmap(i)) + 1.0 - rowcolsum(row(diagmap(i)));
      });
  // std::cout << "i, row, col, val"
  //           << "\n";
  // for (int i = 0; i < nnz; ++i) {
  //   std::cout << i << ", " << row(i) << ", " << col(i) << ", " << val(i) << ", "
  //             << "\n";
  // }
  spmat_type kmat("sparse_transfer_mat", params.Np, params.Np, nnz, val, rowmap,
                  col);
  return kmat;
}

}  // namespace particles
