// #include <iostream>
// #include <vector>

// #include "utilities.hpp"

// ***TESTING***
#include <ArborX_LinearBVH.hpp>
#include <string_view>
#include <typeinfo>

#include "parPT_config.h"
#include "yaml-cpp/yaml.h"

#include "Kokkos_Core.hpp"
#include "KokkosKernels_default_types.hpp"
// #include "KokkosSparse_CrsMatrix.hpp"
// #include "KokkosSparse_spmv.hpp"

// using namespace particles;

// namespace ax {

typedef double Real;
namespace ko = Kokkos;

using Scalar = default_scalar;
using Ordinal = default_lno_t;
using Offset = default_size_type;
using ExecutionSpace = Kokkos::DefaultExecutionSpace;
using MemorySpace = ExecutionSpace::memory_space;
using device_type = typename ko::Device<ExecutionSpace, MemorySpace>;
// using matrix_type = typename KokkosSparse::CrsMatrix<Scalar, Ordinal,
//                                                      device_type, void, Offset>;

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

// } // end namespace ax

namespace {

// Print usage information and exit.
void usage(const char* exe) {
  fprintf(stderr, "ERROR: Too few inputs to %s--usage:\n", exe);
  fprintf(stderr, "%s <input_pts.yml> <outfile.txt>\n", exe);
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
  if (argc < 3) usage(argv[0]);

  ko::ScopeGuard kokkos(argc, argv);

  // {  // Kokkos scope
    printf("Kokkos execution space is: %s\n", typeid(ExecutionSpace).name());

    // if (Kokkos::SpaceAccessibility<ExecutionSpace, MemorySpace>::accessible)
    // {
    //   printf("True");
    // }
    // else
    // {
    //   printf("False");
    // }


    // printf(ko::SpaceAccessibility<ExecutionSpace, MemorySpace>::accessible);
    // ko::print_configuration(std::cout, true);

    std::string infile = install_prefix;
    infile += "/unit_test/arborx/";
    infile += argv[1];

    int xlen, dim;
    Real radius;


    ko::Profiling::pushRegion("read params from yaml");
    auto root = YAML::LoadFile(infile);
    xlen = root["N"].as<int>();
    dim = root["dim"].as<int>();
    radius = root["dist"].as<Real>();
    ko::Profiling::popRegion();

    ko::Profiling::pushRegion("create X views and read from yaml");
    auto X = ko::View<Real**>("X", dim, xlen);
    ko::deep_copy(X, 0.0);
    auto hX = ko::create_mirror_view(X);
    ko::deep_copy(hX, 0.0);

    std::vector<std::string> coords {"x", "y", "z"};

    for (int i = 0; i < dim; ++i) {
      auto pts = root["pts"];
      auto node = pts[coords[i]];
      int j = 0;
      for (auto iter : node) {
        hX(i, j) = iter.as<Real>();
        ++j;
      }
    }
    ko::deep_copy(X, hX);
    ko::Profiling::popRegion();

    ko::Profiling::pushRegion("create device/float views");
    auto fX = ko::View<float**>("float X", 3, xlen);
    ko::deep_copy(fX, 0.0);
    ko::deep_copy(ko::subview(fX, ko::make_pair(0, dim), ko::ALL()), ko::subview(X, ko::make_pair(0, dim), ko::ALL()));
    auto fzero = ko::View<float*>("float 0.0", xlen);
    ko::deep_copy(fzero, 0.0);

    auto frad = ko::View<float*>("float radius", xlen);
    ko::deep_copy(frad, radius);
    auto dxlen = ko::View<int>("device xlen");
    ko::deep_copy(dxlen, xlen);
    ko::Profiling::popRegion();

    ko::Profiling::pushRegion("create BVH");
    ArborX::BVH<MemorySpace> bvh{ExecutionSpace(), PointCloud{&ko::subview(fX, 0, ko::ALL())(), &ko::subview(fX, 1, ko::ALL())(), &ko::subview(fX, 2, ko::ALL())(), xlen}};
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("create offset/indices views");
    ko::View<int*, MemorySpace> offsets("offsets", 0);
    ko::View<int*, MemorySpace> indices("indices", 0);
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("query BVH");
    ArborX::query(bvh, ExecutionSpace(), Spheres{&ko::subview(fX, 0, ko::ALL())(), &ko::subview(fX, 1, ko::ALL())(), &ko::subview(fX, 2, ko::ALL())(), &frad(), xlen}, indices, offsets);
    ko::Profiling::popRegion();

    ko::Profiling::pushRegion("deep copy results to host");
    auto hoff = ko::create_mirror_view(offsets);
    auto hind = ko::create_mirror_view(indices);
    ko::deep_copy(hoff, offsets);
    ko::deep_copy(hind, indices);
    ko::Profiling::popRegion();

    ko::Profiling::pushRegion("write to file");
    FILE* outFile;
    std::string fname = install_prefix;
    fname += "/unit_test/arborx/";
    fname += argv[2];
    outFile = fopen(fname.c_str(),"w");

    for (int i = 0; i < xlen; ++i)
    {
      for (int j = hoff(i); j < hoff(i + 1); ++j)
        {
         fprintf(outFile, "%i", hind(j) + 1);
         if (j < hoff(i + 1) - 1)
         {
            fprintf(outFile, ", ");
         }
        }
        fprintf(outFile, "\n");
    }
    fclose(outFile);
    ko::Profiling::popRegion();

  // }  // end Kokkos scope

  // ko::finalize();

  return 0;
}
