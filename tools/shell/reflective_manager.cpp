
#include <iostream>
#include <iomanip>
#include <tools/logger/logger.hpp>
#include "reflective_manager.hpp"

// first is the time, second the unit
std::pair<double, std::string> get_time(double sec_time);

neam::r::manager::manager()
{
  load_data();
}

neam::r::manager::~manager() {}

void neam::r::manager::load_data()
{
  std::list<introspect> done_list;

  std::vector<introspect> root_list = introspect::get_root_function_list();
  std::list<introspect> to_do(root_list.begin(), root_list.end());
  root_list.clear();

  bool common_path_init = false;
  std::map<std::string, file_info> temp_per_file_info;
  std::string common_path; // the base path that all paths have in common

  // We walk the full graph
  while (to_do.size())
  {
    introspect it = to_do.front();
    to_do.pop_front();
    it.remove_context(); // go context-free
    if (std::find(done_list.begin(), done_list.end(), it) == done_list.end()) // not already done !
    {
      done_list.push_back(it);
      { // populate the to_do list with methods/functions it calls
        std::vector<introspect> next = it.get_callee_list();
        to_do.insert(to_do.end(), next.begin(), next.end());
      }

      // create/update the common_path
      if (!it.get_file().empty() && !common_path_init)
      {
        common_path = it.get_file();
        common_path_init = true;
      }
      else if (!it.get_file().empty())
      {
        std::string file = it.get_file();
        for (size_t i = 0, l = std::min(common_path.size(), file.size()); i < l; ++i)
        {
          if (common_path[i] != file[i])
          {
            common_path.resize(i ? i - 1 : 0);
            break;
          }
        }
      }

      // compile informations
      file_info &nfo = temp_per_file_info[it.get_file().size() ? it.get_file() : "/:orphan"];

      nfo.path = it.get_file().size() ? it.get_file() : "/:orphan";
      nfo.hit_count += it.get_call_count();
      nfo.error_count += it.get_failure_count();
      nfo.functions.push_back(it);
      if (it.get_average_duration_count())
      {
        nfo.average_global_time = (nfo.average_global_time * float(nfo.average_global_time_count)
                                   + it.get_average_duration() * float(it.get_average_duration_count()))
                                  / float(nfo.average_global_time_count + it.get_average_duration_count());
        nfo.average_global_time_count += it.get_average_duration_count();
      }
      if (it.get_average_self_duration_count())
      {
        nfo.average_self_time = (nfo.average_self_time * float(nfo.average_self_time_count)
                                   + it.get_average_self_duration() * float(it.get_average_self_duration_count()))
                                  / float(nfo.average_self_time_count + it.get_average_self_duration_count());
        nfo.average_self_time_count += it.get_average_self_duration_count();
      }
    }
  }

  // reduce the paths
  for (auto &it : temp_per_file_info)
  {
    // for every paths, except for the /:orphan one, we truncate and normalize them
    if (it.second.path != "/:orphan")
    {
      std::string ps = it.second.path.substr(common_path.size());
      if (common_path == it.second.path)
        ps = common_path;
      boost::filesystem::path p(ps);
      if (common_path == it.second.path)
        p = "/" + p.leaf().generic_string();
      p.normalize();
      it.second.path = p.generic_string();
    }
    // print some quick information
    const size_t lcount = neam::r::get_launch_count() - 1;
    auto tm_self = get_time(it.second.average_self_time * float(it.second.hit_count / lcount));
    auto tm_gbl = get_time(it.second.average_global_time * float(it.second.hit_count / lcount));
    std::cout << "  --  " << std::setw(60) << std::left << it.second.path << " [self: ~" << int(tm_self.first) << tm_self.second << "s]" << std::endl;
    per_file_info["/" + it.second.path] = it.second;
  }
}

neam::r::file_info neam::r::manager::get_info_for_path(std::string path)
{
  std::string current_path = file_path.generic_string();

  while (path.back() == '/' && path.size() > 1)
    path.pop_back();
  if (path.size()) // we got a path (relative or absolute) as argument
  {
    boost::filesystem::path np(path); // new path
    boost::filesystem::path fp;       // file-path

    if (np.is_absolute())
      fp = np;
    else
      fp = file_path / np;
    fp.normalize();
    current_path = fp.generic_string();
  }
  if (current_path == "./")
    current_path = file_path.generic_string();
  if (current_path == "/.")
    current_path = "/";
  while (current_path.substr(0, 3) == "/.." &&
         (current_path.size() == 3 || (current_path.size() > 4 && current_path[3] == '/')))
    current_path = current_path.substr(3);

  if (per_file_info.count(current_path))
    return per_file_info.at(current_path);

  file_info ret;
  ret.path = current_path;

  for (std::pair<const std::string, file_info> &it : per_file_info)
  {
    // Test for prefix / equality
    if (it.first.size() >= current_path.size()
        && !it.first.compare(0, current_path.size(), current_path))
    {
      ret.hit_count += it.second.hit_count;
      ret.error_count += it.second.error_count;

      ret.average_global_time = ret.average_global_time * float(ret.average_global_time_count) + it.second.average_global_time * it.second.average_global_time_count;
      if (ret.average_global_time_count || it.second.average_global_time_count)
        ret.average_global_time /= float(ret.average_global_time_count + it.second.average_global_time_count);
      ret.average_global_time_count += it.second.average_global_time_count;

      ret.average_self_time = ret.average_self_time * float(ret.average_self_time_count) + it.second.average_self_time * it.second.average_self_time_count;
      if (ret.average_self_time_count || it.second.average_self_time_count)
        ret.average_self_time /= float(ret.average_self_time_count + it.second.average_self_time_count);
      ret.average_self_time_count += it.second.average_self_time_count;

      ret.functions.insert(ret.functions.end(), it.second.functions.begin(), it.second.functions.end());

      std::string name;
      size_t i = current_path.size();
      if (it.first[i] == '/')
        ++i;
      for (; i < it.first.size(); ++i)
      {
        name += it.first[i];
        if (it.first[i] == '/')
          break;
      }
      ret.files.emplace(name);
    }
  }
  return ret;
}

bool neam::r::manager::change_path(const std::string &new_path)
{
  boost::filesystem::path op(file_path);
  boost::filesystem::path np(new_path);

  if (np.is_absolute())
    file_path = np;
  else
    file_path = file_path / np;

  file_path.normalize();

  std::string current_path = file_path.generic_string();

  if (current_path == "./")
    current_path = op.generic_string();
  if (current_path == "/.")
    current_path = "/";
  while (current_path.substr(0, 3) == "/.." &&
         (current_path.size() == 3 || (current_path.size() > 4 && current_path[3] == '/')))
    current_path = current_path.substr(3);
  file_path = current_path;

  // check the new path
  for (std::pair<const std::string, file_info> &it : per_file_info)
  {
    // Test for prefix / equality
    if (it.first.size() >= current_path.size()
        && !it.first.compare(0, current_path.size(), current_path))
    {
      // we got possibly a partial matching
      if (current_path != "/" && it.first.size() != current_path.size()
          && it.first.compare(0, current_path.size() + 1, current_path + "/"))
        continue;
      // everything's OK
      return true;
    }
  }

  // revert
  file_path = op;
  return false;
}

bool neam::r::manager::change_introspect_path(const std::string &new_path)
{
  boost::filesystem::path np(new_path);
  np.normalize();

  for (boost::filesystem::path pit : np)
  {
    std::string it = pit.generic_string();

    if (it == "/")
      itp_path.clear();
    else if (it == "..")
      pop_introspect();
    else
    {
      std::vector<neam::r::introspect> chld = get_child_introspect();
      auto chld_it = std::find_if(chld.begin(), chld.end(), [&](const neam::r::introspect &chit) -> bool
      {
        return it == chit.get_name() || it == chit.get_pretty_name();
      });
      if (chld_it == chld.end())
      {
        neam::cr::out.error() << LOGGER_INFO << "can't find " << it << " in " << get_current_introspect_path() << std::endl;
        return false;
      }
      push_introspect(*chld_it);
    }
  }

  return true;
}

neam::r::introspect *neam::r::manager::get_top_introspect()
{
  if (itp_path.size())
    return &itp_path.back();
  return nullptr;
}


std::vector<neam::r::introspect> neam::r::manager::get_child_introspect() const
{
  if (itp_path.size())
    return itp_path.back().get_callee_list();
  return introspect::get_root_function_list();
}

void neam::r::manager::push_introspect(const neam::r::introspect &cwi)
{
  if (cwi.is_contextual())
    itp_path.push_back(cwi);
}

neam::r::introspect *neam::r::manager::pop_introspect(size_t count)
{
  while (count-- && itp_path.size())
    itp_path.pop_back();
  if (itp_path.size())
    return &itp_path.back();
  return nullptr;
}

std::string neam::r::manager::get_current_introspect_path() const
{
  if (itp_path.empty())
    return "/";
  std::string path;
  for (const introspect &it : itp_path)
    path += "/" + std::string(it.get_name());
  return path;
}
