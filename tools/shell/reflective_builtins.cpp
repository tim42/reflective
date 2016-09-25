
#include <iomanip>
#include <vector>
#include "reflective_builtins.hpp"
#include "reflective_manager.hpp"
#include "shell.hpp"

#include <reflective.hpp>

// first is the time, second the unit
std::pair<double, std::string> get_time(double sec_time);

// The reflective manager
static neam::r::manager *rmgr = nullptr;

namespace neam
{
  namespace r
  {
    namespace shell
    {
      static void blt_load(shell &sh);
      static void blt_mode(shell &sh);
      static void blt_cd(shell &sh);
      static void blt_ls(shell &sh);
      static void blt_pwd(shell &sh);
      static void blt_info(shell &sh);
      static void blt_stash(shell &sh);
      static void blt_callgraph2dot(shell &sh);

      static enum class e_dir_mode
      {
        files,
        functions
      } dir_mode = e_dir_mode::files;
    } // namespace shell
  } // namespace r
} // namespace neam

void neam::r::shell::register_reflective_builtins(shell &sh, bool load_only)
{
  if (load_only)
    blt_load(sh);
  else
  {
    blt_mode(sh);
    blt_cd(sh);
    blt_pwd(sh);
    blt_ls(sh);
    blt_info(sh);
    blt_stash(sh);
    blt_callgraph2dot(sh);
  }
}

bool neam::r::shell::load_reflective_file(shell &sh, const std::string &filename)
{
  if (!neam::r::load_data_from_disk(filename))
      return false;

  // create the manager
  rmgr = new neam::r::manager;

  // register the other builtins, if needed
  if (!sh.get_builtin_manager().has_builtin("cd"))
    register_reflective_builtins(sh, false);

  // set the variable:
  sh.get_variable_stack().set_variable("R_ACTIVE_FILE", filename, true);

  return true;
}


/// \brief The load builtin
void neam::r::shell::blt_load(neam::r::shell::shell &sh)
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    std::string filename;
    const std::deque<std::string> &args = vs.get_argument_array();
    if (args.size() != 2)
    {
      streamp[stream::stderr] << name << ": wrong parameter number: " << name << " takes only one parameter" << std::endl;
      return 1;
    }

    filename = args[1];

    if (load_reflective_file(sh, filename))
      return 1;

    // unregister myself
    sh.get_builtin_manager().unregister_builtin(name);

    // die and leak the memory
    return 0;
  }, "load a reflective save file", "reflective-file");
  blt.do_not_use_program_options();
  sh.get_builtin_manager().register_builtin("load", blt);
}

void neam::r::shell::blt_mode(neam::r::shell::shell &sh)
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    const std::deque<std::string> &args = vs.get_argument_array();
    if (args.size() == 1)
    {
      streamp[stream::stdout] << "current mode: " << vs.get_variable("R_MODE") << std::endl;
      return 0;
    }

    std::string mode = args[1];

    if (mode == "function")
      dir_mode = e_dir_mode::functions;
    else if (mode == "file")
      dir_mode = e_dir_mode::files;
    else
    {
      streamp[stream::stderr] << name << ": unknown mode " << mode << std::endl;
      return 1;
    }
    return 0;
  }, "change the directory listing mode: function (callgraph) or file", "[mode: 'function' or 'file']");
  blt.do_not_use_program_options();
  sh.get_builtin_manager().register_builtin("mode", blt);

  // sets the special handler for the R_MODE var
  sh.get_variable_stack().set_variable_special_handler("R_MODE", [&](const std::string &) -> std::string
  {
    switch (dir_mode)
    {
      case e_dir_mode::functions: return "function";
      case e_dir_mode::files: return "file";
    }
    return "unknown";
  });
}


void neam::r::shell::blt_pwd(neam::r::shell::shell &sh)
{
  builtin &blt = *new builtin([&](const std::string &, variable_stack &, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    switch (dir_mode)
    {
      case e_dir_mode::functions:
        streamp[stream::stdout] << rmgr->get_current_introspect_path() << std::endl;
        break;
      case e_dir_mode::files:
        streamp[stream::stdout] << rmgr->get_current_path() << std::endl;
        break;
    }
    return 0;
  }, "print the current working directory", "");
  blt.do_not_use_program_options();
  sh.get_builtin_manager().register_builtin("pwd", blt);

  // sets the special handler for the CWD var
  sh.get_variable_stack().set_variable_special_handler("CWD", [&](const std::string &) -> std::string
  {
    switch (dir_mode)
    {
      case e_dir_mode::functions: return rmgr->get_current_introspect_path();
      case e_dir_mode::files: return rmgr->get_current_introspect_path();
    }
    return "";
  });
}

void neam::r::shell::blt_cd(neam::r::shell::shell &sh)
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    const std::deque<std::string> &args = vs.get_argument_array();
    std::string npath = "/";
    if (args.size() == 2)
      npath = args[1];
    else if (args.size() > 2)
    {
      streamp[stream::stderr] << name << ": too many arguments" << std::endl;
      return 1;
    }

    switch (dir_mode)
    {
      case e_dir_mode::functions:
        if (!rmgr->change_introspect_path(npath))
          return 1;
        break;
      case e_dir_mode::files:
        if (!rmgr->change_path(npath))
        {
          streamp[stream::stderr] << name << ": " << npath << ": not found" << std::endl;
          return 1;
        }
        break;
    }
    return 0;
  }, "change the current working directory (depends on which mode is active)", "[directory/path]");
  blt.do_not_use_program_options();
  sh.get_builtin_manager().register_builtin("cd", blt);
}

template<typename Cont, typename Func>
void ls_print_short(neam::r::shell::stream_pack &streamp, const Cont &cont, Func f)
{
  size_t cnt = 0;
  bool first = true;
  for (const auto &it : cont)
  {
    std::string r = f(it);
    cnt += r.size() + 8;
    if (cnt >= 240)
    {
      first = true;
      cnt = r.size();
      streamp[neam::r::shell::stream::stdout] << std::endl;
    }
    else if (!first)
      streamp[neam::r::shell::stream::stdout] << "\t\t";
    first = false;
    streamp[neam::r::shell::stream::stdout] << r;
  }
  if (cnt)
    streamp[neam::r::shell::stream::stdout] << std::endl;
}

template<typename Cont, typename Func>
void ls_print_long(neam::r::shell::stream_pack &streamp, const Cont &cont, Func f)
{
  for (const auto &it : cont)
  {
    f(it, streamp[neam::r::shell::stream::stdout]);
    streamp[neam::r::shell::stream::stdout] << std::endl;
  }
}
void print_introspect(const neam::r::introspect &it, std::iostream &ios)
{
  const size_t lcount = neam::r::get_launch_count() - 1;
  auto tm_self = get_time(it.get_average_self_duration());
  auto tm_xself = get_time(it.get_average_self_duration() * float(it.get_call_count() / lcount));
  auto tm_gbl = get_time(it.get_average_duration());
  auto tm_xgbl = get_time(it.get_average_duration() * float(it.get_call_count() / lcount));
  ios << "[" << std::setw(5) << std::right << it.get_call_count() / lcount << " / "
      << std::setw(3) << std::right << it.get_failure_count() << "] "
      << "s:" << std::setw(3) << int(tm_self.first) << std::setw(3) << std::left << (tm_self.second + "s ") << std::right
      << "xs:" << std::setw(3) << int(tm_xself.first) << std::setw(3) << std::left << (tm_xself.second + "s ") << std::right
      << "g:" << std::setw(3) << int(tm_gbl.first) << std::setw(3) << std::left << (tm_gbl.second + "s ") << std::right
      << "xg:" << std::setw(3) << int(tm_xgbl.first) << std::setw(3) << std::left << (tm_xgbl.second + "s ") << std::right
      << "  " << it.get_name();
}

auto print_file_info(neam::r::shell::shell &, const std::string &add_path)
{
  return [&](const std::string &it, std::iostream &ios)
  {
    neam::r::file_info info = rmgr->get_info_for_path((add_path.size() ? add_path + "/" : "") + it);
    const size_t lcount = neam::r::get_launch_count() - 1;
    auto tm_self = get_time(info.average_self_time);
    auto tm_xself = get_time(info.average_self_time * float(info.average_self_time_count / lcount));
    auto tm_gbl = get_time(info.average_global_time);
    auto tm_xgbl = get_time(info.average_global_time * float(info.average_global_time_count / lcount));
    ios << "[" << std::setw(7) << std::right << info.hit_count / lcount << " / "
        << std::setw(4) << std::right << info.error_count << "] "
        << "s:" << std::setw(3) << int(tm_self.first) << std::setw(3) << std::left << (tm_self.second + "s ") << std::right
        << "xs:" << std::setw(3) << int(tm_xself.first) << std::setw(3) << std::left << (tm_xself.second + "s ") << std::right
        << "g:" << std::setw(3) << int(tm_gbl.first) << std::setw(3) << std::left << (tm_gbl.second + "s ") << std::right
        << "xg:" << std::setw(3) << int(tm_xgbl.first) << std::setw(3) << std::left << (tm_xgbl.second + "s ") << std::right
        << "  " << it;
  };
}

void neam::r::shell::blt_ls(neam::r::shell::shell &sh)
{
  builtin &blt = *new builtin([&](const std::string &, variable_stack &, stream_pack &streamp, boost::program_options::variables_map &vm) -> int
  {
    bool long_option = vm.count("long");
    bool force_function = vm.count("function");
    std::string add_path;

    if (vm.count("path"))
      add_path = vm["path"].as<std::string>();

    switch (dir_mode)
    {
      case e_dir_mode::functions:
      {
        if (!long_option)
          ls_print_short(streamp, rmgr->get_child_introspect(), [](const introspect & it) -> std::string { return it.get_name(); });
        else
          ls_print_long(streamp, rmgr->get_child_introspect(), print_introspect);
        break;
      }
      case e_dir_mode::files:
      {
        file_info finfo = rmgr->get_info_for_path(add_path);
        if (finfo.files.empty() || force_function)
        {
          if (!long_option)
            ls_print_short(streamp, finfo.functions, [](const introspect & it) -> std::string { return it.get_name(); });
          else
            ls_print_long(streamp, finfo.functions, print_introspect);
        }
        else
        {
          if (!long_option)
            ls_print_short(streamp, finfo.files, [](const std::string & it) -> std::string { return it; });
          else
            ls_print_long(streamp, finfo.files, print_file_info(sh, add_path));
        }
        break;
      }
    }
    return 0;
  }, "list content of the current working directory/function (depends on which mode is active)", "[options] [path]");
  blt.add_options()("long,l", "use a long listing format")
                   ("function,f", "in file mode, display the information about functions of the current directory tree")
                   ("path", boost::program_options::value<std::string>(), "specify a custom path (relative or absolute). Only works in file mode.");
  blt.get_positional_options_description().add("path", 1);
  blt.disallow_unknow_parameters();
  sh.get_builtin_manager().register_builtin("ls", blt);
}

void info_print_file_info(const neam::r::file_info &info, bool full_listing, std::iostream &ios)
{
  const size_t lcount = neam::r::get_launch_count() - 1;
  ios << "path: " << info.path << "\n";

  // print number things:
  if (info.average_global_time_count)
  {
    auto tm = get_time(info.average_global_time);
    auto xtm = get_time(info.average_global_time * float(info.hit_count / lcount));
    ios << "average global time: " << tm.first << tm.second << "s\n"
        << "total global time (per-launch): " << xtm.first << xtm.second << "s\n";
  }
  else
    ios << "average global time not available\n";
  if (info.average_self_time_count)
  {
    auto tm = get_time(info.average_self_time);
    auto xtm = get_time(info.average_self_time * float(info.hit_count / lcount));
    ios << "average self time: " << tm.first << tm.second << "s\n"
        << "total self time (per-launch): " << xtm.first << xtm.second << "s\n";
  }
  else
    ios << "average self time not available\n";

  ios << "number of error: " << info.error_count << "\n";

  // The full listing
  if (full_listing)
  {
    ios << "files:\n";
    for (const std::string &it : info.files)
      ios << "  " << it << '\n';
    ios << std::endl;
    ios << "functions:\n";
    for (const neam::r::introspect &it : info.functions)
      ios << "  " << it.get_name() << '\n';
    ios << std::endl;
  }
}

void info_print_introspect_info(neam::r::introspect &info, bool full_listing, std::iostream &ios)
{
  const size_t lcount = neam::r::get_launch_count() - 1;
  ios << "pretty name: " << info.get_pretty_name() << '\n'
      << "usual name: " << info.get_name() << '\n'
      << "number of call (per-launch average): " << (info.get_call_count() / float(lcount)) << '\n'
      << "is contextualized: " << std::boolalpha << info.is_contextual() << '\n';
  if (!info.get_file().empty())
    ios << "file: " << info.get_file() << ": " << info.get_line() << '\n';
  if (info.get_average_duration_count())
  {
    auto tm = get_time(info.get_average_duration());
    auto xtm = get_time(info.get_average_duration() * float(info.get_call_count() / lcount));
    ios << "average global time: " << tm.first << tm.second << "s\n"
        << "total global time (per-launch): " << xtm.first << xtm.second << "s\n";
  }
  else
    ios << "average global time not available\n";
  if (info.get_average_self_duration_count())
  {
    auto tm = get_time(info.get_average_self_duration());
    auto xtm = get_time(info.get_average_self_duration() * float(info.get_call_count() / lcount));
    ios << "average self time: " << tm.first << tm.second << "s\n"
        << "total self time (per-launch): " << xtm.first << xtm.second << "s\n";
  }
  else
    ios << "average self time not available\n";

  if (full_listing)
  {
    // time thing
    std::deque<neam::r::duration_progression> sdp = info.get_self_duration_progression();
    if (sdp.size())
    {
      ios << "self duration progression:\n";
      for (const neam::r::duration_progression &dp : sdp)
      {
        auto dptm = get_time(dp.value);
        ios << "  at " << std::put_time(std::localtime((long *)&dp.timestamp), "%F %T") << ": " << int(dptm.first) << dptm.second << "s\n";
      }
    }
    std::deque<neam::r::duration_progression> gdp = info.get_global_duration_progression();
    if (gdp.size())
    {
      ios << "global duration progression:\n";
      for (const neam::r::duration_progression &dp : gdp)
      {
        auto dptm = get_time(dp.value);
        ios << "  at " << std::put_time(std::localtime((long *)&dp.timestamp), "%F %T") << ": " << int(dptm.first) << dptm.second << "s\n";
      }
    }
  }

  // measure points
  const auto &measure_point_map = info.get_measure_point_map();
  if (measure_point_map.size())
  {
    ios << "measure points:\n";
    for (const auto & it : measure_point_map)
    {
      auto mpetm = get_time(it.second.value);
      ios << "  " << it.first << ": " << mpetm.first << mpetm.second << "s\n";
    }
  }

  if (full_listing)
  {
    // callee/caller
    std::vector<neam::r::introspect> vct = info.get_callee_list();
    if (vct.size())
      ios << "calls:\n";
    for (const neam::r::introspect &it : vct)
      ios << "  " << it.get_pretty_name() << '\n';

    vct = info.get_caller_list();
    if (vct.size())
      ios << "called by:\n";
    for (const neam::r::introspect &it : vct)
      ios << "  " << it.get_pretty_name() << '\n';
  }

  ios << "number of failures: " << info.get_failure_count() << " (ratio: " << std::setprecision(3) << (info.get_failure_ratio() * 100) << "%)\n";
}

// can't do otherwise: I cannot construct "empty" introspects.
neam::r::introspect info_get_introspect(bool global, const std::string &name)
{
  if (name.empty())
  {
    if (rmgr->get_top_introspect())
    {
      if (!global)
        return *rmgr->get_top_introspect();
      else
      {
        neam::r::introspect ret = (*rmgr->get_top_introspect());
        ret.remove_context();
        return ret;
      }
    }
    else
      throw std::runtime_error("there's no current function");
  }

  neam::r::introspect itp(name.c_str());
  if (!global)
  {
    neam::r::introspect *titp = rmgr->get_top_introspect();
    if (!titp)
      throw std::runtime_error("there's no current function");
    if (!itp.set_context(*titp))
      throw std::runtime_error("requested function is not called by the current one");
  }
  return itp;
}

void neam::r::shell::blt_info(neam::r::shell::shell &sh)
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &, stream_pack &streamp, boost::program_options::variables_map &vm) -> int
  {
    std::string element;

    // gather options
    if (vm.count("element"))
      element = vm["element"].as<std::string>();
    bool all_l = vm.count("all");
    bool short_l = vm.count("short") && !all_l;
    bool error = vm.count("error");
    bool global = vm.count("global");
    size_t error_count = vm["error-count"].as<size_t>();

    // run ! run ! run !
    switch (dir_mode)
    {
      case e_dir_mode::functions:
      {
        try
        {
          neam::r::introspect itp = info_get_introspect(global, element);

          if (short_l)
          {
            print_introspect(itp, streamp[stream::stdout]);
            streamp[stream::stdout] << std::endl;
          }
          else
            info_print_introspect_info(itp, all_l, streamp[stream::stdout]);

          // print errors
          if (error)
          {
            std::vector<neam::r::reason> errlist = itp.get_failure_reasons(error_count);
            if (errlist.size())
            {
              streamp[stream::stdout] << "errors:";
              if (itp.is_contextual())
                streamp[stream::stdout] << " (including errors from other contexts)";
              streamp[stream::stdout] << '\n';
            }
            for (const neam::r::reason & r : errlist)
            {
              streamp[stream::stdout] << "  " << r.type << ": '" << r.message << "'" << "\n"
              << "  | file: " << (r.file.size() ? r.file : "[---]") << " line " << r.line << "\n"
              << "  |" << (all_l ? ' ' : '_') << "number of time reported: " << r.hit << "\n";
              if (all_l)
              {
                streamp[stream::stdout] << "  |  from: " << std::put_time(std::localtime((long *)&r.initial_timestamp), "%F %T") << "\n";
                streamp[stream::stdout] << "  |__to:   " << std::put_time(std::localtime((long *)&r.last_timestamp), "%F %T") << std::endl;
              }
            }
          }
        }
        catch (std::exception &e)
        {
          streamp[stream::stderr] << name << ": " << e.what() << std::endl;
          return 1;
        }
        break;
      }
      case e_dir_mode::files:
      {
        if (global)
          streamp[stream::stderr] << name << ": warning: in file mode the --global (-g) option has no meaning" << std::endl;
        if (error)
          streamp[stream::stderr] << name << ": warning: in file mode the --error (-e) option has no meaning" << std::endl;
        // we can ignore global, error & error_count
        if (short_l)
        {
          print_file_info(sh, "")(element, streamp[stream::stdout]);
          streamp[stream::stdout] << std::endl;
        }
        else
          info_print_file_info(rmgr->get_info_for_path(element), all_l, streamp[stream::stdout]);
        break;
      }
    }
    streamp[stream::stdout] << std::endl;
    return 0;
  }, "display information about an element (file or function)", "[options] [element]");
  blt.add_options()("all,a", "print everything that it can (except errors)")
                   ("short,s", "display the same thing a `ls -l` would do")
                   ("error,e", "display the ${error-count} last error(s) (only valid in the function mode)")
                   ("error-count,c", boost::program_options::value<size_t>()->default_value(1), "set the number of errors to display")
                   ("global,g", "specify that the search scope is global: the 'element' is the name of a function, not a path (relative or not). Only works for functions.")
                   ("element", boost::program_options::value<std::string>(), "specify a custom element (relative or absolute). Defaults to the cwd element.");
  blt.get_positional_options_description().add("element", 1);
  blt.disallow_unknow_parameters();
  sh.get_builtin_manager().register_builtin("info", blt);
}

void neam::r::shell::blt_stash(neam::r::shell::shell &sh)
{
    builtin &blt = *new builtin([&](const std::string &name, variable_stack &, stream_pack &streamp, boost::program_options::variables_map &vm) -> int
  {
    std::string element;

    // gather options
    if (vm.count("select"))
      element = vm["select"].as<std::string>();
    bool only_active = vm.count("active");
    bool with_timestamp = vm.count("timestamp");

    // run ! run ! run !

    if (vm.count("select"))
    {
      if (!neam::r::load_data_from_stash(element))
      {
        streamp[stream::stderr] << name << ": " << element << ": Cannot find a stash entry with this name" << std::endl;
        return 1;
      }
      only_active = true;
    }

    std::vector<long> timestamps = neam::r::get_stashes_timestamp();
    std::vector<std::string> names = neam::r::get_stashes_name();
    const size_t active_idx = neam::r::get_active_stash_index();

    if (only_active)
    {
      streamp[stream::stdout] << "* " << names[active_idx];
      if (with_timestamp && timestamps[active_idx])
        streamp[stream::stdout] << "  [" << std::put_time(std::localtime(&timestamps[active_idx]), "%F %T") << "]";
      else if (with_timestamp)
        streamp[stream::stdout] << "  [active]";
      streamp[stream::stdout] << std::endl;
    }
    else
    {
      for (size_t i = 0; i < names.size(); ++i)
      {
        if (i == active_idx)
          streamp[stream::stdout] << "* ";
        else
          streamp[stream::stdout] << "  ";

        streamp[stream::stdout] << names[i];
        if (with_timestamp && timestamps[i])
          streamp[stream::stdout] << "  [" << std::put_time(std::localtime(&timestamps[i]), "%F %T") << "]";
        else if (with_timestamp)
          streamp[stream::stdout] << "  [active]";
        streamp[stream::stdout] << std::endl;
      }
    }

    return 0;
  }, "manage stashes", "[options] [[-s] stash-name]");
  blt.add_options()("select,s", boost::program_options::value<std::string>(), "Select a stash to use")
                   ("active,a", "only print the active stash")
                   ("timestamp,t", "also display the timestamp");
  blt.get_positional_options_description().add("select", 1);
  blt.disallow_unknow_parameters();
  sh.get_builtin_manager().register_builtin("stash", blt);
}


void neam::r::shell::blt_callgraph2dot(neam::r::shell::shell &)
{
  // TODO
}

