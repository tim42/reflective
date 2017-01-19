
#include <map>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <cctype>
#include <algorithm>
#include "dot_gen.hpp"

bool neam::r::callgraph_to_dot::write_to_stream(std::ostream &os, neam::r::introspect *root)
{
  if (root && !root->is_contextual())
    return false;

  os << "digraph callgraph {\n";

  std::map<std::string, bool> is_root;

  // walk the roots
  if (root)
  {
    float error_factor;
    bool insignificant = false;
    walk_get_max(*root);
    max_self /= float(max_self_count);
    walk_root(os, *root, error_factor, insignificant);
    if (!insignificant)
      is_root[root->get_name()] = true;
  }
  else
  {
    std::vector<neam::r::introspect> roots = neam::r::introspect::get_root_function_list();
    for (neam::r::introspect &it : roots)
      walk_get_max(it);
    max_self /= float(max_self_count);

    for (neam::r::introspect &it : roots)
    {
      float error_factor;
      bool insignificant = false;
      walk_root(os, it, error_factor, insignificant);
      if (!insignificant)
        is_root[it.get_name()] = true;
    }
  }

  os << "\n  // reasons labels\n";

  for (auto &it : reason_idxs)
  {
    // create the label
    std::string label = it.first.type;
    if (!it.first.message.empty())
    {
      label += "\n";
      label += it.first.message;
    }
//     if (!it.first.file.empty())
//     {
//       label += "\n";
//       label += it.first.file;
//       label += ":" + std::to_string(it.first.line);
//     }

    // print the label for the reason
    os << "  N" << it.second << " [label=" << std::quoted(label) << "];\n";
  }

  os << "\n  // function/methods labels\n";

  for (auto &it : introspect_labels)
  {
    // create the label
    std::string str = it.second.get_pretty_name();

    // split the label
    for (size_t i = 0, j = 0; i < str.size(); ++i, ++j)
    {
      if (j >= 20 && (std::isspace(str[i]) || str[i] == ':'))
      {
        j = 0;
        str.insert(i, "\n");
      }
    }

    // Append the info string (if any)
    if (!it.second.get_file().empty())
    {
      str += "\n";
      str += it.second.get_file();
      str += " : " + std::to_string(it.second.get_line());
    }

    // print the label for the node
    os << "  N" << it.first << " [label=" << std::quoted(str) << ";shape=box;";
    if (is_root[it.second.get_name()])
      os << "penwidth=4;fontcolor=blue;";
    os << "];\n";
  }

  os << "}" << std::endl;

  return true;
}

// first is the time, second the unit
std::pair<double, std::string> get_time(double sec_time)
{
  double rtime = sec_time;

  const char *tab[] = {"", "m", "u", "n"};
  size_t idx = 0;

  for (; idx < (sizeof(tab) / sizeof(tab[0]) - 1) && rtime < 1.; ++idx)
  {
    rtime *= 1000;
  }

  return std::make_pair(rtime, tab[idx]);
}

void neam::r::callgraph_to_dot::walk_get_max(neam::r::introspect &root)
{
  std::vector<neam::r::introspect> callees = root.get_callee_list();

  max_count = std::max(max_count, float(root.get_call_count()));
  max_self += float(root.get_average_self_duration());
  max_self_count++;

  for (neam::r::introspect &callee : callees)
    walk_get_max(callee);
}
// walk a single root and print the graph
// NOTE: this is recursive
// This is not the best code I have ever written, but it works.
void neam::r::callgraph_to_dot::walk_root(std::ostream &os, neam::r::introspect &root, float &error_factor, bool &insignificant)
{
  error_factor = 0.f;
  std::string name = root.get_name();
  size_t idx = 0;

  // output the callgraph
  std::vector<neam::r::introspect> callees = root.get_callee_list();

  for (neam::r::introspect &callee : callees)
  {
    float callee_error_factor;
    bool callee_insignificant = remove_insignificant;
    float callee_call_count = float(callee.get_call_count());
    if (average_call_count)
      callee_call_count /= float(neam::r::get_launch_count() - 1); // remove the current launch count
    if ((callee_call_count > min_call_count) || (callee.get_average_duration() > min_gbl_time))
      callee_insignificant = false;

    walk_root(os, callee, callee_error_factor, callee_insignificant);

    if (!callee_insignificant)
    {
      idx = get_idx_for_introspect(root);
      insignificant = false;
      size_t subidx = get_idx_for_introspect(callee);
      std::pair<double, std::string> self_tm = get_time(callee.get_average_self_duration());
      std::pair<double, std::string> gbl_tm = get_time(callee.get_average_duration());

      // output the edge
      float weight = 0.f;
      if (weight_with_callcount)
        weight = std::max(float(callee.get_call_count()) / (max_count) * 3.5f, 0.45f);
      if (weight_with_global_time)
        weight += std::max(float(callee.get_average_duration()) / (max_self) * 3.5f, 0.45f);
      weight = std::min(weight, 6.f);
      os << "  N" << idx << " -> N" << subidx << " ["
         << "label=\" " << callee_call_count << "\\n"
         << " self " << size_t(self_tm.first) << self_tm.second << "s\\n"
         << " gbl " << size_t(gbl_tm.first) << gbl_tm.second << "s" << "\";"
//         << "weight=" << weight * 600.f << ";"
         << "penwidth=" << weight << ";";

      if (callee_error_factor)
      {
        insignificant = false;
        if (trace_full_error_path)
          error_factor += callee_error_factor;

        os << "color=\"#" << std::hex << std::setw(2) << std::setfill('0') << std::min(size_t(callee_error_factor * 150. + 105.f), size_t(255ul)) << std::dec << "0000\";";
      }

      os << "];\n";
    }
  }

  // output some fail reasons
  if (out_error)
  {
    std::vector<neam::r::reason> rsn = root.get_failure_reasons(100);

    if (rsn.empty())
      return;

    if (root.get_failure_ratio())
    {
      if (trace_full_error_path)
        error_factor += root.get_failure_ratio();
      insignificant = false;
    }

    idx = get_idx_for_introspect(root);

    if (introspect_errors.count(idx))
      return;
    introspect_errors.emplace(idx);

    // prepare the vector (uniquify it)
    std::unordered_map<neam::r::reason, neam::r::reason, internal::reason_hasher> rsn_map;
    for (neam::r::reason &r : rsn)
    {
      if (rsn_map.count(r))
        rsn_map[r].hit += r.hit;
      else
        rsn_map[r] = r;
    }

    for (auto & r : rsn_map)
      output_reason(os, idx, r.second);

    // update the box of the function
    os << "  N" << idx << " [penwidth=3;color=\"#"
       << std::hex << std::setw(2) << std::setfill('0') << std::min(size_t(root.copy_without_context().get_failure_ratio() * 150. + 105.f), size_t(255ul)) << std::dec << "0000\";]\n";
  }
}

void neam::r::callgraph_to_dot::output_reason(std::ostream &os, size_t idx, neam::r::reason &r)
{
  size_t ridx;
  if (!reason_idxs.count(r))
  {
    ridx = counter++;
    reason_idxs[r] = ridx;
    os << "  N" << ridx << " [shape=octagon;color=red;fontcolor=red;penwidth=3]; // failure node " << reason_idxs[r] << "\n";
  }
  else ridx = reason_idxs[r];//return; // already linked

  os << "  N" << idx << " -> N" << ridx << " ["
     << "label=\" x" << r.hit << "\";"
     << "dir=none;"
     << "color=red;"
     << "fontcolor=red;"
     << "style=dashed;"
     << "penwidth=3.5;"
     << "];\n";
}

size_t neam::r::callgraph_to_dot::get_idx_for_introspect(const neam::r::introspect &itr, bool *added)
{
  if (!idxs.count(itr.get_function_descriptor().key_name))
  {
    size_t idx = counter++;
    idxs[itr.get_function_descriptor().key_name] = idx;
    introspect_labels.emplace(idx, itr);
    introspect_labels.at(idx).remove_context();
    if (added)
      *added = true;
  }
  else if (added)
    *added = false;

  return idxs[itr.get_function_descriptor().key_name];
}

