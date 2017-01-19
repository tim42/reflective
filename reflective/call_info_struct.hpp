//
// file : call_struct.hpp
// in : file:///home/tim/projects/reflective/reflective/call_struct.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 28/01/2016 17:17:55
//
//
// // Copyright (C) 2016 Timothée Feuillet
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

#ifndef __N_54570708560285988_386345818__CALL_STRUCT_HPP__
# define __N_54570708560285988_386345818__CALL_STRUCT_HPP__

#include <cstdint>
#include <deque>
#include <map>

#include "reason.hpp"
#include "func_descriptor.hpp"
#include "type.hpp"

namespace neam
{
  namespace r
  {
    /// \brief Information about a measure point
    struct measure_point_entry
    {
      size_t hit_count = 0; /// \brief Number of time the measure_point has been "hit"
      double value = 0;     /// \brief The average value
    };

    namespace internal
    {
      /// \brief Hold some information about a called function
      struct call_info_struct
      {
#ifdef _MSC_VER
        call_info_struct(const func_descriptor& fd) : descr(fd) {}
#endif
        func_descriptor descr; ///< \brief The function descriptor object

        // ----- //

        size_t call_count = 0; ///< \brief The number of call of this function
        size_t fail_count = 0; ///< \brief The number of time that function failed

        double average_self_time = 0; ///< \brief The average time consumed by the function and only this function
        size_t average_self_time_count = 0; ///< \brief Number of time the self_time has been monitored
        double average_global_time = 0; ///< \brief The average time consumed by the whole function call (including all its children)
        size_t average_global_time_count = 0; ///< \brief Number of time the global_time has been monitored

        std::deque<reason> fails = std::deque<reason>(); ///< \brief Holds all the past fails reasons

        std::map<std::string, std::deque<reason>> reports = decltype(reports)(); /// \brief Hold reports

        // GCC does not like std::map<std::string, measure_point_entry> measure_points = std::map<std::string, measure_point_entry>()
        std::map<std::string, measure_point_entry> measure_points = decltype(measure_points)(); /// \brief Holds informations about measure points
      };
    } // namespace internal
  } // namespace r
} // namespace neam

#endif /*__N_54570708560285988_386345818__CALL_STRUCT_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

