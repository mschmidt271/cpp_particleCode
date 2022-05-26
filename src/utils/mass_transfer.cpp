#include "mass_transfer.hpp"

namespace particles {

template <typename CRSViewPolicy>
MassTransfer<CRSViewPolicy>::MassTransfer(const Params& _params,
                                          const ko::View<Real**>& _X,
                                          ko::View<Real*>& _mass)
    : params(_params) {
  X = _X;
  mass = _mass;
}

template <typename CRSViewPolicy>
void MassTransfer<CRSViewPolicy>::transfer_mass() {
  if (params.pctRW < 1.0) {
    ko::Profiling::pushRegion("build_Tmat");
    // SpmatType mat = get_transfer_mat();
    SpmatType kmat = build_sparse_transfer_mat();
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("allocate");
    auto tmpmass = ko::View<Real*>("tmpmass", params.Np);
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("deepcopy");
    ko::deep_copy(tmpmass, mass);
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("matvec");
    KokkosSparse::spmv("N", 1.0, kmat, tmpmass, 0.0, mass);
    ko::Profiling::popRegion();
  }
}

template <typename CRSViewPolicy>
SpmatType MassTransfer<CRSViewPolicy>::build_sparse_transfer_mat() {
  int nnz = 0;
  auto lX = X;
  auto lNp = params.Np;
  auto rowcolsum = ko::View<Real*>("rowcolsum", lNp);
  get_crs_views(nnz);
  // NOTE: possibly try a team policy here, a la sec. 8.1 in the Kokkos v3
  // paper
  // NOTE: may also be more efficient using auto z = mat.row(i)
  // does a built in sum() method exist?
  // it's probably just a parfor in the end...
  auto lspmat = spmat_views;
  ko::parallel_for(
      "compute_rowcolsum", lNp, KOKKOS_LAMBDA(const int& i) {
        for (int j = lspmat.rowmap(i); j < lspmat.rowmap(i + 1); ++j) {
          rowcolsum(i) += lspmat.val(j);
        }
      });
  // NOTE: we may have to do this more than once if we run into issues.
  // I suspect we should be ok, though, since we still guarantee symmetry
  ko::parallel_for(
      "normalize_mat", nnz, KOKKOS_LAMBDA(const int& i) {
        lspmat.val(i) = 2.0 * lspmat.val(i) /
                        (rowcolsum(lspmat.row(i)) + rowcolsum(lspmat.col(i)));
      });
  auto diagmap = ko::View<int*>("diagmap", lNp);
  // compute the rowsum again for the transfer mat construction
  ko::parallel_for(
      "compute_rowcolsum", lNp, KOKKOS_LAMBDA(const int& i) {
        rowcolsum(i) = 0.0;
        for (int j = lspmat.rowmap(i); j < lspmat.rowmap(i + 1); ++j) {
          rowcolsum(i) += lspmat.val(j);
          if (lspmat.col(j) == i) {
            diagmap(i) = j;
          }
        }
      });
  ko::parallel_for(
      "create_transfer_mat", lNp, KOKKOS_LAMBDA(const int& i) {
        lspmat.val(diagmap(i)) =
            lspmat.val(diagmap(i)) + 1.0 - rowcolsum(lspmat.row(diagmap(i)));
      });
  SpmatType kmat("sparse_transfer_mat", lNp, lNp, nnz, spmat_views.val,
                 spmat_views.rowmap, spmat_views.col);
  return kmat;
}

template <typename CRSViewPolicy>
void MassTransfer<CRSViewPolicy>::get_crs_views(int& nnz) {
  spmat_views = CRSViewPolicy::get_views(X, params, nnz);
}

}  // namespace particles

#include "brute_force_crs_policy.hpp"
template class particles::MassTransfer<particles::BruteForceCRSPolicy>;

#include "tree_crs_policy.hpp"
template class particles::MassTransfer<particles::TreeCRSPolicy>;
