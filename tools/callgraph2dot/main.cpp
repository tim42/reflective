
#include <cstddef>
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

#define N_R_XBUILD_COMPAT // I want a file that works across multiple builds / compilers

#include <reflective/reflective.hpp>
#include <reflective/signal.hpp>
#include <tools/logger/logger.hpp>

#include "dot_gen.hpp"

int main(int argc, char **argv)
{
  // setup the conf
  neam::r::conf::disable_auto_save = true;
  neam::r::conf::out_file = "";

  size_t min_call_count;
  float min_gbl_time;
  std::string input_file;
  std::string output_file;
  bool weight_with_global_time;
  bool weight_with_callcount;

  // parse cmdline arguments
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print this message and exit")
    ("version,v", "print version and exit")

    ("input,i", boost::program_options::value<std::string>(&input_file), "the input file to use (must be a reflective output file)")
    ("output,o", boost::program_options::value<std::string>(&output_file)->default_value("-"), "the output file ( '-' for stdout)")

    ("weight-with-global-time,G", boost::program_options::value<bool>(&weight_with_global_time)->default_value(true), "account the global-time in the weight of an edge (its 'boldness')")
    ("weight-with-callcount,C", boost::program_options::value<bool>(&weight_with_callcount)->default_value(true), "account the call count in the weight of an edge (its 'boldness')")

    ("no-failure-reason,n", "the output graph will NOT include the failures report of the program")
    ("no-failure-reason-full-trace", "the output graph will NOT include the full path to the error")

    ("remove-insignificant-branch,r", "remove from the callgraph all insignificant calls (useful for performance analysis)")

    ("min-call-count,c", boost::program_options::value<size_t>(&min_call_count)->default_value(10), "control the minimum call count for a function to be considered significant")
    ("min-global-time,g", boost::program_options::value<float>(&min_gbl_time)->default_value(0.001), "control the minimum global time for a function to be considered significant")
  ;

  boost::program_options::positional_options_description pod;
  pod.add("input", 1);

  boost::program_options::variables_map vm;
  try
  {
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(pod).run(), vm);
    boost::program_options::notify(vm);
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 0;
  }
  catch (...)
  {
    return 0;
  }

  // handle options
  if (vm.count("help"))
  {
    std::cout << "Usage: " << argv[0] << " [options] input-file" << std::endl;
    std::cout << desc << std::endl;
    return 0;
  }
  if (vm.count("version"))
  {
    std::cout << "reflective callgraph to dot converter, using reflective " << _PROJ_VERSION_MAJOR << '.' << _PROJ_VERSION_MINOR << '.' << _PROJ_VERSION_SUPERMINOR << std::endl;;
    return 0;
  }

  // load the file
  neam::r::load_data_from_disk(input_file);

  // create the callgraph_to_dot object
  neam::r::callgraph_to_dot ctd;

  ctd.set_weight_properties(weight_with_global_time, weight_with_callcount);

  if (vm.count("remove-insignificant-branch"))
    ctd.remove_insignificant_branch(true, min_call_count, min_gbl_time);
  else
    ctd.remove_insignificant_branch(false);

  if (vm.count("no-failure-reason"))
    ctd.include_failure_reasons(false);
  else if (vm.count("no-failure-reason-full-trace"))
    ctd.include_failure_reasons(true, false);
  else
    ctd.include_failure_reasons(true, true);

  // output the dot file
  if (output_file != "-")
  {
    std::ofstream f(output_file);
    ctd.write_to_stream(f);
  }
  else
    ctd.write_to_stream(std::cout);

  return 0;
}
