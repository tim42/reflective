
#include <csignal>
#include "tools/logger/logger.hpp"
#include "tools/backtrace.hpp"
#include "signal.hpp"
#include "storage.hpp"
#include "config.hpp"
#include "function_call.hpp"

void neam::r::on_signal(int sig)
{
  // report
  function_call *fc = function_call::get_active_function_call();
  if (fc)
  {
    switch (sig)
    {
      case SIGSEGV:
        fc->fail(segfault_reason(N_REASON_INFO), "segmentation fault signal");
        break;
      case SIGABRT:
        fc->fail(abort_reason(N_REASON_INFO), "abort signal");
        break;
      case SIGFPE:
        fc->fail(floating_point_exception_reason(N_REASON_INFO), "floating point exception signal (dividing by 0 is bad)");
        break;
      case SIGILL:
        fc->fail(illegal_instruction_reason(N_REASON_INFO), "illegal instruction");
        break;
      case SIGINT:
        fc->fail(illegal_instruction_reason(N_REASON_INFO), "keyboard interrupt (well, that's not an error)");
        break;
      default:
        fc->fail(unknown_signal_reason(N_REASON_INFO), "not a SIGSEGV, SIGABRT, SIGFPE, SIGINT, nor a SIGILL");
        break;
    }
  }
  else
  {
    neam::cr::out.critical() << LOGGER_INFO << "could not report that I just died from a nasty signal, so here is a backtrace" << std::endl;
    neam::cr::print_callstack(100, 2);
  }

  // save
  sync_data_to_disk(conf::out_file);
}

void neam::r::install_default_signal_handler(std::initializer_list<int> signals)
{
#ifdef _WIN32
  auto handler = [](int sig)
  {
    // report & save
    on_signal(sig);

    // die...
    signal(sig, SIG_DFL);
    raise(sig);
  };

  for (int sig : signals)
    signal(sig, handler);
#else
  struct sigaction sct;
  sct.sa_handler = [](int sig)
  {
    // report & save
    on_signal(sig);

    // die...
    raise(sig);
  };

  sct.sa_flags = SA_NODEFER | SA_RESETHAND;
  sigemptyset(&sct.sa_mask);

  // register sig handlers
  for (int sig : signals)
    sigaction(sig, &sct, nullptr);
#endif
}

