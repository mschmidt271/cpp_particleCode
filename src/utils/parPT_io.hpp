#ifndef PARPT_IO_HPP
#define PARPT_IO_HPP

#include "Kokkos_Core.hpp"
#include "containers.hpp"
#include "type_defs.hpp"
#include "version_info.hpp"

namespace particles {

// class for writing particle output
class ParticleIO {
 private:
  FILE* outfile;

 public:
  ParticleIO(std::string f);
  void write(const ko::View<Real*>& X, const ko::View<Real*>& mass,
             const Params& pars, int i);
};

}  // namespace particles

#endif
