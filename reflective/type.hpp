//
// file : type.hpp
// in : file:///home/tim/projects/reflective/reflective/type.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 31/01/2016 15:08:13
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

#ifndef __N_865426894276070490_1131732371__TYPE_HPP__
# define __N_865426894276070490_1131732371__TYPE_HPP__

#include "tools/spinlock.hpp"

namespace neam
{
  namespace r
  {
    namespace internal
    {
      /// \brief The type of mutex used by classes
      using mutex_type = neam::spinlock;

      /// \brief only present for type deduction
      template<typename T> struct type { using t = T; };

      /// \brief only present for having a cache
      template<uint32_t Hash, uint64_t Line> struct file_type {};
    } // namespace internal
  } // namespace r
} // namespace neam

#endif /*__N_865426894276070490_1131732371__TYPE_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

