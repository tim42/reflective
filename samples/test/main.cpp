
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
      neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(s::d));
      for (volatile size_t k = 0; k < 100000; ++k);

      if (rand() % 50 < 3)
        self_call.fail(neam::r::lazy_programmer_reason(N_REASON_INFO, "can't touch this"));
    }

    void f()
    {
      neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(s::f));

      // randomly make the prog segfault
      if (rand() % 100000 < 10)
      {
        neam::cr::out.warning() << LOGGER_INFO << "random segfault spotted !" << std::endl;
        volatile int *ptr = nullptr;
        *ptr = 0;
      }
    }
};

void fnc()
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(fnc));

  for (volatile size_t k = 0; k < 100000; ++k);

  s lol;

  self_call.if_wont_fail(N_FUNCTION_INFO(s::d))
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

  // make the logger verbose
  neam::cr::out.log_level = neam::cr::stream_logger::verbosity_level::debug;

  // We finally start main
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(main));

  // We do some introspection & pretty printing
  sample::introspect_function("s::d");
  sample::introspect_function("s::f");
  sample::introspect_function("fnc");
  sample::introspect_function("main");

  s lol;

  srand((long)malloc(0));

  lol.d();

  for (volatile size_t j = 0; j < 10000; ++j)
    fnc();
  return 0;
}
