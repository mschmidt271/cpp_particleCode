#include "random.hpp"

namespace particles {

RandPoolType init_random_seed() {
  RandPoolType rand_pool = RandPoolType(5374857);
  return rand_pool;
}

RandPoolType init_random_seed(const int& seed) {
  RandPoolType rand_pool = RandPoolType(seed);
  return rand_pool;
}

}  // namespace particles
