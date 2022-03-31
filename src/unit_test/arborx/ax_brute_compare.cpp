// #include <assert.h>

#include "ArborX_LinearBVH.hpp"
#include "Kokkos_Core.hpp"
#include "Kokkos_Sort.hpp"
#include "brute_force_crs_policy.hpp"
#include "containers.hpp"
#include "mass_transfer.hpp"
#include "spdlog/formatter.h"
#include "tree_crs_policy.hpp"
#include "type_defs.hpp"
#include "yaml-cpp/yaml.h"

using namespace particles;

namespace {

// Print usage information and exit.
void usage(const char* exe) {
  fmt::print(stderr, "ERROR: Too few inputs to {}--usage:\n", exe);
  fmt::print(stderr, "{} <input_pts.yml> <outfile.txt>\n", exe);
  exit(1);
}

}  // end namespace

int main(int argc, char* argv[]) {
  // print usage if to few args are provided
  if (argc < 2) usage(argv[0]);

  // this is essentially kokkos initialization, but supposedly
  // takes care of scoping issues proactively
  ko::ScopeGuard guard(argc, argv);

  fmt::print("Kokkos execution space is: {}\n", typeid(ExecutionSpace).name());

  // set the string giving the input file
  std::string infile = install_prefix;
  infile += "/unit_test/arborx/";
  infile += argv[1];

  // host parameters
  // note that radius is a real here and then recast as float for arborx, since
  // that is the way it will be for the actual code
  int xlen, dim;
  Real radius;

  // get the parameters from the input file
  ko::Profiling::pushRegion("read params from yaml");
  auto root = YAML::LoadFile(infile);
  xlen = root["N"].as<int>();
  dim = root["dim"].as<int>();
  radius = root["dist"].as<Real>();
  ko::Profiling::popRegion();

  // create the particle location views
  ko::Profiling::pushRegion("create X views and read from yaml");
  auto X = ko::View<Real**>("X", xlen, dim);
  ko::deep_copy(X, 0.0);
  auto hX = ko::create_mirror_view(X);
  ko::deep_copy(hX, 0.0);

  // for indexing the following
  std::vector<std::string> coords{"x", "y", "z"};

  // loop over dimension and particles to fill X view
  for (int j = 0; j < dim; ++j) {
    auto pts = root["pts"];
    auto node = pts[coords[j]];
    int i = 0;
    for (auto iter : node) {
      hX(i, j) = iter.as<Real>();
      ++i;
    }
  }
  ko::deep_copy(X, hX);
  ko::Profiling::popRegion();

  Params params;
  params.Np = xlen;
  params.cutdist = radius;
  params.pctRW = 0.5;
  params.denom = 6.384;
  ko::View<Real*> mass;
  auto X1d = ko::subview(X, ko::ALL(), 0);
  auto mass_trans = MassTransfer<BruteForceCRSPolicy>(params, X1d, mass);
  auto temp = mass_trans.build_sparse_transfer_mat();

  auto hcol = ko::create_mirror_view(mass_trans.spmat_views.col);
  auto hrow = ko::create_mirror_view(mass_trans.spmat_views.row);
  auto hrowmap = ko::create_mirror_view(mass_trans.spmat_views.rowmap);
  auto hval = ko::create_mirror_view(mass_trans.spmat_views.val);
  ko::deep_copy(hcol, mass_trans.spmat_views.col);
  ko::deep_copy(hrow, mass_trans.spmat_views.row);
  ko::deep_copy(hrowmap, mass_trans.spmat_views.rowmap);
  ko::deep_copy(hval, mass_trans.spmat_views.val);

  auto mass_trans_tree = MassTransfer<TreeCRSPolicy>(params, X1d, mass);
  auto temp2 = mass_trans_tree.build_sparse_transfer_mat();

  auto hcol_tree = ko::create_mirror_view(mass_trans_tree.spmat_views.col);
  auto hrow_tree = ko::create_mirror_view(mass_trans_tree.spmat_views.row);
  auto hrowmap_tree =
      ko::create_mirror_view(mass_trans_tree.spmat_views.rowmap);
  auto hval_tree = ko::create_mirror_view(mass_trans_tree.spmat_views.val);

  // sort the tree-generated columns, so as to match the naturally ordered brute
  // force ones. then apply the same permutation to the val view
  for (int i = 0; i < xlen; ++i) {
    auto sort_view = ko::subview(
        hcol_tree, ko::make_pair(hrowmap_tree(i), hrowmap_tree(i + 1)));
    auto permutation =
        ArborX::Details::sortObjects(ExecutionSpace{}, sort_view);
    auto val_sort_view = ko::subview(
        hval_tree, ko::make_pair(hrowmap_tree(i), hrowmap_tree(i + 1)));
    auto temp = ko::View<Real*>("permute_temp", val_sort_view.size());
    ko::deep_copy(temp, val_sort_view);
    ArborX::Details::applyPermutation(ExecutionSpace{}, permutation, temp,
                                      val_sort_view);
  }

  ko::deep_copy(hcol_tree, mass_trans_tree.spmat_views.col);
  ko::deep_copy(hrow_tree, mass_trans_tree.spmat_views.row);
  ko::deep_copy(hrowmap_tree, mass_trans_tree.spmat_views.rowmap);
  ko::deep_copy(hval_tree, mass_trans_tree.spmat_views.val);

  for (int i = 0; i < xlen + 1; ++i) {
    assert(hrow(i) - hrow_tree(i) == 0);
  }
  fmt::print(stdout, "Rowmaps match!\n");

  int nnz = hcol_tree.size();
  for (int i = 0; i < nnz; ++i) {
    assert(hcol(i) - hcol_tree(i) == 0);
  }
  fmt::print(stdout, "Columns match!\n");
  for (int i = 0; i < nnz; ++i) {
    assert(hval(i) - hval_tree(i) < 1.0e-13);
  }
  fmt::print(stdout, "Values match!\n");

  fmt::print(
      stdout,
      "SUCCESS: brute force and tree search produce identical results.\n");

  return 0;
}
