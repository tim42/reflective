
#include "builtin.hpp"
#include "flow_control.hpp"

neam::r::shell::builtin::builtin(std::function<callback_function_sig> _callback, const std::string &message_banner)
 : help_message_banner(message_banner), help_usage_message("[options]"), callback(_callback)
{
}

neam::r::shell::builtin::builtin(std::function< callback_function_sig > _callback, const std::string &message_banner, const std::string &usage_message)
 : help_message_banner(message_banner), help_usage_message(usage_message), callback(_callback)
{
}


void neam::r::shell::builtin::help(const std::string &name, neam::r::shell::stream_pack &streamp)
{
  if (!streamp.has(stream::stdout))
  {
    streamp[stream::stderr] << "builtin " << name << ": no stdout stream" << std::endl;
    return;
  }
  if (!help_message_banner.empty())
    streamp[stream::stdout] << name << ": " << help_message_banner << std::endl;
  streamp[stream::stdout] << "Usage: " << name << " " << help_usage_message << "\n";
  if (!skip_program_options)
    streamp[stream::stdout] << desc << std::endl;
}

int neam::r::shell::builtin::call(const std::string &name, neam::r::shell::variable_stack &stack, neam::r::shell::stream_pack &streamp)
{
  const std::deque<std::string> &res = stack.get_argument_array();
  if (!res.size()) // should never happen
  {
    streamp[stream::stderr] << "error in builtin " << name << ": bad builtin call (buggy shell)" << std::endl;
    return 1;
  }

  std::vector<std::string> args(res.begin() + 1, res.end());
  if (!skip_program_options)
  {
    try
    {
      if (no_unknown_params)
        boost::program_options::store(boost::program_options::command_line_parser(args).options(desc).positional(pod).run(), vm);
      else
        boost::program_options::store(boost::program_options::command_line_parser(args).options(desc).positional(pod).allow_unregistered().run(), vm);
      boost::program_options::notify(vm);
    }
    catch (std::exception &e)
    {
      streamp[stream::stderr] << name << ": " << e.what() << std::endl;
      vm.clear();
      return 1;
    }
  }

  int ret;
  try
  {
    ret = callback(name, stack, streamp, vm);
  }
  catch (flow_control &e) { throw; }
  catch (std::exception &e)
  {
    streamp[stream::stderr] << "error in builtin " << name << ": caught exception: " << e.what() << std::endl;
    ret = 1; // fail
  }
  catch (...)
  {
    streamp[stream::stderr] << "error in builtin " << name << ": caught unknown exception" << std::endl;
    ret = 1; // fail
  }

  vm.clear(); // clear the map (do not hold memory for nothing)
  return ret;
}
