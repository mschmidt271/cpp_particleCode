#ifndef MASS_TRANSFER_HPP
#define MASS_TRANSFER_HPP

#include "ArborX_LinearBVH.hpp"
#include "KokkosSparse_spmv.hpp"
#include "Kokkos_Core.hpp"
#include "Kokkos_Random.hpp"
#include "containers.hpp"
#include "type_defs.hpp"

namespace particles {

template <typename CRSViewPolicy>
class MassTransfer {
 public:
  // copy of the Particles' Params
  Params params;
  ko::View<Real**> X;
  ko::View<Real*> mass;
  SparseMatViews spmat_views;
  MassTransfer<CRSViewPolicy>() = default;
  MassTransfer<CRSViewPolicy>(const Params& params, const ko::View<Real**>& X,
                              ko::View<Real*>& mass);
  void transfer_mass();
  SpmatType get_transfer_mat();
  SpmatType build_sparse_transfer_mat();
  void get_crs_views(int& nnz);
};

}  // namespace particles

#endif
