#ifndef MASS_TRANSFER_HPP
#define MASS_TRANSFER_HPP

#include "ArborX_LinearBVH.hpp"
#include "containers.hpp"
#include "KokkosSparse_spmv.hpp"
#include "Kokkos_Core.hpp"
#include "Kokkos_Random.hpp"
#include "type_defs.hpp"
#include "version_info.hpp"

namespace particles {

template
class MassTransfer<typename CRSViewPolicy> {
 public:
  // copy of the Particles' Params
  Params params;
  ko::View<Real*> X;
  ko::View<Real*> mass;
  // Data data;
  // mask for distmat reduction
  // FIXME: is it better to make this int or boolean? I scan and overwrite to
  // generate the COO indices, so it seems like int is the right call
  ko::View<int*> mask;
  MassTransfer() = default;
  MassTransfer(const Params& params, const ko::View<Real*>& X,
               ko::View<Real*>& mass);
  void transfer_mass();
  SpmatType get_transfer_mat();
  void get_nnz_mask_brute(int& nnz);
  SpmatType build_sparse_transfer_mat(const int& nnz);
  void get_crs_views(const int& nnz, const ko::View<int*>& mask,
                           ko::View<int*>& row, ko::View<int*>& col,
                           ko::View<Real*>& val, ko::View<int*>& rowmap);
};

}  // namespace particles

#endif
