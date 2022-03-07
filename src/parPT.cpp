#include <iostream>
#include <vector>

#include "utilities.hpp"

// ***TESTING***
#include <ArborX_LinearBVH.hpp>
#include <string_view>
#include <typeinfo>

#include "KokkosKernels_default_types.hpp"
#include "KokkosSparse_CrsMatrix.hpp"
#include "KokkosSparse_spmv.hpp"
#include "Kokkos_Core.hpp"
#include "type_defs.hpp"

using namespace particles;

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

  // this is essentially kokkos initialization, but supposedly
  // takes care of scoping issues proactively
  ko::ScopeGuard guard(argc, argv);

  {  // Kokkos scope
    printf("Kokkos execution space is: %s\n", typeid(ExecutionSpace).name());
    ko::print_configuration(std::cout, true);

    // get the input file name from command line argument
    std::string input_file(argv[1]);
    // create the particles object
    ko::Profiling::pushRegion("particle constructor");
    Particles particles(input_file);
    ko::Profiling::popRegion();

    int tStep = 0;

    ko::Profiling::pushRegion("write positions");
    // write initial positions/masses to file
    particles.particleIO.write(particles.X, particles.mass, particles.params,
                               tStep);
    ko::Profiling::popRegion();

    ko::Profiling::pushRegion("timestepping");
    // begin time stepping
    for (int tStep = 1; tStep <= particles.params.nSteps; ++tStep) {
      // random walk and mass transfer
      ko::Profiling::pushRegion("RW");
      particles.random_walk();
      ko::Profiling::popRegion();
      ko::Profiling::pushRegion("MT");
      particles.mass_transfer();
      ko::Profiling::popRegion();
      ko::Profiling::pushRegion("tstep_write");
      // write updated particle info to file
      particles.particleIO.write(particles.X, particles.mass, particles.params,
                                 tStep);
      ko::Profiling::popRegion();
    }
    ko::Profiling::popRegion();

  }  // end Kokkos scope

  return 0;
}
