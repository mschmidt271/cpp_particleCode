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

  ko::initialize(argc, argv);

  {  // Kokkos scope
    printf("Kokkos execution space is: %s\n",
           typeid(ko::DefaultExecutionSpace).name());
    ko::print_configuration(std::cout, true);

    // get the input file name from command line argument
    std::string input_file(argv[1]);
    // create the particles object
    ko::Profiling::pushRegion("ctor");
    Particles particles(input_file);
    ko::Profiling::popRegion();

    int tStep = 0;
    
    ko::Profiling::pushRegion("write positions");
    // write initial positions
    particles.particleIO.write(particles.X, particles.mass, particles.params,
                               tStep);
    ko::Profiling::popRegion();

    ko::Profiling::pushRegion("timestepping");
    // begin time stepping
    for (int tStep = 1; tStep <= particles.params.nSteps; ++tStep) {
      // std::cout << "time step = " << tStep << "\n";
      ko::Profiling::pushRegion("RW");
      particles.random_walk();
      ko::Profiling::popRegion();
      ko::Profiling::pushRegion("MT");
      particles.mass_transfer();
      ko::Profiling::popRegion();
      ko::Profiling::pushRegion("tstep_write");
      particles.particleIO.write(particles.X, particles.mass, particles.params,
                                 tStep);
      ko::Profiling::popRegion();
    }
    ko::Profiling::popRegion();

  }  // end Kokkos scope

  ko::finalize();

  return 0;
}
