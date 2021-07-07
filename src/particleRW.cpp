#include "utilities.hpp"
#include <iostream>
#include <vector>
// this will give the install directory location (so far)
#include "ParticleRW_config.h"

using namespace particles;

int main(int argc, char* argv[]) {

  RandyNorm rN_std(0.0, 1.0);

  // create the params object and get the values from the input file
  Params params;
  params.set_values(std::string(installPrefix) +
                    std::string("/data/particleParams.yml"));
  params.print_summary();

  // calculate number of steps to take
  params.nSteps = ceil(params.maxT / params.dt);

  // create particle vector and assign the position of all particle to X0
  std::vector<double> pVec(params.Np, params.X0);

  // create writeParticles object for writing information to file
  particleIO particleIO(std::string(installPrefix) +
                        std::string("/data/") +
                        std::string(params.pFile));

  // begin time stepping
  for (int tStep = 1; tStep <= params.nSteps; ++tStep)
  {
    std::cout << "time step = " << tStep << "\n";
    random_walk(pVec, params.D, params.dt, rN_std);
    particleIO.write(pVec, params, tStep);
  }

  return 0;
  }
