//
// file : config.hpp
// in : file:///home/tim/projects/reflective/reflective/config.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 31/01/2016 23:05:18
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

#ifndef __N_1434750452659720710_1253467611__CONFIG_HPP__
# define __N_1434750452659720710_1253467611__CONFIG_HPP__

#include <cstddef>

namespace neam
{
  namespace r
  {
    namespace conf
    {
      extern bool monitor_self_time/* = false*/;
      extern bool monitor_global_time/* = false*/;
      extern const char *out_file/* = "./.out.nr"*/;
      extern bool disable_auto_save/* = false*/; ///< \brief Whether or not the last function_call on the stack will trigger a save at its destruction
      extern bool cleanup_on_crash/* = true*/; ///< \brief Whether or not, when a signal is caught, reflective will cleanup its stack(s).
                                               ///         Enabling this option allows reflective to have consistant informations about the crash
                                               ///         and correct timings. (if false, the callstack of the crash will not be available).
                                               /// \note This may result in multiple saves. If you have issues with spurious crashs / data corruption,
                                               ///       you may want to disable this feature.

      extern bool watch_uncaught_exceptions; ///< \brief Whether or not the function_call destructor will watch for uncaught exception (default is true).
                                             /// \note uncaught exception are only reported in the first function that encounter them. The file/line of
                                             ///       the failure reason is the file/line where the function_call object is created for this function (if available)
                                             /// \note Since this is C++11, this feature is quite broken in some cases. This is fixed in C++17.
      extern bool integrate_with_n_debug; ///< \brief Whether or not reflective should also report errors reported by neam::debug::on_error. (Default is true).
      extern bool print_fails_to_stdout; ///< \brief Whether or not reflective should print function_call::fail() to stdout. (Default is false).
      extern bool print_reports_to_stdout; ///< \brief Whether or not reflective should print function_call::report() to stdout. (Default is false).

      extern bool sliding_average; ///< \brief If the average will be a sliding average or a "true" average. The default is true (sliding average).
      extern size_t past_average_weight; ///< \brief The maximum weight for past value (only used when the sliding average is activated).

      extern float progression_min_factor; ///< \brief The minimum variation factor in an average variable for it to be pushed in the progression vector. Default is x10.
      extern size_t max_progression_entries; ///< \brief The maximum entries in the progression vectors (default is somewhere between 25 and 50)

      extern long max_stash_count; ///< \brief Default is somewhere around 5. It's the maximum number of stashes to keep. -1 mean no limit. Minimum is 2.
    } // namespace conf
  } // namespace r
} // namespace neam

#endif /*__N_1434750452659720710_1253467611__CONFIG_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

