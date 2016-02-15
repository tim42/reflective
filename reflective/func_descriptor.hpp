//
// file : func_descriptor.hpp
// in : file:///home/tim/projects/reflective/reflective/func_descriptor.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 03/02/2016 16:33:54
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

#ifndef __N_4595750722075452917_1312795707__FUNC_DESCRIPTOR_HPP__
# define __N_4595750722075452917_1312795707__FUNC_DESCRIPTOR_HPP__

#include <cstddef>
#include <cstdint>

#include <cstring>

#include "id_gen.hpp"

namespace neam
{
  namespace r
  {
    /// \brief Describe a function
    /// This is used to store/retrieve information to/from the function DB
    struct func_descriptor
    {
      const char *name = nullptr; ///< \brief User access name
      const char *pretty_name = nullptr; ///< \brief print name

      const char *file = nullptr; ///< \brief The zip code
      size_t line = 0; ///< \brief The line of the fiel

      const char *key_name = nullptr; ///< \brief Used as unique ID to compare
      uint32_t key_hash = 0; ///< \brief Used to fast, compare the unique ID

      /// \brief Comparison function
      bool operator == (const func_descriptor &other) const
      {
        if (other.key_hash && this->key_hash && (this->key_hash & 0x1) == 0) // ultra fast hash/hash comparison
          return other.key_hash == this->key_hash; // The hash guaranteed to be unique if the first bit is not set.
        if (other.key_name && this->key_name) // key/key comparison
        {
          if (other.key_hash && this->key_hash && other.key_hash != this->key_hash) // the hash mismatch = no the same key_name
            return false;
          return !strcmp(other.key_name, this->key_name);
        }

        if (other.file && this->file && other.line && this->line)
          return other.line == this->line && !strcmp(other.file, other.file);

        // non-unique comparisons
        if (this->pretty_name && other.pretty_name) // non-unique
          return !strcmp(other.pretty_name, this->pretty_name);
        if (this->name && other.name) // slower, non-unique...
          return !strcmp(other.name, this->name);
        return false; // Well...
      }

      /// \brief Comparison operator
      bool operator != (const func_descriptor &other) const
      {
        return !(*this == other);
      }
    };
  } // namespace r
} // namespace neam

#endif /*__N_4595750722075452917_1312795707__FUNC_DESCRIPTOR_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 
