
#include <csignal>

#include <reflective/reflective.hpp> // The reflective header
#include <reflective/signal.hpp>     // We want reflective to also monitor signals
#include <tools/logger/logger.hpp>   // Just to set the logger in debug mode
#include <tools/debug/assert.hpp>    // For neam::debug
#include <tools/debug/unix_errors.hpp>    // For neam::debug


using unix_error = neam::debug::on_error<neam::debug::errors::unix_errors>;

int main(int /*argc*/, char **/*argv*/)
{
  // Set the reflective configuration
  neam::r::conf::monitor_global_time = true;
  neam::r::conf::monitor_self_time = true;
  neam::r::conf::out_file = "./ndebug-integration.nr";

  // Install some signal handlers.
  neam::r::install_default_signal_handler({SIGABRT, SIGSEGV, SIGFPE, SIGINT});

  // Auto stash.
  neam::r::auto_stash_current_data(N_DEFAULT_STASH_NAME);

  // make the logger verbose
  neam::cr::out.log_level = neam::cr::stream_logger::verbosity_level::debug;


  // We finally start monitoring main()
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(main));

  // This assert will go just fine
  unix_error::n_assert(nullptr == (void *)0, "C++ is broken");

  // this one will fail and generate an error entry in main()
  // also, main will die from uncaugh exception
  unix_error::n_assert(nullptr != (void *)0, "C++ isn't broken");


  // If either this scope is exited or something bad happens (signal is caught,
  // exception throw but not caugh, a protection/segmentation fault, ...)
  // reflective will save the data to the file (unless this is disabled in the
  // configuration).
  //
  // With that file (in this sample, ./ndebug-integration.nr) you will be able to create a dot graph
  // using the following command:
  //  callgraph2dot ./ndebug-integration.nr -o out.gv
  // If you want a png image from this DOT file:
  //  dot -Kdot -Gscale=5 -Tpng out.gv >out.png
  //
  // You can also use the reflective shell:
  //  reflective-shell -l basic.nr
  //
  // More information about the shell can be found here: https://github.com/tim42/reflective/blob/master/tools/shell/README.md
  return 0;
}
