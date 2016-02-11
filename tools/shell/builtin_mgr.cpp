
#include <list>
#include <iomanip>
#include "builtin_mgr.hpp"

neam::r::shell::builtin_mgr::builtin_mgr()
{
}

bool neam::r::shell::builtin_mgr::register_builtin(const std::string &name, neam::r::shell::builtin &blt)
{
  return builtin_list.emplace(name, blt).second;
}

void neam::r::shell::builtin_mgr::unregister_builtin(neam::r::shell::builtin &blt)
{
  std::list<std::string> names;
  for (const auto &it : builtin_list)
  {
    if (&it.second == &blt)
      names.push_back(it.first);
  }

  for (const auto &it : names)
    unregister_builtin(it);
}

void neam::r::shell::builtin_mgr::unregister_builtin(const std::string &name)
{
  builtin_list.erase(name);
}

bool neam::r::shell::builtin_mgr::has_builtin(const std::string &name) const
{
  return builtin_list.count(name);
}

bool neam::r::shell::builtin_mgr::call(const std::string &name, neam::r::shell::variable_stack &stack, neam::r::shell::stream_pack &streamp, int &ret) const
{
  auto find_it = builtin_list.find(name);
  if (find_it == builtin_list.end())
    return false;

  builtin &blt = find_it->second;

  try
  {
    ret = blt.call(name, stack, streamp);
  }
  catch (std::exception &e) // avoid being killed by exception
  {
    streamp[stream::stderr] << "error in builtin " << name << ": caught exception: " << e.what() << std::endl;
    ret = 1; // fail
  }
  catch (...) // avoid being killed by exception
  {
    streamp[stream::stderr] << "error in builtin " << name << ": caught unknown exception" << std::endl;
    ret = 1; // fail
  }
  return true;
}

bool neam::r::shell::builtin_mgr::print_help(const std::string &name, neam::r::shell::stream_pack &streamp) const
{
  auto find_it = builtin_list.find(name);
  if (find_it == builtin_list.end())
    return false;

  builtin &blt = find_it->second;

  try
  {
    blt.help(name, streamp);
  }
  catch (std::exception &e) // avoid being killed by exception
  {
    streamp[stream::stderr] << "error in builtin " << name << ": caught exception: " << e.what() << std::endl;
    return false;
  }
  catch (...) // avoid being killed by exception
  {
    streamp[stream::stderr] << "error in builtin " << name << ": caught unknown exception" << std::endl;
    return false;
  }
  return true;
}

void neam::r::shell::builtin_mgr::print_builtin_summary(neam::r::shell::stream_pack &streamp) const
{
  if (!streamp.has(stream::stdout))
  {
    streamp[stream::stderr] << "error: no stdout stream" << std::endl;
    return;
  }

  for (const std::pair<const std::string, builtin &> &it : builtin_list)
  {
    std::string cmd = it.first + " " + it.second.get_usage_message();
    streamp[stream::stdout] << std::setw(35) << std::left << cmd << "    " << it.second.get_help_message_banner() << std::endl;
  }
}
