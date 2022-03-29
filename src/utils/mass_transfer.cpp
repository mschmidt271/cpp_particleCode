#include "mass_transfer.hpp"

namespace particles {

// anonymous namespace
namespace {

static constexpr double pi = 3.14159265358979323846264;

}  // end anonymous namespace

MassTransfer::MassTransfer(const Params& _params, const ko::View<Real*>& _X,
                           ko::View<Real*>& _mass)
    : params(_params) {
  X = _X;
  mass = _mass;
  mask = ko::View<int*>("idx_mask", pow(params.Np, 2));
}

void MassTransfer::transfer_mass() {
  if (params.pctRW < 1.0) {
    ko::Profiling::pushRegion("build_Tmat");
    SpmatType mat = get_transfer_mat();
    ko::Profiling::popRegion();
    ko::Profiling::pushRegion("matvec");
    auto tmpmass = ko::View<Real*>("tmpmass", params.Np);
    ko::deep_copy(tmpmass, mass);
    KokkosSparse::spmv("N", 1.0, mat, tmpmass, 0.0, mass);
    ko::Profiling::popRegion();
  }
}

SpmatType MassTransfer::get_transfer_mat() {
  int nnz = 0;
  get_nnz_mask_brute(nnz);
  SpmatType kmat = build_sparse_transfer_mat(nnz);
  return kmat;
}

void MassTransfer::get_nnz_mask_brute(int& nnz) {
  int Np2 = pow(params.Np, 2);
  auto lcutdist = params.cutdist;
  auto lX = X;
  auto lNp = params.Np;
  auto lmask = mask;
  ko::parallel_reduce(
      "sum nnz", ko::MDRangePolicy<ko::Rank<2>>({0, 0}, {params.Np, params.Np}),
      KOKKOS_LAMBDA(const int& i, const int& j, int& val) {
        Real dist = fabs(lX(i) - lX(j));
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

SpmatType MassTransfer::build_sparse_transfer_mat(const int& nnz) {
  auto row = ko::View<int*>("row", nnz);
  auto col = ko::View<int*>("col", nnz);
  auto val = ko::View<Real*>("val", nnz);
  auto rowmap = ko::View<int*>("rowmap", params.Np + 1);
  get_crs_views_brute(nnz, mask, row, col, val, rowmap);
  auto lX = X;
  auto lNp = params.Np;
  auto rowcolsum = ko::View<Real*>("rowcolsum", lNp);
  // NOTE: possibly try a team policy here, a la sec. 8.1 in the Kokkos v3
  // paper
  // NOTE: may also be more efficient using auto z = mat.row(i)
  ko::parallel_for(
      "compute_rowcolsum", lNp, KOKKOS_LAMBDA(const int& i) {
        for (int j = rowmap(i); j < rowmap(i + 1); ++j) {
          rowcolsum(i) += val(j);
        }
      });
  // NOTE: we may have to do this more than once if we run into issues.
  // I suspect we should be ok, though, since we still guarantee symmetry
  ko::parallel_for(
      "normalize_mat", nnz, KOKKOS_LAMBDA(const int& i) {
        val(i) = 2.0 * val(i) / (rowcolsum(row(i)) + rowcolsum(col(i)));
      });
  auto diagmap = ko::View<int*>("diagmap", lNp);
  // compute the rowsum again for the transfer mat construction
  ko::parallel_for(
      "compute_rowcolsum", lNp, KOKKOS_LAMBDA(const int& i) {
        rowcolsum(i) = 0.0;
        for (int j = rowmap(i); j < rowmap(i + 1); ++j) {
          rowcolsum(i) += val(j);
          if (col(j) == i) {
            diagmap(i) = j;
          }
        }
      });
  ko::parallel_for(
      "create_transfer_mat", lNp, KOKKOS_LAMBDA(const int& i) {
        val(diagmap(i)) = val(diagmap(i)) + 1.0 - rowcolsum(row(diagmap(i)));
      });
  SpmatType kmat("sparse_transfer_mat", lNp, lNp, nnz, val, rowmap, col);
  // fmt::print("nnz = {}\n", kmat.nnz());
  return kmat;
}

template<typename CRSViewPolicy>
void MassTransfer::get_crs_views<CRSViewPolicy>(const int& nnz,
                                       const ko::View<int*>& mask,
                                       ko::View<int*>& row, ko::View<int*>& col,
                                       ko::View<Real*>& val,
                                       ko::View<int*>& rowmap) {
  CRSViewPolicy::get_views(nnz, mask, row, col, val, rowmap);
}

}  // namespace particles
