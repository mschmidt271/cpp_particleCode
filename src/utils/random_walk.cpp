#include "random_walk.hpp"

namespace particles {

// parallelized random walk function that calls the RandomWalk functor
void random_walk(ko::View<Real*>& X, const Params& params,
                 RandPoolType& rand_pool) {
  ko::Profiling::pushRegion("RW_fxn_pre-if");
  if (params.pctRW > 0.0) {
    ko::Profiling::pushRegion("RW_fxn");
    ko::parallel_for(params.Np,
                     RandomWalk<RandPoolType>(
                         X, rand_pool, 0.0,
                         sqrt(2.0 * params.pctRW * params.D * params.dt)));
    ko::Profiling::popRegion();
    ko::Profiling::popRegion();
  }
}

}  // namespace particles
