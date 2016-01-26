
#include <unistd.h>
#include <cstddef>
#include <thread>
#include <iostream>
#include <signal.h>

#include <reflective/reflective.hpp>
#include <tools/logger/logger.hpp>
#include <tools/backtrace.hpp>

void fnc()
{
  neam::cr::out.info() << LOGGER_INFO << __PRETTY_FUNCTION__ << ": In the thread !" << std::endl;
  volatile int *ptr = 0;
  *ptr = 5;
}

int main(int /*argc*/, char **/*argv*/)
{
  struct sigaction sct;
  sct.sa_handler = [](int) { neam::cr::print_callstack(); };
  sct.sa_flags = 0;
  sigemptyset(&sct.sa_mask);

  // register sig handlers
  sigaction(SIGSEGV, &sct, NULL);
  sigaction(SIGABRT, &sct, NULL);

  // create a thread
  for (int i = 0; i < 15; ++i)
  {
    std::thread th(fnc);
    th.detach();
  }

  // abort the prog
  abort();
}
