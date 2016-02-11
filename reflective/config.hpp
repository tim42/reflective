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
      extern bool disable_auto_save/* = false*/;

      extern bool sliding_average; ///< \brief If the average will be a sliding average or a "true" average. The default is true.
      extern size_t past_average_weight; ///< \brief The maximum weight for past value (only used when the sliding average is activated).

      extern float progression_min_factor; ///< \brief The minimum variation factor in an average variable for it to be pushed in the progression vector. Default is x10.
      extern size_t max_progression_entries; ///< \brief The maximum entries in the progression vectors (default is somewhere between 25 and 50)

      extern bool use_json_backend; ///< \brief Use the json backend instead of the default (binary) one. The default is to NOT use it. (the default backend is faster, better, ... than the json one).
    } // namespace conf
  } // namespace r
} // namespace neam

#endif /*__N_1434750452659720710_1253467611__CONFIG_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

