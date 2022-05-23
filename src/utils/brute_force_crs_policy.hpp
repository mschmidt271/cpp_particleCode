#ifndef BRUTE_FORCE_CRS_POLICY_HPP
#define BRUTE_FORCE_CRS_POLICY_HPP

#include "constants.hpp"
#include "containers.hpp"
#include "Kokkos_Core.hpp"
#include "type_defs.hpp"

namespace particles {

struct BruteForceCRSPolicy {
  static void get_nnz_mask(const ko::View<Real**>& X, const Params& params,
                                 int& nnz, ko::View<int*>& mask) {
    int Np2 = pow(params.Np, 2);
    auto ldim = params.dim;
    auto lcutdist = params.cutdist;
    auto lX = X;
    auto lNp = params.Np;
    auto lmask = mask;
    ko::parallel_reduce(
        "sum nnz",
        ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {params.Np, params.Np}),
        KOKKOS_LAMBDA(const int& i, const int& j, int& val) {
          Real dim_sum = 0.0;
          for (int k = 0; k < ldim; ++k)
          {
            dim_sum += pow(lX(k, i) - lX(k, j), 2);
          }
          Real dist = sqrt(dim_sum);
          int idx = j + (i * lNp);
          lmask(idx) = 0;
          if (dist <= lcutdist) {
            val += 1;
            lmask(idx) = 1;
          }
        },
        nnz);
    // FIXME: there's a chance this could compute nnz, depending on how I do
    // the scan (inclusive/exclusive)
    ko::parallel_scan(
        "create_reduction_maskmat", Np2,
        KOKKOS_LAMBDA(const int& i, int& update, const bool final) {
          const int val_i = lmask(i);
          if (final) {
            lmask(i) = update;
          }
          update += val_i;
        });
  }
  static SparseMatViews get_views(const ko::View<Real**>& X, const Params& params,
                        int& nnz) {
    SparseMatViews spmat_views;
    auto mask = ko::View<int*>("idx_mask", pow(params.Np, 2));
    get_nnz_mask(X, params, nnz, mask);
    spmat_views.row = ko::View<int*>("row", nnz);
    spmat_views.col = ko::View<int*>("col", nnz);
    spmat_views.val = ko::View<Real*>("val", nnz);
    spmat_views.rowmap = ko::View<int*>("rowmap", params.Np + 1);
    auto lmask = mask;
    auto ldim = params.dim;
    Real denom = params.denom;
    Real c = pow(denom  * pi, (Real) ldim / 2.0 );
    auto lX = X;
    auto lNp = params.Np;
    auto lcutdist = params.cutdist;

    ko::parallel_for(
        "form_coo_distmat", ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {lNp, lNp}),
        KOKKOS_LAMBDA(const int& i, const int& j) {
          int idx = lmask(j + (i * lNp));
          Real dim_sum = 0.0;
          for (int k = 0; k < ldim; ++k)
          {
            dim_sum += pow(lX(k, i) - lX(k, j), 2);
          }
          Real d = sqrt(dim_sum);
          if (d <= lcutdist) {
            spmat_views.row(idx) = i;
            spmat_views.col(idx) = j;
            spmat_views.val(idx) = exp(pow(d, 2) / -denom) / c;
          }
        });
    ko::parallel_for(
        "compute_rowmap", nnz, KOKKOS_LAMBDA(const int& i) {
          ko::atomic_increment(&spmat_views.rowmap(spmat_views.row(i) + 1));
        });
    ko::parallel_scan(
        "finalize_rowmap", lNp + 1,
        KOKKOS_LAMBDA(const int& i, int& update, const bool final) {
          const int val_i = spmat_views.rowmap(i);
          update += val_i;
          if (final) {
            spmat_views.rowmap(i) = update;
          }
        });
    return spmat_views;
  }
};

}  // namespace particles

#endif
