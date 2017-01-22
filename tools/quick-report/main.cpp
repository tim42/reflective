
#include <fstream>
#include <map>

#include <reflective/reflective.hpp> // The reflective header
#include <tools/logger/logger.hpp>   // Just to set the logger in debug mode

#include "graph_walker.hpp"

// first is the time, second the unit
std::pair<double, const char *> get_time(double sec_time)
{
  double rtime = sec_time;

  const char *tab[] = {"s", "ms", "us", "ns"};
  size_t idx = 0;

  for (; idx < (sizeof(tab) / sizeof(tab[0]) - 1) && rtime < 1.; ++idx)
  {
    rtime *= 1000;
  }

  return std::make_pair(rtime, tab[idx]);
}

void print_function(const neam::r::introspect &intr, size_t spc_count = 0)
{
  std::string spcs;
  for (size_t i = 0; i < spc_count; ++i)
    spcs += ' ';
  const auto &fd = intr.get_function_descriptor();
  neam::cr::out.log() << spcs << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]:" << std::endl;
}

void print_callstack(const std::deque<neam::r::introspect> &stack, size_t spc_count = 2)
{
  std::string spcs;
  for (size_t i = 0; i < spc_count; ++i)
    spcs += ' ';

  neam::cr::out.log() << spcs << "callstack (most recent call last) :" << std::endl;
  for (const neam::r::introspect &it : stack)
  {
    const auto &fd = it.get_function_descriptor();
    neam::cr::out.log() << spcs << "  " << fd.pretty_name << "  [" << fd.file << ": " << fd.line << "]" << std::endl;
  }
}

struct report_entry
{
  neam::r::reason report_reason;
  std::deque<neam::r::introspect> stack;

  bool operator < (const report_entry &other) const
  {
    return report_reason.initial_timestamp < other.report_reason.initial_timestamp
           || stack < other.stack
           || report_reason.line < other.report_reason.line
           || report_reason.file < other.report_reason.file
           || report_reason.type < other.report_reason.type
           || report_reason.message < other.report_reason.message;
  }

  bool operator == (const report_entry &other) const
  {
    return other.report_reason == report_reason && stack == other.stack;
  }
};

struct introspect_entry
{
  neam::r::introspect intr;
  std::deque<neam::r::introspect> stack;
};

int main(int argc, char **argv)
{
  // Set the reflective configuration
  neam::r::conf::disable_auto_save = true;
  neam::r::conf::out_file = "";

  if (argc != 2)
  {
    neam::cr::out.log() << LOGGER_INFO << "Usage: " << argv[0] << " [reflective-file]" << neam::cr::newline
                        << "  it will then create (if the file is valid) a [reflective-file].report that will contain the report" << std::endl;
    return 1;
  }

  // load the file
  if (!neam::r::load_data_from_disk(argv[1]))
  {
    neam::cr::out.error() << LOGGER_INFO << "Error: Unable to load the file '" << argv[1] << "'. No output produced." << std::endl;
    return 2;
  }

  std::string out_file = std::string(argv[1]) + ".report";
  neam::cr::out.add_stream(*(new std::ofstream(out_file, std::ios_base::trunc)), true);
  neam::cr::out.no_header = true;

  const size_t stash_index = neam::r::get_active_stash_index();

  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
  neam::cr::out.log() << "report for " << argv[1] << " [stash: '" << neam::r::get_stashes_name()[stash_index] << "']" << std::endl;

  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
  neam::cr::out.log() << "ERRORS (local): " << std::endl;
  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

  neam::rtools::graph_walker::walk([](const neam::r::introspect &current, const std::deque<neam::r::introspect> &stack)
  {
    if (current.get_failure_count() > 0)
    {
      print_function(current);
      neam::cr::out.log() << "  list of errors:" << std::endl;
      for (const auto &r : current.get_failure_reasons(10))
      {
        neam::cr::out.log() << neam::cr::internal::end_header
                            << "   -" << r.type << ": '" << r.message << "'" << neam::cr::newline
                            << "    | file: " << (r.file.size() ? r.file : "[---]") << " line " << r.line << neam::cr::newline
                            << "    | number of time reported: " << r.hit << neam::cr::newline
                            << "    |  from: " << std::put_time(std::localtime((const time_t *)&r.initial_timestamp), "%F %T") << neam::cr::newline
                            << "    |__to:   " << std::put_time(std::localtime((const time_t *)&r.last_timestamp), "%F %T") << std::endl;
      }
      print_callstack(stack);
      neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
    }
  });


  neam::cr::out.log() << std::endl;
  neam::cr::out.log() << "MEASURE POINTS (local): " << std::endl;
  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

  neam::rtools::graph_walker::walk([](const neam::r::introspect &current, const std::deque<neam::r::introspect> &stack)
  {
    auto mpm = current.get_measure_point_map();
    if (mpm.size() > 0)
    {
      print_function(current);
      neam::cr::out.log() << "  list of measure points:" << std::endl;
      for (const auto &m : mpm)
      {
        auto tm = get_time(m.second.value);
        neam::cr::out.log() << "   - " << m.first << ": " << tm.first << tm.second << " [hit " << m.second.hit_count << " time]" << std::endl;
      }
      print_callstack(stack);

      neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
    }
  });


  // stats reports //

  constexpr size_t func_count = 50;

  {
    std::multimap<double, introspect_entry> intr_by_lcl_self_time;
    std::multimap<double, introspect_entry> intr_by_lcl_self_time_per_call;
    std::multimap<size_t, introspect_entry> intr_by_lcl_call_count;

    std::multimap<double, introspect_entry> intr_by_ttl_self_time;
    std::multimap<double, introspect_entry> intr_by_ttl_self_time_per_call;
    std::multimap<size_t, introspect_entry> intr_by_ttl_call_count;

    // generate the global map
    neam::rtools::graph_walker::walk([&](const neam::r::introspect &current, const std::deque<neam::r::introspect> &stack)
    {
      intr_by_lcl_self_time.insert({current.get_average_self_duration() * double(current.get_call_count()), {current, stack}});
      intr_by_lcl_call_count.insert({current.get_call_count(), {current, stack}});
      intr_by_lcl_self_time_per_call.insert({current.get_average_self_duration(), {current, stack}});

      neam::r::introspect gbl = current.copy_without_context();
      intr_by_ttl_self_time.insert({gbl.get_average_self_duration() * double(gbl.get_call_count()), {gbl, {}}});
      intr_by_ttl_self_time_per_call.insert({gbl.get_average_self_duration(), {gbl, {}}});
      intr_by_ttl_call_count.insert({gbl.get_call_count(), {gbl, {}}});
    });

    neam::cr::out.log() << std::endl;
    neam::cr::out.log() << "TOP " << func_count << " functions (total time spent -- localized): " << std::endl;
    neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

    size_t i = 0;
    for (auto it = intr_by_lcl_self_time.rbegin(); it != intr_by_lcl_self_time.rend(); ++it, ++i)
    {
      if (i >= func_count) 
        break;

      const auto &fd = it->second.intr.get_function_descriptor();
      auto tm = get_time(it->first);
      auto avgtm = get_time(it->second.intr.get_average_self_duration());
      neam::cr::out.log() << "  " << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]: " << tm.first << tm.second << " "
                          "[avg " << avgtm.first << avgtm.second << " / call, " << it->second.intr.get_call_count() << " calls]" << std::endl;
      print_callstack(it->second.stack, 4);
    }

    neam::cr::out.log() << std::endl;
    neam::cr::out.log() << "TOP " << func_count << " functions (total time spent -- global): " << std::endl;
    neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

    i = 0;
    neam::r::introspect *last = nullptr;
    for (auto it = intr_by_ttl_self_time.rbegin(); it != intr_by_ttl_self_time.rend(); ++it, ++i)
    {
      if (i >= func_count)
        break;
      if (last && *last == it->second.intr)
        continue;
      last = &it->second.intr;

      const auto &fd = it->second.intr.get_function_descriptor();
      auto tm = get_time(it->first);
      neam::cr::out.log() << "  " << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]: " << tm.first << tm.second << std::endl;
    }

    neam::cr::out.log() << std::endl;
    neam::cr::out.log() << "TOP " << func_count << " functions (time spent per call -- localized): " << std::endl;
    neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

    i = 0;
    for (auto it = intr_by_lcl_self_time_per_call.rbegin(); it != intr_by_lcl_self_time_per_call.rend(); ++it, ++i)
    {
      if (i >= func_count)
        break;

      const auto &fd = it->second.intr.get_function_descriptor();
      auto tm = get_time(it->first);
      auto avgtm = get_time(it->second.intr.get_average_self_duration());
      neam::cr::out.log() << "  " << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]: " << tm.first << tm.second << " "
                          "[avg " << avgtm.first << avgtm.second << " / call, " << it->second.intr.get_call_count() << " calls]" << std::endl;
      print_callstack(it->second.stack, 4);
    }

    neam::cr::out.log() << std::endl;
    neam::cr::out.log() << "TOP " << func_count << " functions (time spent per call -- global): " << std::endl;
    neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

    i = 0;
    last = nullptr;
    for (auto it = intr_by_ttl_self_time_per_call.rbegin(); it != intr_by_ttl_self_time_per_call.rend(); ++it, ++i)
    {
      if (i >= func_count)
        break;
      if (last && *last == it->second.intr)
        continue;
      last = &it->second.intr;

      const auto &fd = it->second.intr.get_function_descriptor();
      auto tm = get_time(it->first);
      neam::cr::out.log() << "  " << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]: " << tm.first << tm.second << std::endl;
    }

    neam::cr::out.log() << std::endl;
    neam::cr::out.log() << "TOP " << func_count << " functions (call count -- localized): " << std::endl;
    neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

    i = 0;
    for (auto it = intr_by_lcl_call_count.rbegin(); it != intr_by_lcl_call_count.rend(); ++it, ++i)
    {
      if (i >= func_count)
        break;

      const auto &fd = it->second.intr.get_function_descriptor();
      neam::cr::out.log() << "  " << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]: " << it->first << " calls" << std::endl;
      print_callstack(it->second.stack, 4);
    }

    neam::cr::out.log() << std::endl;
    neam::cr::out.log() << "TOP " << func_count << " functions (call count -- global): " << std::endl;
    neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

    i = 0;
    last = nullptr;
    for (auto it = intr_by_ttl_call_count.rbegin(); it != intr_by_ttl_call_count.rend(); ++it, ++i)
    {
      if (i >= func_count)
        break;
      if (last && *last == it->second.intr)
        continue;
      last = &it->second.intr;

      const auto &fd = it->second.intr.get_function_descriptor();
      neam::cr::out.log() << "  " << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]: " << it->first << " calls" << std::endl;
    }

    neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
  }


  // probably huge things //

  neam::cr::out.log() << std::endl;
  neam::cr::out.log() << "SEQUENCES (local): " << std::endl;
  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

  neam::rtools::graph_walker::walk([](const neam::r::introspect &current, const std::deque<neam::r::introspect> &stack)
  {
    auto msq = current.get_sequences();
    if (msq.size() > 0)
    {
      print_function(current);
      neam::cr::out.log() << "  list of sequences:" << std::endl;
      for (const auto &sq : msq)
      {
        neam::cr::out.log() << "   - " << sq.first << ": " << std::endl;
        for (const auto &en : sq.second.get_entries())
        {
          neam::cr::out.log() << "     - " << en.name << ": " << en.description << " [" << en.file << ": " << en.line << "]" << std::endl;
        }
      }
      print_callstack(stack);

      neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
    }
  });


  neam::cr::out.log() << std::endl;
  neam::cr::out.log() << "REPORTS (may be huge, but it's the last thing): " << std::endl;
  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

  neam::rtools::graph_walker::walk([](const neam::r::introspect &current, const std::deque<neam::r::introspect> &stack)
  {
    auto rm = current.get_reports();
    if (rm.size() > 0)
    {
      print_function(current);
      neam::cr::out.log() << "  list of reports:" << std::endl;
      for (const auto &re : rm)
      {
        neam::cr::out.log() << "  -" << re.first << ":"<< std::endl;
        for (const neam::r::reason &r : re.second)
        {
          neam::cr::out.log() << neam::cr::internal::end_header
                              << "     -" << r.type << ": '" << r.message << "'" << neam::cr::newline
                              << "      | file: " << (r.file.size() ? r.file : "[---]") << " line " << r.line << neam::cr::newline
                              << "      | number of time reported: " << r.hit << neam::cr::newline
                              << "      |  from: " << std::put_time(std::localtime((const time_t *)&r.initial_timestamp), "%F %T") << neam::cr::newline
                              << "      |__to:   " << std::put_time(std::localtime((const time_t *)&r.last_timestamp), "%F %T") << std::endl;
        }
      }
      print_callstack(stack);
      neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
    }
  });
  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
  return 0;
}
