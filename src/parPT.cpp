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
using device_type =
    typename ko::Device<ko::DefaultExecutionSpace,
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
}  // namespace

int main(int argc, char* argv[]) {
  // make sure the input file is specified, or print usage and exit
  if (argc < 2) usage(argv[0]);

  ko::initialize();

  {  // Kokkos scope
    printf("Kokkos execution space is: %s\n",
           typeid(ko::DefaultExecutionSpace).name());

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
      // particles.random_walk();
      particles.mass_transfer();
      particles.particleIO.write(particles.X, particles.params, tStep);
    }

  }  // end Kokkos scope

  ko::finalize();

  return 0;
}
