#include <iostream>
#include <vector>

#include "utilities.hpp"

// ***TESTING***
#include <typeinfo>
#include "KokkosKernels_default_types.hpp"
#include "KokkosSparse_CrsMatrix.hpp"
#include "KokkosSparse_spmv.hpp"

#include "Kokkos_Core.hpp"

using namespace particles;

using Scalar = default_scalar;
using Ordinal = default_lno_t;
using Offset = default_size_type;
using device_type = typename ko::Device<
    ko::DefaultExecutionSpace,
    typename ko::DefaultExecutionSpace::memory_space>;
using matrix_type = typename KokkosSparse::CrsMatrix<Scalar, Ordinal,
                                                     device_type, void, Offset>;

namespace {

// Print usage information and exit.
void usage(const char* exe) {
  fprintf(stderr, "ERROR: Too few inputs to %s--usage:\n", exe);
  fprintf(stderr, "%s <input.yml>\n", exe);
  exit(1);
}
}

int main(int argc, char* argv[]) {
  // make sure the input file is specified, or print usage and exit
  if (argc < 2) usage(argv[0]);

  ko::initialize();

  { // Kokkos scope
    printf("Kokkos execution space is: %s\n",
         typeid(ko::DefaultExecutionSpace).name());

    // =========================================================================
    // NOTE: this block is a test of the kokkos sparse matvec and is unrelated
    // to the particle simulation
    // =========================================================================
    int nrows = 3;
    int ncols = 3;
    int annz = 3;
    std::vector<int> colidx = {0, 1, 2};
    std::vector<int> rowmap = {0, 1, 2, 3};
    std::vector<Real> vals = {1, 1, 1};
    matrix_type A("A", nrows, ncols, annz, vals.data(), rowmap.data(),
                  colidx.data());
    auto x = ko::View<Real*>("x", 3);
    auto b = ko::View<Real*>("b", 3);
    auto hx = ko::create_mirror_view(x);
    auto hb = ko::create_mirror_view(b);

    hx(0) = 3;
    hx(1) = 4;
    hx(2) = 5;
    ko::deep_copy(x, hx);

    const Real alpha = 1.0;
    const Real beta = 0.0;

    KokkosSparse::spmv("N", alpha, A, x, beta, b);
    ko::deep_copy(hb, b);

    for (int i = 0; i < 3; ++i) {
      std::cout << "hb(i) = " << hb(i) << "\n";
    }
    // =========================================================================

    // get the input file name from command line argument
    std::string input_file(argv[1]);
    // create the particles object
    Particles particles(input_file);

    int tStep = 0;

    // write initial positions
    particles.particleIO.write(particles.X, particles.params, tStep);

    // begin time stepping
    for (int tStep = 1; tStep <= particles.params.nSteps; ++tStep) {
      std::cout << "time step = " << tStep << "\n";
      particles.random_walk();
      particles.particleIO.write(particles.X, particles.params, tStep);
    }

  } // end Kokkos scope

  ko::finalize();

  return 0;
}
