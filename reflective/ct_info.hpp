//
// file : ct_info.hpp
// in : file:///home/tim/projects/reflective/reflective/ct_info.hpp
//
// created by : Timothée Feuillet on linux.site
// date: 20/01/2016 18:24:55
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

#ifndef __N_5941161391869237784_784459826__CT_INFO_HPP__
# define __N_5941161391869237784_784459826__CT_INFO_HPP__

#include "tools/ct_string.hpp"

namespace neam
{
  namespace r
  {
    /// \brief some informations about the compilation (yes, some really basic ones)
    namespace ct
    {
      constexpr neam::string_t build_date = __DATE__ " " __TIME__;
      constexpr neam::string_t build_file = __BASE_FILE__;
    } // namespace ct
  } // namespace r
} // namespace neam

#endif /*__N_5941161391869237784_784459826__CT_INFO_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 


