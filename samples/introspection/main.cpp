
#include <unistd.h>
#include <csignal>

#include <reflective/reflective.hpp> // The reflective header
#include <reflective/signal.hpp>     // We want reflective to also monitor signals
#include <tools/logger/logger.hpp>   // Just to set the logger in debug mode
#include <tools/debug/assert.hpp>    // For neam::debug
#include <tools/debug/unix_errors.hpp>    // For neam::debug

#include "my_class.hpp"

using unix_error = neam::debug::on_error<neam::debug::errors::unix_errors>;

int main(int /*argc*/, char **/*argv*/)
{
  // Set the reflective configuration
  neam::r::conf::monitor_global_time = true;
  neam::r::conf::monitor_self_time = true;
  neam::r::conf::out_file = "./introspection.nr";

  // Install some signal handlers.
  neam::r::install_default_signal_handler({SIGABRT, SIGSEGV, SIGFPE, SIGINT});

  // Auto stash.
  neam::r::auto_stash_current_data(N_DEFAULT_STASH_NAME);

  // make the logger a bit less verbose
  neam::cr::out.log_level = neam::cr::stream_logger::verbosity_level::info;


  // We finally start monitoring main()
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(main));

  // create some data:
  sample::my_class instance0;
  {
    sample::my_class instance1 = instance0;

    instance1.push(true);
    instance1.my_function();

    instance0 = instance1;
    instance0.push(true);
  }

  // Now, get that data:
  // As with function_call, there is a N_FUNCTION() and a N_NAME()
  // But you can also create an introspect object from a dynamic string
  // (see below). But N_NAME() isn't something that you should use.
  //
  // Here we access the global information about that function. You can also access
  // contextual information. (the context is one of the caller, in a callgraph)
  neam::r::introspect introspect(N_FUNCTION(sample::my_class::may_throw));

  neam::cr::out.log() << LOGGER_INFO << introspect.get_pretty_name() << ": " << int(introspect.get_failure_ratio() * 100) << "% chance of failure" << std::endl;
  neam::cr::out.log() << LOGGER_INFO << introspect.get_pretty_name() << ": " << introspect.get_average_duration() << "s average duration" << std::endl;

  // You can also use the name as a dynamic string (this is a bit slower, but it works the same)
  introspect = neam::r::introspect("sample::my_class::get_top_value");
  neam::cr::out.log() << LOGGER_INFO << introspect.get_pretty_name() << ": " << int(introspect.get_failure_ratio() * 100) << "% chance of failure" << std::endl;
  neam::cr::out.log() << LOGGER_INFO << introspect.get_pretty_name() << ": " << introspect.get_average_duration() << "s average duration" << std::endl;

  // You can also have conditional calls based on the success rate of the function
  // You have to specify the name of the function and its pointer.
  self_call.if_wont_fail("sample::my_class::may_throw", &sample::my_class::may_throw)
  .call(&instance0, 0)   // then, call(...) will effectively call the function, if the conditions are OK
  .then([]() // then() is called if the function has been called
  {
    neam::cr::out.log() << LOGGER_INFO << "Called sample::my_class::may_throw() with success !!" << std::endl;
  })
  .otherwise([]() // otherwise() is called if the function hasn't been called
  {
    neam::cr::out.log() << LOGGER_INFO << "Didn't called sample::my_class::may_throw()" << std::endl;
  });

  // If either this scope is exited or something bad happens (signal is caught,
  // exception throw but not caugh, a protection/segmentation fault, ...)
  // reflective will save the data to the file (unless this is disabled in the
  // configuration).
  //
  // With that file (in this sample, ./introspection.nr) you will be able to create a dot graph
  // using the following command:
  //  callgraph2dot ./introspection.nr -o out.gv
  // If you want a png image from this DOT file:
  //  dot -Kdot -Gscale=5 -Tpng out.gv >out.png
  //
  // You can also use the reflective shell:
  //  reflective-shell -l basic.nr
  //
  // More information about the shell can be found here: https://github.com/tim42/reflective/blob/master/tools/shell/README.md
  return 0;
}
