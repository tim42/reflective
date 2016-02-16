
#include <unistd.h>
#include <csignal>
#include <cstddef>
#include <iostream>

#define N_R_XBUILD_COMPAT // I want a file that works across multiple builds / compilers

#include <reflective/reflective.hpp>
#include <reflective/signal.hpp>
#include <tools/logger/logger.hpp>
#include "introspect_helper.hpp"

class s
{
  public:
    void d()
    {
      // create a silly measure_point
      neam::r::measure_point mp1("function_call-constructor-time");

      neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(s::d));

      // stop the measure_point
      mp1.stop();

      // another silly measure_point
      neam::r::measure_point mp2("half-time", neam::r::defer_start);

      volatile size_t max = 100000;
      volatile size_t r = 0;
      for (volatile size_t k = 0; k < max; ++k)
      {
        if (k == (max / 2))
          mp2.start();
        r += rand();
      }

      if ((rand() ^ r) % 50 < 3)
        self_call.fail(neam::r::lazy_programmer_reason(N_REASON_INFO, "[can't touch this]"));
    }

    void f()
    {
      neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(s::f));

      // randomly make the prog segfault-
      if (rand() % 100000 < 10)
      {
        neam::cr::out.warning() << LOGGER_INFO << "random crash spotted !" << std::endl;
        if (rand() % 2)
        {
          volatile int *ptr = nullptr;
          *ptr = 0;
        }
        else
        {
          abort();
        }
      }
    }
};

void fnc()
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(fnc));

  for (volatile size_t k = 0; k < 100000; ++k);

  s lol;

  self_call.if_wont_fail("s::d", &s::d)
      .call(&lol)
      .otherwise([&]()
      {
        lol.f();
      });

  lol.f();
}


int main(int /*argc*/, char **/*argv*/)
{
  // set the reflective configuration
  neam::r::conf::monitor_global_time = true;
  neam::r::conf::monitor_self_time = true;
  neam::r::conf::out_file = "./sample.nr";

  // some signal handlers
  neam::r::install_default_signal_handler({SIGABRT, SIGSEGV, SIGFPE, SIGINT});

  // auto stash
  neam::r::auto_stash_current_data(N_DEFAULT_STASH_NAME);

  // make the logger verbose
  neam::cr::out.log_level = neam::cr::stream_logger::verbosity_level::debug;

  // We finally start main
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(main));

  // We do some introspection & pretty printing
  sample::introspect_function("s::d");
  sample::introspect_function("s::f");
  sample::introspect_function("fnc");
  sample::introspect_function("main");

  // print all stashes:
  {
    std::cout << "\nstashes:\n";
    size_t cidx = 0;
    const size_t active_idx = neam::r::get_active_stash_index();
    for (const std::string & n : neam::r::get_stashes_name())
    {
      if (cidx++ == active_idx)
        std::cout << "  * " << n << '\n';
      else
        std::cout << "    " << n << '\n';
    }
    std::cout << std::endl;
  }

  s lol;

  srand((long)malloc(0));

  lol.d();

  for (volatile size_t j = 0; j < 10000; ++j)
    fnc();
  return 0;
}
