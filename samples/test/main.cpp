
#include <csignal>
#include <cstddef>
#include <iostream>

#define N_R_XBUILD_COMPAT // I want a file that works across multiple builds / compilers

#include <reflective/reflective.hpp>
#include <reflective/signal.hpp>
#include <tools/logger/logger.hpp>
#include <tools/debug/assert.hpp>  // for the integration with neam::debug::*
#include <tools/debug/unix_errors.hpp>  // for the integration with neam::debug::*
#include "introspect_helper.hpp"

neam::r::sequence *seq = nullptr;

#define PUSH_SEQ(x, y)    if (seq) seq->add_entry({N_SEQUENCE_ENTRY_INFO, x, y})
#define PUSH_CALL         PUSH_SEQ("function call", self_call.get_introspect().get_name())

// A correct implementation of a logger w/ reflective is to both have a report and a sequence
static const neam::r::reason log_reason = neam::r::reason {"log"};
#define LOG(x)            do {self_call.report("log", log_reason(N_REASON_INFO, x)); PUSH_SEQ("log", x); } while (0)

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

      PUSH_CALL;

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
      else
        self_call.report("not an error", neam::r::lazy_programmer_reason(N_REASON_INFO, "[YAY]"));
    }

    void f()
    {
      neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(s::f));
      PUSH_CALL;

      // randomly make the prog segfault-
      if (rand() % 100000 < 10000)
      {
        LOG("RANDOM CRASH SPOTTED");
        neam::cr::out.warning() << LOGGER_INFO << "random crash spotted !" << std::endl;
        if (rand() % 2 == 1)
        {
          LOG("WE GONNA HAVE A SEGFAULT !");
          volatile int *ptr = nullptr;
          *ptr = 0;
        }
        else
        {
          LOG("WE GONNA HAVE AN EXCEPTION !");
          std::vector<int> vct;
          vct.at(50) = 1;
        }
      }
    }
};

static bool xfirst = false;

void fnc()
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(fnc));
  PUSH_CALL;

  for (volatile size_t k = 0; k < 100000; ++k);

  s lol;


  lol.d();

  lol.f();

  if (!xfirst)
  {
    xfirst = true;
    LOG("FAILED TEST ?!");
    try
    {
      neam::debug::on_error<neam::debug::errors::unix_errors>::n_assert(false == true, "wow, I don't understand what failed here...");
    }
    catch (...) {}
  }
}


int main(int /*argc*/, char **/*argv*/)
{
  // set the reflective configuration
  neam::r::conf::monitor_global_time = true;
  neam::r::conf::monitor_self_time = true;
  neam::r::conf::out_file = "./sample.nr";
  neam::r::conf::watch_uncaught_exceptions = true;

  // some signal handlers
  neam::r::install_default_signal_handler({SIGABRT, SIGSEGV, SIGFPE, SIGINT});

  // auto stash
  neam::r::auto_stash_current_data(N_DEFAULT_STASH_NAME);

  // make the logger verbose
  neam::cr::out.log_level = neam::cr::stream_logger::verbosity_level::debug;

  // We finally start main
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(main));

  seq = &self_call.create_sequence("call sequence");

  PUSH_CALL;

  // We do some introspection & pretty printing
  sample::introspect_function("s::d");
  sample::introspect_function("s::f");
  sample::introspect_function("fnc");
  sample::introspect_function("main");

  // print all stashes:
  {
    LOG("PRINT STASHES");
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

  LOG("RUNNING A BIT");

  s lol;

  srand((long)malloc(0));

  lol.d();

  try
  {
    for (volatile size_t j = 0; j < 10000; ++j)
    {
      fnc();
    }
  }
  catch (...)
  {
    LOG("GOT AN EXCEPTION, IGNORED IT");
  }
  return 0;
}
