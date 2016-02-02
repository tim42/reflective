
#include <cstddef>
#include <fstream>

#define N_R_XBUILD_COMPAT // I want a file that works across multiple builds / compilers

#include <reflective/reflective.hpp>
#include <reflective/signal.hpp>
#include <tools/logger/logger.hpp>

#include "dot_gen.hpp"

int main(int argc, char **argv)
{
  // setup the conf
  neam::r::conf::disable_auto_save = true;
  if (argc != 1)
    neam::r::conf::out_file = argv[1];

  // load the file
  neam::r::load_data_from_disk(neam::r::conf::out_file);


  // output the dot file
  std::ofstream f("out.gv");
  neam::r::callgraph_to_dot ctd;
  ctd.write_to_stream(f);

  return 0;
}
