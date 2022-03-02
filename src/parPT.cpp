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

using namespace particles;

using Scalar = default_scalar;
using Ordinal = default_lno_t;
using Offset = default_size_type;
using ExecutionSpace = Kokkos::DefaultExecutionSpace;
using MemorySpace = ExecutionSpace::memory_space;
using device_type = typename ko::Device<ExecutionSpace, MemorySpace>;
using matrix_type = typename KokkosSparse::CrsMatrix<Scalar, Ordinal,
                                                     device_type, void, Offset>;

// the below is clipped from arborx:example_cuda_access_trait.cpp
struct PointCloud {
  float* d_x;
  float* d_y;
  float* d_z;
  int N;
};
struct Spheres {
  float* d_x;
  float* d_y;
  float* d_z;
  float* d_r;
  int N;
};
template <>
struct ArborX::AccessTraits<PointCloud, ArborX::PrimitivesTag> {
  static KOKKOS_FUNCTION std::size_t size(PointCloud const& cloud) {
    return cloud.N;
  }
  static KOKKOS_FUNCTION ArborX::Point get(PointCloud const& cloud,
                                           std::size_t i) {
    return {{cloud.d_x[i], cloud.d_y[i], cloud.d_z[i]}};
  }
  using memory_space = MemorySpace;
};
template <>
struct ArborX::AccessTraits<Spheres, ArborX::PredicatesTag> {
  static KOKKOS_FUNCTION std::size_t size(Spheres const& d) { return d.N; }
  static KOKKOS_FUNCTION auto get(Spheres const& d, std::size_t i) {
    return ArborX::intersects(
        ArborX::Sphere{{{d.d_x[i], d.d_y[i], d.d_z[i]}}, d.d_r[i]});
  }
  using memory_space = MemorySpace;
};

// the below is clipped from arborx:example_callback.hpp
struct NearestToOrigin {
  int k;
};
template <>
struct ArborX::AccessTraits<NearestToOrigin, ArborX::PredicatesTag> {
  static KOKKOS_FUNCTION std::size_t size(NearestToOrigin) { return 1; }
  static KOKKOS_FUNCTION auto get(NearestToOrigin d, std::size_t) {
    return ArborX::nearest(ArborX::Point{0, 0, 0}, d.k);
  }
  using memory_space = MemorySpace;
};

namespace {

// Print usage information and exit.
void usage(const char* exe) {
  fprintf(stderr, "ERROR: Too few inputs to %s--usage:\n", exe);
  fprintf(stderr, "%s <input.yml>\n", exe);
  exit(1);
}

// **TESTING**
// // clever code from SO (seems to only work with clang or c++17)
// template <typename T>
// constexpr auto type_name() {
//   std::string_view name, prefix, suffix;
// #ifdef __clang__
//   name = __PRETTY_FUNCTION__;
//   prefix = "auto type_name() [T = ";
//   suffix = "]";
// #elif defined(__GNUC__)
//   name = __PRETTY_FUNCTION__;
//   prefix = "constexpr auto type_name() [with T = ";
//   suffix = "]";
// #elif defined(_MSC_VER)
//   name = __FUNCSIG__;
//   prefix = "auto __cdecl type_name<";
//   suffix = ">(void)";
// #endif
//   name.remove_prefix(prefix.size());
//   name.remove_suffix(suffix.size());
//   return name;
// }
}  // namespace

int main(int argc, char* argv[]) {
  // make sure the input file is specified, or print usage and exit
  if (argc < 2) usage(argv[0]);

  ko::initialize(argc, argv);

  {  // Kokkos scope
    printf("Kokkos execution space is: %s\n", typeid(ExecutionSpace).name());
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
      particles.particleIO.write(particles.X, particles.mass,
      particles.params,
                                 tStep);
      ko::Profiling::popRegion();
    }
    ko::Profiling::popRegion();

  }  // end Kokkos scope

  ko::finalize();

  return 0;
}
