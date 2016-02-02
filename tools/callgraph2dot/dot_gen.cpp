
#include <map>
#include <iomanip>
#include <set>
#include "dot_gen.hpp"

bool neam::r::callgraph_to_dot::write_to_stream(std::ostream &os, neam::r::introspect *root)
{
  if (root && !root->is_contextual())
    return false;

  os << "digraph callgraph {\n";

  std::map<const char *, bool> is_root;

  // walk the roots
  if (root)
  {
    walk_get_max(*root);
    walk_root(os, *root);
    is_root[root->get_pretty_name()] = true;
  }
  else
  {
    std::vector<neam::r::introspect> roots = neam::r::introspect::get_root_function_list();
    for (neam::r::introspect &it : roots)
      walk_get_max(it);

    for (neam::r::introspect &it : roots)
    {
      walk_root(os, it);
      is_root[it.get_pretty_name()] = true;
    }
  }

  os << "\n";

  for (auto &it : labels)
  {
    // split the string
    std::string str = it.first;
    for (size_t i = 0, j = 0; i < str.size(); ++i, ++j)
    {
      if (j >= 20 && (std::isspace(str[i]) || str[i] == ':'))
      {
        j = 0;
        str.insert(i, "\n");
      }
    }
    // print the label for the node
    os << "  N" << it.second << " [label=" << std::quoted(str) << ";";
    if (is_root[it.first])
      os << "shape=box;";
    os << "];\n";
  }

  os << "}\n";

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
  max_self = std::max(max_self, float(root.get_average_self_duration()));

  for (neam::r::introspect &callee : callees)
    walk_get_max(callee);
}

// walk a single root and print the graph
// NOTE: this is recursive
// This is not the best code I have ever written, but it works.
void neam::r::callgraph_to_dot::walk_root(std::ostream &os, neam::r::introspect &root)
{
  const char *name = root.get_pretty_name();
  size_t idx;
  if (!labels.count(name))
  {
    idx = counter;
    ++counter;
    labels[name] = idx;
  }
  else
  {
    idx = labels[name];
  }

  // output the callgraph
  std::vector<neam::r::introspect> callees = root.get_callee_list();

  for (neam::r::introspect &callee : callees)
  {
    walk_root(os, callee);

    size_t subidx = labels[callee.get_pretty_name()];
    std::pair<double, std::string> self_tm = get_time(callee.get_average_self_duration());
    std::pair<double, std::string> gbl_tm = get_time(callee.get_average_duration());

    // output the edge
    float weight = std::max(float(callee.get_call_count()) / max_count * 2.5f, 0.5f);
           weight += std::max(float(callee.get_average_self_duration()) / max_self * 2.5f, 0.5f);
    weight = std::min(weight, 5.f);
    os << "  N" << idx << " -> N" << subidx << " ["
       << "label=\" " << callee.get_call_count() << "\\n"
                      << " self " << size_t(self_tm.first) << self_tm.second << "s\\n"
                      << " gbl " << size_t(gbl_tm.first) << gbl_tm.second << "s" << "\";"
//        << "weight=" << callee.get_call_count() << ";"
       << "penwidth=" << weight << ";";

    if (callee.get_failure_ratio())
    {
      os << "color=\"#" << std::hex << std::setw(2) << std::setfill('0') << std::min(size_t(callee.get_failure_ratio() * 150. + 105.f), 255ul) << std::dec <<"0000\";";
    }

    os << "];\n";
    if (callee.get_failure_ratio())
    {
      os << "  N" << subidx << " [penwidth=3;color=\"#" << std::hex << std::setw(2) << std::setfill('0') << std::min(size_t(callee.get_failure_ratio() * 150. + 105.f), 255ul) << std::dec <<"0000\";]";
    }
  }

  // output some fail reasons
  if (out_error)
  {
    std::vector<neam::r::reason> rsn = root.get_failure_reasons(10);
    std::set<size_t> already_done;
    for (neam::r::reason & r : rsn)
    {
      size_t ridx;
      if (labels.count(r.type))
      {
        ridx = labels[r.type];
      }
      else
      {
        ridx = counter++;
        labels[r.type] = ridx;
        os << "  N" << ridx << " [shape=octagon;color=red;fontcolor=red;penwidth=3];\n";
      }
      if (!already_done.count(ridx))
      {
        already_done.emplace(ridx);
        os << "  N" << idx << " -> N" << ridx << " ["
           << "dir=none;"
           << "color=red;"
           << "style=dashed;"
           << "penwidth=3.5;"
           << "];\n";
      }
    }
  }
}
