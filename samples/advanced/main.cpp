
#include <csignal>

#include <reflective/reflective.hpp> // The reflective header
#include <reflective/signal.hpp>     // We want reflective to also monitor signals
#include <tools/logger/logger.hpp>   // Just to set the logger in debug mode

#include "my_class.hpp"

int main(int /*argc*/, char **/*argv*/)
{
  // Set the reflective configuration
  neam::r::conf::monitor_global_time = true;
  neam::r::conf::monitor_self_time = true;
  neam::r::conf::out_file = "./advanced.nr";
  neam::r::conf::print_fails_to_stdout = true;

  // Install some signal handlers.
  neam::r::install_default_signal_handler({SIGABRT, SIGSEGV, SIGFPE, SIGINT});

  // Auto stash.
  neam::r::auto_stash_current_data(N_DEFAULT_STASH_NAME);

  // make the logger verbose
  neam::cr::out.log_level = neam::cr::stream_logger::verbosity_level::debug;

  // start monitoring main()
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(main));

  // this demonstrates how reflective will create the callgraph of that
  sample::my_class instance0;

  {
    sample::my_class instance1 = instance0;

    instance1.push(true);
    instance1.my_function();

    instance0 = instance1;
  }

  instance0.may_throw(5); // this would terminate the program with SIGABRT (on linux at least)

  // If either this scope is exited or something bad happens (signal is caught,
  // exception throw but not caugh, a protection/segmentation fault, ...)
  // reflective will save the data to the file (unless this is disabled in the
  // configuration).
  //
  // With that file (in this sample, ./advanced.nr) you will be able to create a dot graph
  // using the following command:
  //  callgraph2dot ./advanced.nr -o out.gv
  // (or the following one, with branch pruning enabled):
  //  callgraph2dot ./advanced.nr -r -o out.gv
  // If you want a png image from this DOT file:
  //  dot -Kdot -Gscale=5 -Tpng out.gv >out.png
  //
  // You can also use the reflective shell:
  //  reflective-shell -l advanced.nr
  //
  // More information about the shell can be found here: https://github.com/tim42/reflective/blob/master/tools/shell/README.md
  return 0;
}
