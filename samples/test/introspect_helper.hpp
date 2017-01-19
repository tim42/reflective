//
// file : introspect_helper.hpp
// in : file:///home/tim/projects/reflective/samples/test/introspect_helper.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 31/01/2016 23:15:34
//
//
// Copyright (C) 2016 Timothée Feuillet
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#ifndef __N_19001464581807975617_1831732230__INTROSPECT_HELPER_HPP__
# define __N_19001464581807975617_1831732230__INTROSPECT_HELPER_HPP__

#include <set>
#include <iostream>
#include <iomanip>

#define N_R_XBUILD_COMPAT // I want a file that works across multiple builds / compilers

#include <reflective/reflective.hpp>

namespace sample
{

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

  void output_introspect_vector(const char *text, const std::vector<neam::r::introspect> &vi)
  {
    if (vi.size())
    {
      std::cout << "  " << text;
      std::set<std::string> names;
      size_t line_count = 0;
      for (const neam::r::introspect &i : vi)
      {
        const std::string &name = i.get_pretty_name();
        if (!names.count(name))
        {
          if (line_count > 5)
          {
            std::cout << "\n   ";
            line_count = 0;
          }
          else if (line_count++)
            std::cout << ", ";
          std::cout << " " << name;
          names.emplace(name);
        }
      }
      std::cout << "\n";
    }
  }

  // example on how you can use reflective to create reports on functions
  void introspect_function(const char *const name)
  {
    try
    {
      neam::r::introspect fnc_introspect(name);

      if (fnc_introspect.is_faithfull())
      {
        std::cout << fnc_introspect.get_pretty_name() << " report:" << "\n"
                  << "  call count: " << fnc_introspect.get_call_count() << "\n"
                  << "  fail ratio: " << (100 * fnc_introspect.get_failure_ratio()) << "% [ " << size_t(float(fnc_introspect.get_call_count()) * fnc_introspect.get_failure_ratio()) << " ]"
                  << std::endl;
        auto tminfo = get_time(fnc_introspect.get_average_self_duration());
        std::cout << "  avg self time: " << tminfo.first << " " << tminfo.second << "seconds" << std::endl;
        tminfo = get_time(fnc_introspect.get_average_duration());
        std::cout << "  avg global time: " << tminfo.first << " " << tminfo.second << "seconds" << std::endl;

        // output a called by list
        output_introspect_vector("called by:", fnc_introspect.get_caller_list());

        // output a called by list
        output_introspect_vector("calls:", fnc_introspect.get_callee_list());

        const auto &measure_point_map = fnc_introspect.get_measure_point_map();
        if (measure_point_map.size())
        {
          std::cout << "  measure points:\n";
          for (const auto &it : measure_point_map)
          {
            auto mpetm = get_time(it.second.value);
            std::cout << "    " << it.first << ": " << mpetm.first << ' ' << mpetm.second << "seconds\n";
          }
        }

        // output the last error
        std::vector<neam::r::reason> last_error = fnc_introspect.get_failure_reasons(1);
        if (last_error.size())
        {
          neam::r::reason r = last_error.back();
          std::cout << "  last error: " << r.type << ": '" << r.message << "'" << "\n"
                    << "  | file: " << (r.file.size() ? r.file : "[---]") << " line " << r.line << "\n"
                    << "  | number of time reported: " << r.hit << "\n"
                    << "  |  from: " << std::put_time(std::localtime((const time_t *)&r.initial_timestamp), "%F %T") << "\n";
          std::cout << "  |__to:   " << std::put_time(std::localtime((const time_t *)&r.last_timestamp), "%F %T") << std::endl;
        }
      }
    }
    catch (...) {}
  }
} // namespace sample

#endif /*__N_19001464581807975617_1831732230__INTROSPECT_HELPER_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

