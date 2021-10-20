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

// // this is directly from the kokkos wiki page section on parallel reduce
// // https://github.com/kokkos/kokkos/wiki/ParallelDispatch
// struct ColumnSums {
//   // In this case, the reduction result is an array of Real.
//   using value_type = Real[];

//   using size_type = View<Real**>::size_type;

//   // Tell Kokkos the result array's number of entries.
//   // This must be a public value in the functor.
//   size_type value_count;

//   View<Real**> X_;

//   // As with the above examples, you may supply an
//   // execution_space typedef. If not supplied, Kokkos
//   // will use the default execution space for this functor.

//   // Be sure to set value_count in the constructor.
//   ColumnSums(const View<Real**>& X, const View<Real*>& range)
//       : value_count(X.extent(1)),  // # columns in X
//         X_(X) {}

//   // value_type here is already a "reference" type,
//   // so we don't pass it in by reference here.
//   KOKKOS_INLINE_FUNCTION void operator()(const size_type i,
//                                          value_type sum) const {
//     // You may find it helpful to put pragmas above this loop
//     // to convince the compiler to vectorize it. This is
//     // probably only helpful if the View type has LayoutRight.
//     for (size_type j = 0; j < value_count; ++j) {
//       sum[j] += X_(i, j);
//     }
//   }

//   // value_type here is already a "reference" type,
//   // so we don't pass it in by reference here.
//   KOKKOS_INLINE_FUNCTION void join(volatile value_type dst,
//                                    const volatile value_type src) const {
//     for (size_type j = 0; j < value_count; ++j) {
//       dst[j] += src[j];
//     }
//   }

//   KOKKOS_INLINE_FUNCTION void init(value_type sum) const {
//     for (size_type j = 0; j < value_count; ++j) {
//       sum[j] = 0.0;
//     }
//   }
// };

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
  denom = 4 * D * dt;
  cdist_coeff = file_params["cdist_coeff"].as<Real>();
  cutdist = cdist_coeff * sqrt(denom);
  pFile = file_params["pFile"].as<std::string>();
  nSteps = ceil(maxT / dt);
}

void Params::print_summary() {
  std::cout << "************************************************************\n";
  std::cout << "  " << particles::toy_problem_intro();
  std::cout << "************************************************************\n";
  std::cout << "Np = " << Np << "\n";
  std::cout << "IC type = " << IC_str << "\n";
  std::cout << "omega = [";
  for (auto i : omega) {std::cout << i << " ";}
  std::cout << "] \n";
  std::cout << "X0 = " << X0 << "\n";
  std::cout << "maxT = " << maxT << "\n";
  std::cout << "dt = " << dt << "\n";
  std::cout << "D = " << D << "\n";
  std::cout << "cdist_coeff = " << cdist_coeff << "\n";
  std::cout << "cutdist = " << cutdist << "\n";
  std::cout << "pFile = " << pFile << "\n";
  std::cout << "nSteps = " << nSteps << "\n";
  std::cout << "*************************************************************\n";
}

// constructor for WriteParticles object that takes in the filename f and
// creates an outFile object
ParticleIO::ParticleIO(std::string f) : outFile(f) {}

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
      // deep copy the params X0 (host) to device
      auto X0 = ko::View<Real>("X0");
      ko::deep_copy(X0, params.X0);
      // fill the X view so particles are all located at X0
      ko::parallel_for(
          "InitX", params.Np, KOKKOS_LAMBDA(const int& i) { X(i) = X0(); });
      break;
    }
    case equi: {
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
      ko::parallel_for(params.Np, RandomUniform<RandPoolType>(X, rand_pool,
                                                              params.omega[0],
                                                              params.omega[1]));
      break;
    }
  }
}

// parallelized random walk function that calls the RandomWalk functor
void Particles::random_walk() {
  ko::parallel_for(params.Np, RandomWalk<RandPoolType>(X, rand_pool, 0.0,
                                                       2.0 * D() * dt()));
}

void Particles::mass_transfer() {
  spmat_type mat = get_transfer_mat();
  auto x = ko::View<Real*>("x", params.Np);
  auto b = ko::View<Real*>("b", params.Np);
  for (int i = 0; i < params.Np; ++i)
  {
    x(i) = i;
  }
  KokkosSparse::spmv("N", 1.0, mat, x, 0.0, b);
  std::cout << "b = " << "\n";
  for (int i = 0; i < params.Np; ++i)
  {
    std::cout << b(i) << "\n";
  }
}

spmat_type Particles::get_transfer_mat() {
  Real Np2 = pow(params.Np, 2);
  auto dmat = ko::View<Real**>("dist_mat", params.Np, params.Np);
  Real cutdist = params.cutdist;
  int nnz = 0;
  auto mask = ko::View<Real*>("idx mask", Np2);
  ko::parallel_reduce(
      "sum nnz", ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {params.Np, params.Np}),
      KOKKOS_LAMBDA(const int& i, const int& j, int& val) {
        dmat(i, j) = abs(X(i) - X(j));
        if (dmat(i, j) <= cutdist) {
          val += 1;
          mask(j + (i * params.Np)) = 1;
        }
      },
      nnz);
  ko::parallel_scan("create_reduction_maskmat", Np2,
      KOKKOS_LAMBDA(const int& i, Real& update, const bool final) {
        const Real val_i = mask(i);
        if (final) {
          mask(i) = update;
        }
        update += val_i;
      });
  auto kmat = sparse_kernel_mat(dmat, nnz, mask);
  return kmat;
}

spmat_type Particles::sparse_kernel_mat(const ko::View<Real**>& dmat,
                                        const int& nnz,
                                        const ko::View<Real*>& mask) {
  auto row = ko::View<int*>("row", nnz);
  auto col = ko::View<int*>("col", nnz);
  auto val = ko::View<Real*>("val", nnz);
  auto rowmap = ko::View<int*>("rowmap", params.Np + 1);
  // auto diagmask = ko::View<int*>("diagmask", nnz);
  ko::deep_copy(rowmap, 0.0);
  ko::deep_copy(col, 0.0);
  ko::deep_copy(val, 0.0);
  Real cutdist = params.cutdist;
  ko::parallel_for(
      "form_sparse_kmat",
      ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {params.Np, params.Np}),
      KOKKOS_LAMBDA(const int& i, const int& j) {
        int idx = mask(j + (i * params.Np));
        if (dmat(i, j) <= cutdist) {
          row(idx) = i;
          col(idx) = j;
          val(idx)= exp(pow(dmat(i, j), 2) / -params.denom) / sqrt(params.denom * pi);
        }
      });
  // ***NOTE: possibly sort by row here?***
  ko::parallel_for(
      "compute_rowmap", nnz,
      KOKKOS_LAMBDA(const int& i) { rowmap(row(i) + 1)++; });
  ko::parallel_scan("finalize_rowmap",
      params.Np + 1,
      KOKKOS_LAMBDA(const int& i, int& update, const bool final) {
        const int val_i = rowmap(i);
        update += val_i;
        if (final) {
          rowmap(i) = update;
        }
      });
  auto rowcolsum = ko::View<Real*>("rowcolsum", params.Np);
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
        // val(i) = val(i) / ((rowcolsum(row(i)) + rowcolsum(col(i))) / 2);
        val(i) = 2.0 * val(i) / (rowcolsum(row(i)) + rowcolsum(col(i)));
      });
  auto diagmap = ko::View<int*>("diagmap", params.Np);
  // compute the rowsum again for the transfer mat construction
  ko::parallel_for(
      "compute_rowcolsum", params.Np, KOKKOS_LAMBDA(const int& i) {
        rowcolsum(i) = 0.0;
        for (int j = rowmap(i); j < rowmap(i + 1); ++j) {
          rowcolsum(i) += val(j);
          if (col(j) == i)
          {
            diagmap(i) = j;
          }
        }
      });
  ko::parallel_for(
      "creat_transfer_mat", params.Np, KOKKOS_LAMBDA(const int& i) {
        val(diagmap(i)) = val(diagmap(i)) + 1.0 - rowcolsum(row(diagmap(i)));
      });
  std::cout << "i, row, col, val" << "\n";
  for (int i = 0; i < nnz; ++i)
  {
    std::cout << i << ", " << row(i) << ", " << col(i) << ", " << val(i) << ", " << "\n";
  }
  spmat_type kmat("sparse_transfer_mat", params.Np, params.Np, nnz, val, rowmap,
                  col);
  return kmat;
}

// spmat_type Particles::get_transfer_mat(spmat_type& kmat) {
//   spmat_type tmat;

//   auto ones = ko::View<Real*>("ones", params.Np);
//   auto rowsum = ko::View<Real*>("rowsum", params.Np);
//   auto colsum = ko::View<Real*>("colsum", params.Np);
//   ko::deep_copy(ones, 1.0);
//   // NOTE: for matrix A and compatible column vector x of all 1's,
//     // rowsum(A) = Ax
//     // colsum(A) = transpose(A)x
//   KokkosSparse::spmv("N", 1.0, kmat, ones, 0.0, rowsum);
//   std::cout << "rowsum = " << "\n";
//   for (int i = 0; i < params.Np; ++i)
//   {
//     std::cout << rowsum(i) << "\n";
//   }
//   auto rowmap = ko::View<int*>("rowmap", params.Np + 1);
//   auto col = ko::View<int*>("col", params.Np);
//   auto val = ko::View<Real*>("val", params.Np);
//   ko::deep_copy(rowmap, params.Np);
//   ko::parallel_for(
//       "create_norm_vector", params.Np,
//       KOKKOS_LAMBDA(const int& i) {
//         rowmap(i) = i;
//         col(i) = i;
//         val(i) = ones(i) / rowsum(i);
//       });
//   spmat_type norm_mat("sparse_normmat", params.Np, params.Np, params.Np, val, rowmap,
//                   col);
//   // spmat_type input_copy(kmat);
//   // Create KokkosKernelHandle
//   using KernelHandle = KokkosKernels::Experimental::KokkosKernelsHandle<
//       Offset, Ordinal, Scalar, execution_space, memory_space, memory_space>;
//   KernelHandle kh;
//   kh.set_team_work_size(16);
//   kh.set_dynamic_scheduling(true);
//   // std::string myalg("SPGEMM_KK_SPEED");
//   //   KokkosSparse::SPGEMMAlgorithm spgemm_algorithm =
//   //       KokkosSparse::StringToSPGEMMAlgorithm(myalg);
//   //   kh.create_spgemm_handle(spgemm_algorithm);
//   // KokkosSparse::spgemm_numeric(kh, kmat, false, norm_mat, false, tmat);
//   // std::cout << "tmat = " << "\n";
//   // for (int i = 0; i < params.Np; ++i)
//   // {
//   //   for (int j = 0; j < params.Np; ++j)
//   //   {
//   //     std::cout << tmat(i, j) << "\n";
//   //   }
//   // }
//   return tmat;
// }

}  // namespace particles
