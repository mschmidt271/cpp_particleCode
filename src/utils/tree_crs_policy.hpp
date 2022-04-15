#ifndef TREE_CRS_POLICY_HPP
#define TREE_CRS_POLICY_HPP

#include "ArborX_LinearBVH.hpp"
#include "Kokkos_Core.hpp"
#include "constants.hpp"
#include "containers.hpp"
#include "type_defs.hpp"

// these are some structs that are necessary for the ArborX fixed-radius search
// this was provided by the helpful ArborX/Kokkos developer dalg24
template <class MemorySpace>
struct Spheres {
  Kokkos::View<float**, MemorySpace> points;
  float radius;
};
template <class MemorySpace>
struct ArborX::AccessTraits<Spheres<MemorySpace>, ArborX::PredicatesTag> {
  using memory_space = MemorySpace;
  using size_type = std::size_t;
  static KOKKOS_FUNCTION size_type size(Spheres<MemorySpace> const& x) {
    return x.points.extent(0);
  }
  static KOKKOS_FUNCTION auto get(Spheres<MemorySpace> const& x, size_type i) {
    return ArborX::attach(
        ArborX::intersects(ArborX::Sphere{
            ArborX::Point{x.points(i, 0), x.points(i, 1), x.points(i, 2)},
            x.radius}),
        (int)i);
  }
};

namespace particles {

struct TreeCRSPolicy {
  static SparseMatViews get_views(const ko::View<Real*>& X,
                                  const Params& params, int& nnz) {
    SparseMatViews spmat_views;
    Real denom = params.denom;
    Real c = sqrt(denom * pi);
    // ***FIXMEFIXMEFIXME***: change this when X is a 2d view (Np x dim)
    // int dim = X.extent(1);
    auto lX = X;
    auto lNp = params.Np;

    // make a float version of radius and X, as required by arborx, being
    // careful because the X view passed to arborx must have dimension 3
    ko::Profiling::pushRegion("create device/float views");
    auto fX = ko::View<float* [3], MemorySpace>("float X", lNp);
    ko::deep_copy(fX, 0.0);
    // ko::deep_copy(ko::subview(fX, ko::ALL(), ko::make_pair(0, dim)),
    //               lX);
    // ***FIXMEFIXMEFIXME***: change this when X is a 2d view (Np x dim)
    ko::parallel_for(
        "fill_floatX", lNp, KOKKOS_LAMBDA(const int& i) { fX(i, 0) = X(i); });
    float frad = float(params.cutdist);
    ko::Profiling::popRegion();

    // create the bounding value hierarchy and query it for the fixed-radius
    // search
    ko::Profiling::pushRegion("create BVH");
    ArborX::BVH<MemorySpace> bvh{ExecutionSpace(), fX};
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("create offset/indices views");
    spmat_views.rowmap = ko::View<int*, MemorySpace>("rowmap", 0);
    spmat_views.col = ko::View<int*, MemorySpace>("col", 0);
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("query BVH");
    ArborX::query(bvh, ExecutionSpace{}, Spheres<MemorySpace>{fX, frad},
                  spmat_views.col, spmat_views.rowmap);
    ko::Profiling::popRegion();

    nnz = spmat_views.col.size();
    spmat_views.row = ko::View<int*>("row", nnz);

    // keeping this around, just in case
    // sort the tree-generated columns, to make life easier when calculating val
    // for (int i = 0; i < lNp; ++i) {
    //   auto sort_view = ko::subview(
    //       spmat_views.col,
    //       ko::make_pair(spmat_views.rowmap(i), spmat_views.rowmap(i + 1)));
    //   auto device_permutation =
    //       ArborX::Details::sortObjects(ExecutionSpace{}, sort_view);
    // }

    spmat_views.val = ko::View<Real*>("val", nnz);
    ko::parallel_for(
        "compute_val", lNp, KOKKOS_LAMBDA(const int& i) {
          int begin = spmat_views.rowmap(i);
          int end = spmat_views.rowmap(i + 1);
          for (int j = begin; j < end; ++j) {
            spmat_views.val(j) =
                exp(pow(lX(i) - lX(spmat_views.col(j)), 2) / -denom) / c;
            spmat_views.row(j) = i;
          }
        });
    return spmat_views;
  }
};
}  // namespace particles

#endif
