
#include <unistd.h>
#include <csignal>

#include <reflective/reflective.hpp> // The reflective header
#include <reflective/signal.hpp>     // We want reflective to also monitor signals
#include <tools/logger/logger.hpp>   // Just to set the logger in debug mode


int main(int /*argc*/, char **/*argv*/)
{
  // Set the reflective configuration
  // You can set/change the configuration at any time you want, reflective will
  // read those value each time an operation is performed
  neam::r::conf::monitor_global_time = true;
  neam::r::conf::monitor_self_time = true;
  neam::r::conf::out_file = "./basic.nr";
  neam::r::conf::print_fails_to_stdout = true;

  // Install some signal handlers.
  // When a signal is caught, it will generate a failure in the current function
  // then trigger a save and pass the execution to the default handler (that will
  // most likely kill the program)
  neam::r::install_default_signal_handler({SIGABRT, SIGSEGV, SIGFPE, SIGINT});

  // Auto stash.
  // Stashes are saves of reflective data from previous versions / launches / ...
  // auto_stash_current_data will stash the old data if it detects the current build
  // is not the same that the one that created the reflective data. (current build
  // of the program, not of reflective).
  // This is usefull is you want to have some history of how previous version performed
  // compared to this one.
  //
  // Stashes functions must be called while no function_call are active on any thread
  // (a good thing to do is to put it right at the begining of you main() function, like here)
  neam::r::auto_stash_current_data(N_DEFAULT_STASH_NAME);

  // make the logger verbose
  // This is not a refective configuration, but a logger configuration.
  neam::cr::out.log_level = neam::cr::stream_logger::verbosity_level::debug;


  // We finally start monitoring main()
  // The N_PRETTY_FUNCTION_INFO(function_name_here) macro will
  // populate from you important reflective information. If you can't
  // have the function name (because of overload, it's a constructor, a lambda, ...)
  // You can still use N_PRETTY_NAME_INFO("function_name_here"). (more on that below).
  //
  // If your function is member of a class that is in a namespace, please specify
  // the whole path (namespace1::namepspace2::class::function), as this is the identifier
  // and it should be unique. (and the shell tool uses namespaces as folders).
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(main));

  // We have a lambda. How do I monitore that ???
  auto lbd = [](double d, int j) -> double
  {
    // When monitoring lambda, constructors, or overloaded functions
    // you'll have to use N_PRETTY_NAME_INFO instead of N_PRETTY_FUNCTION_INFO.
    // N_PRETTY_NAME_INFO differs from N_PRETTY_FUNCTION_INFO in that you will
    // have to set yourself the "identifier-name" of the function.
    // Beware, identifiers must be unique.
    //
    // If your function is member of a class that is in a namespace, please specify
    // the whole path ("namespace1::namepspace2::class::function(int, int)"), as this is the identifier
    // and it should be unique. (and the shell tool uses namespaces as folders).
    neam::r::function_call self_call(N_PRETTY_NAME_INFO("main()::lbd#1"));

    // dumb loop, volatile to prevent optimisations
    for (volatile int i = 0; i < 3000 * 1000 * i * 5; ++i)
    {
      d *= d - i + j;
      if (d < 0)
        d = -1.;
    }

    // dumb failure condition for learning purpose only
    if (d < 10)
    {
      // fail(reason) is used to indicate a failure in the current function.
      // The reason parameter must be a neam::r::reason instance. Multiple predefined reason types
      // are included with reflective, but nothing stops you from defining some more that fits your needs
      // and describe best the errors you program may encounter.
      // N_REASON_INFO simply expand to the current file and the current line and next is the optional description
      // of the error.
      // Please note that this won't exit the program nor return from the function.
      // This only states that the current function has encoutered something bad.
      self_call.fail(neam::r::should_not_happen_reason(N_REASON_INFO, "d is lower than 10 at the end of the computation"));
    }

    return d;
  };

  // simulate some load
  for (int i = 0; i < 100; ++i)
  {
    lbd(i, i * 10);
  }

  // If either this scope is exited or something bad happens (signal is caught,
  // exception throw but not caugh, a protection/segmentation fault, ...)
  // reflective will save the data to the file (unless this is disabled in the
  // configuration).
  //
  // With that file (in this sample, ./basic.nr) you will be able to create a dot graph
  // using the following command:
  //  callgraph2dot ./basic.nr -o out.gv
  // If you want a png image from this DOT file:
  //  dot -Kdot -Gscale=5 -Tpng out.gv >out.png
  //
  // You can also use the reflective shell:
  //  reflective-shell -l basic.nr
  // and the enter some commands (help will print some help). In this tutorial,
  // you can enter those commands:
  //  mode function
  //  cd main
  //  info
  //  ls
  //  cd "main()::lbd#1"
  //  info -ae
  //
  // `mode function`: the shell has two modes: file and functions. Here we want to walk the callgraph, not
  //  the files, so we want the funciton mode. just after the `mode function`, `ls` would give one result: main.
  //  (if there was multiple threads, we would have multiple entry points).
  //
  // `cd main`: Just as you do in an unix shell, you enter the main function
  // `info`: This command is used to output information about the current function or any arbitrary function
  //  The command `help info` will give more details about its arguments.
  // `ls`: Just as with the ls program, it will print the functions in the current function.
  //  In the shell, in function mode, this mean that those functions are called by the current one
  //
  // `cd "main()::lbd#1"`: Just the same as with `cd main`. The "" tells the shell to ignore
  //  special symbols / spaces.
  //
  // `info -ae`: Print some detailed information about the function.
  //  The -a flag tells info to print all information about the function.
  //  The -e flag tells info to print the errors.
  //  The current function has something particular, the "{self,global} duration progression" should have multiple
  //  entries (depending on your computer). On mine it looks like:
  //   self duration progression:
  //     at 2016-09-25 16:36:57: 69us
  //     at 2016-09-25 16:36:57: 6us
  //     at 2016-09-25 16:36:57: 656ns
  //  This is a realy helpful indicator that track progression of the self duration (time spent in the function only)
  //  In that case, it starts at 656 nano-seconds and grow until reaching 69 micro-seconds. (Currently the only progression
  //  reporting is exponential, but it is planned to have linear progression reports).
  //  This can indicate a bottleneck, and in this particular case, it may become problematic if we increase the number
  //  of iteration of the for-loop in main()
  //
  // More information about the shell can be found here: https://github.com/tim42/reflective/blob/master/tools/shell/README.md
  return 0;
}
