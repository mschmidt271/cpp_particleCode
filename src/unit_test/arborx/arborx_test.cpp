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

// the below is clipped from arborx:example_cuda_access_trait.cpp
// struct PointCloud {
//   float* d_x;
//   float* d_y;
//   float* d_z;
//   int N;
// };
// struct Spheres {
//   float* d_x;
//   float* d_y;
//   float* d_z;
//   float* d_r;
//   int N;
// };

template <class MemorySpace>
struct Spheres
{
  Kokkos::View<float**, MemorySpace> points;
  float radius;
};
template <class MemorySpace>
struct ArborX::AccessTraits<Spheres<MemorySpace>, ArborX::PredicatesTag>
{
  using memory_space = MemorySpace;
  using size_type = std::size_t;
  static KOKKOS_FUNCTION size_type size(Spheres<MemorySpace> const &x)
  {
    return x.points.extent(0);
  }
  static KOKKOS_FUNCTION auto get(Spheres<MemorySpace> const &x, size_type i)
  {
    return ArborX::attach(ArborX::intersects(ArborX::Sphere{ArborX::Point{x.points(i,0), x.points(i,1), x.points(i,2)}, x.radius}), (int)i);
  }
};

// template <>
// struct ArborX::AccessTraits<PointCloud, ArborX::PrimitivesTag> {
//   static KOKKOS_FUNCTION std::size_t size(PointCloud const& cloud) {
//     return cloud.N;
//   }
//   static KOKKOS_FUNCTION ArborX::Point get(PointCloud const& cloud,
//                                            std::size_t i) {
//     return {{cloud.d_x[i], cloud.d_y[i], cloud.d_z[i]}};
//   }
//   using memory_space = MemorySpace;
// };
// template <>
// struct ArborX::AccessTraits<Spheres, ArborX::PredicatesTag> {
//   static KOKKOS_FUNCTION std::size_t size(Spheres const& d) { return d.N; }
//   static KOKKOS_FUNCTION auto get(Spheres const& d, std::size_t i) {
//     return ArborX::intersects(
//         ArborX::Sphere{{{d.d_x[i], d.d_y[i], d.d_z[i]}}, d.d_r[i]});
//   }
//   using memory_space = MemorySpace;
// };

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

  ko::ScopeGuard guard(argc, argv);

  printf("Kokkos execution space is: %s\n", typeid(ExecutionSpace).name());

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

  printf("xlen = %i\n", xlen);
  printf("dim = %i\n", dim);
  printf("radius = %f\n", radius);

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


  std::cout << "size hX(0, ko::ALL()): " << ko::subview(hX, 0, ko::ALL()).size() << "\n";
  printf("size X(0, ko::ALL()): %zu\n", ko::subview(X, 0, ko::ALL()).size());
  for (size_t i = 0; i < 3; i++)
  {
    std::cout << "hX(i, :) = " << hX(i, 0) << ", " << hX(i, 1) << ", " << hX(i, 2) << "\n";
  }


  ko::Profiling::pushRegion("create device/float views");
  auto fX = ko::View<float*[3], ko::LayoutRight, MemorySpace>("float X", xlen);
  ko::deep_copy(fX, 0.0);
  ko::deep_copy(ko::subview(fX, ko::make_pair(0, dim), ko::ALL()), ko::subview(X, ko::make_pair(0, dim), ko::ALL()));
  auto fzero = ko::View<float*, ko::LayoutRight, MemorySpace>("float 0.0", xlen);
  ko::deep_copy(fzero, 0.0);

  ko::parallel_for(
  "print fX", xlen, KOKKOS_LAMBDA(const int& i)
  {
    printf("i, fX(i, :) = %d, %g, %g, %g\n", i, fX(0, i), fX(1, i), fX(2, i));
    });

  // auto frad = ko::View<float, ko::LayoutRight, MemorySpace>("float radius");
  float frad = float(radius);
  // ko::deep_copy(frad, radius);
  auto dxlen = ko::View<int>("device xlen");
  ko::deep_copy(dxlen, xlen);
  ko::Profiling::popRegion();

  // auto fx = ko::subview(fX, 0, ko::ALL());
  // auto fy = ko::subview(fX, 1, ko::ALL());
  // auto fz = ko::subview(fX, 2, ko::ALL());

  ko::Profiling::pushRegion("create BVH");
  // ArborX::BVH<MemorySpace> bvh{ExecutionSpace, PointCloud{fx, xlen}};
  ArborX::BVH<MemorySpace> bvh{ExecutionSpace(), fX};
  ko::Profiling::popRegion();
  ko::Profiling::pushRegion("create offset/indices views");
  ko::View<int*, MemorySpace> offsets("offsets", 0);
  ko::View<int*, MemorySpace> indices("indices", 0);
  ko::Profiling::popRegion();
  ko::Profiling::pushRegion("query BVH");
  ArborX::query(bvh, ExecutionSpace{}, Spheres<MemorySpace>{fX, frad}, indices, offsets);
  ko::Profiling::popRegion();

  // Kokkos::parallel_for(Kokkos::RangePolicy<ExecutionSpace>(0, xlen),
  //                    KOKKOS_LAMBDA(const int& i) {
  //                      for (int j = offsets(i); j < offsets(i + 1); ++j)
  //                      {
  //                        printf("device %i %i\n", i, indices(j) + 1);
  //                      }
  //                    });

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
      // printf("%i", hind(j) + 1);
        if (j < hoff(i + 1) - 1)
        {
          fprintf(outFile, ", ");
          // printf(", ");
        }
      }
      fprintf(outFile, "\n");
      // printf("\n");
  }
  fclose(outFile);
  ko::Profiling::popRegion();

  return 0;
}
