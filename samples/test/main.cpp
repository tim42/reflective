
#include <unistd.h>
#include <cstddef>
#include <thread>
#include <iostream>
#include <signal.h>

#define N_R_XBUILD_COMPAT // I want a file that works across multiple builds / compilers

#include <reflective/reflective.hpp>
#include <tools/logger/logger.hpp>
#include <tools/backtrace.hpp>

class s
{
  public:
    void d()
    {
      neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(s::d));
      for (volatile size_t k = 0; k < 100000; ++k);
    }
};

void fnc()
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(fnc));
  self_call.monitor_global_time();
  self_call.monitor_self_time();

  for (volatile size_t k = 0; k < 100000; ++k);

  s lol;

  self_call.if_wont_fail(N_FUNCTION_INFO(s::d)).call(&lol);

  if (self_call.get_failure_ratio() < 0.5) // gently fail when needed in order to maintain a 50% ratio
    self_call.fail(neam::r::lazy_programmer_reason(N_REASON_INFO, "can't touch it"));
}

int main(int /*argc*/, char **/*argv*/)
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(main));
  self_call.monitor_global_time();
  self_call.monitor_self_time();

//   struct sigaction sct;
//   sct.sa_handler = [](int) { neam::cr::print_callstack(); };
//   sct.sa_flags = 0;
//   sigemptyset(&sct.sa_mask);
// 
//   // register sig handlers
//   sigaction(SIGSEGV, &sct, NULL);
//   sigaction(SIGABRT, &sct, NULL);
// 
//   // create a thread
//   for (int i = 0; i < 2; ++i)
//   {
//     std::thread th(fnc);
//     th.detach();
//   }

  s lol;
  lol.d();
  for (volatile size_t j = 0; j < 10000; ++j)
    fnc();

  // abort the prog
//   abort();
}
