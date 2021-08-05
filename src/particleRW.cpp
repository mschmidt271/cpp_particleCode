#include <iostream>
#include <vector>

#include "utilities.hpp"
// this will give the install directory location (so far)
#include "ParticleRW_config.h"

// ***TESTING***
#include <typeinfo>
#include "KokkosKernels_default_types.hpp"
#include "KokkosSparse_CrsMatrix.hpp"
#include "KokkosSparse_spmv.hpp"
using Scalar = default_scalar;
using Ordinal = default_lno_t;
using Offset = default_size_type;
using device_type = typename Kokkos::Device<
    Kokkos::DefaultExecutionSpace,
    typename Kokkos::DefaultExecutionSpace::memory_space>;
using matrix_type = typename KokkosSparse::CrsMatrix<Scalar, Ordinal,
                                                     device_type, void, Offset>;

#include "Kokkos_Core.hpp"

using namespace particles;

int main(int argc, char* argv[]) {
  Kokkos::initialize();

  {
    printf("Kokkos execution space is: %s\n",
         typeid(Kokkos::DefaultExecutionSpace).name());

    int nrows = 3;
    int ncols = 3;
    int annz = 3;
    std::vector<int> colidx = {0, 1, 2};
    std::vector<int> rowmap = {0, 1, 2, 3};
    std::vector<Real> vals = {1, 1, 1};
    matrix_type A("A", nrows, ncols, annz, vals.data(), rowmap.data(),
                  colidx.data());
    auto x = Kokkos::View<Real*>("x", 3);
    auto b = Kokkos::View<Real*>("b", 3);
    auto hx = Kokkos::create_mirror_view(x);
    auto hb = Kokkos::create_mirror_view(b);

    hx(0) = 3;
    hx(1) = 4;
    hx(2) = 5;
    Kokkos::deep_copy(x, hx);

    const Real alpha = 1.0;
    const Real beta = 0.0;

    KokkosSparse::spmv("N", alpha, A, x, beta, b);
    Kokkos::deep_copy(hb, b);

    for (int i = 0; i < 3; ++i) {
      std::cout << "hb(i) = " << hb(i) << "\n";
    }
  }

  Kokkos::finalize();

  RandyNorm rN_std(0.0, 1.0);

  // create the params object and get the values from the input file
  Params params;
  params.set_values(std::string(installPrefix) +
                    std::string("/data/particleParams.yml"));
  params.print_summary();

  // calculate number of steps to take
  params.nSteps = ceil(params.maxT / params.dt);

  // create particle vector and assign the position of all particle to X0
  std::vector<Real> pVec(params.Np, params.X0);

  // create writeParticles object for writing information to file
  ParticleIO particleIO(std::string(installPrefix) + std::string("/data/") +
                        std::string(params.pFile));

  // begin time stepping
  for (int tStep = 1; tStep <= params.nSteps; ++tStep) {
    std::cout << "time step = " << tStep << "\n";
    random_walk(pVec, params.D, params.dt, rN_std);
    particleIO.write(pVec, params, tStep);
  }

  return 0;
}
