#include "mass_transfer.hpp"

namespace particles {

// anonymous namespace
namespace {

static constexpr double pi = 3.14159265358979323846264;

}  // end anonymous namespace

struct BruteForceCRSPolicy {
  static void get_views(const int& nnz, const ko::View<int*>& mask, ko::View<int*>& row,
                ko::View<int*>& col, ko::View<Real*>& val,
                ko::View<int*>& rowmap) {

    // ***put nnz calc here***
    auto lmask = mask;
    Real denom = params.denom;
    Real c = sqrt(denom * pi);
    auto lX = X;
    auto lNp = params.Np;
    auto lcutdist = params.cutdist;

    ko::parallel_for(
        "form_coo_distmat", ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {lNp, lNp}),
        KOKKOS_LAMBDA(const int& i, const int& j) {
          int idx = lmask(j + (i * lNp));
          Real d = fabs(lX(i) - lX(j));
          if (d <= lcutdist) {
            row(idx) = i;
            col(idx) = j;
            val(idx) = exp(pow(d, 2) / -denom) / c;
          }
        });
    ko::parallel_for(
        "compute_rowmap", nnz, KOKKOS_LAMBDA(const int& i) {
          ko::atomic_increment(&rowmap(row(i) + 1));
        });
    ko::parallel_scan(
        "finalize_rowmap", lNp + 1,
        KOKKOS_LAMBDA(const int& i, int& update, const bool final) {
          const int val_i = rowmap(i);
          update += val_i;
          if (final) {
            rowmap(i) = update;
          }
        });
  }
};

}  // namespace particles
