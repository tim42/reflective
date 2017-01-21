
#include <fstream>

#include <reflective/reflective.hpp> // The reflective header
#include <tools/logger/logger.hpp>   // Just to set the logger in debug mode

#include "graph_walker.hpp"

// first is the time, second the unit
std::pair<double, const char *> get_time(double sec_time)
{
  double rtime = sec_time;

  const char *tab[] = {"", "milli", "micro", "nano"};
  size_t idx = 0;

  for (; idx < (sizeof(tab) / sizeof(tab[0]) - 1) && rtime < 1.; ++idx)
  {
    rtime *= 1000;
  }

  return std::make_pair(rtime, tab[idx]);
}

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

  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
  neam::cr::out.log() << "report for " << argv[1] << ": " << std::endl;

  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
  neam::cr::out.log() << "ERRORS: " << std::endl;
  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

  neam::rtools::graph_walker::walk([](const neam::r::introspect &current, const std::deque<neam::r::introspect> &stack)
  {
    if (current.get_failure_count() > 0)
    {
      {
        const auto &fd = current.get_function_descriptor();
        neam::cr::out.log() << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]:" << std::endl;
      }
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
      neam::cr::out.log() << "  callstack (most recent call last) :" << std::endl;
      for (const neam::r::introspect &it : stack)
      {
        const auto &fd = it.get_function_descriptor();
        neam::cr::out.log() << "    " << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]:" << std::endl;
      }
      neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
    }
  });


  neam::cr::out.log() << std::endl;
  neam::cr::out.log() << "MEASURE POINTS: " << std::endl;
  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;

  neam::rtools::graph_walker::walk([](const neam::r::introspect &current, const std::deque<neam::r::introspect> &stack)
  {
    auto mpm = current.get_measure_point_map();
    if (mpm.size() > 0)
    {
      {
        const auto &fd = current.get_function_descriptor();
        neam::cr::out.log() << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]:" << std::endl;
      }
      neam::cr::out.log() << "  list of measure points:" << std::endl;
      for (const auto &m : mpm)
      {
        auto tm = get_time(m.second.value);
        neam::cr::out.log() << "   - " << m.first << ": " << tm.first << " " << tm.second << "seconds [hit " << m.second.hit_count << " time]" << std::endl;
      }
      neam::cr::out.log() << "  callstack (most recent call last) :" << std::endl;
      for (const neam::r::introspect &it : stack)
      {
        const auto &fd = it.get_function_descriptor();
        neam::cr::out.log() << "    " << fd.pretty_name << " [" << fd.file << ": " << fd.line << "]:" << std::endl;
      }
      neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
    }
  });
  neam::cr::out.log() << "---------------------------------------------------------------------------------------------" << std::endl;
  return 0;
}
