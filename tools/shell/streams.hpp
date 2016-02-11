//
// file : streams.hpp
// in : file:///home/tim/projects/reflective/tools/shell/streams.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 10/02/2016 21:18:05
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

#ifndef __N_1068911839867312161_1017526335__STREAMS_HPP__
# define __N_1068911839867312161_1017526335__STREAMS_HPP__

#include <deque>
#include <iostream>

namespace neam
{
  namespace r
  {
    namespace shell
    {
      namespace internal
      {
        extern std::iostream nullstream;
      } // namespace internal

      enum class stream : size_t
      {
        stdin = 0,
        stdout = 1,
        stderr = 2,
      };

      struct stream_pack
      {
        std::deque<std::iostream> streams; ///< \brief Holds the streams (was a vector, but iostreams can't be copied)

        inline std::iostream &operator [] (size_t index)
        {
          if (!has(index))
            return internal::nullstream;
          return streams[index];
        }
        inline std::iostream &operator [] (stream sindex)
        {
          return (*this)[static_cast<size_t>(sindex)];
        }

        const std::iostream &operator [] (size_t index) const
        {
          if (!has(index))
            return internal::nullstream;
          return streams[index];
        }
        const std::iostream &operator [] (stream sindex) const
        {
          return (*this)[static_cast<size_t>(sindex)];
        }

        bool has(size_t index) const
        {
          if (streams.size() > index)
            return (streams[index].rdbuf() != nullptr);
          return false;
        }
        bool has(stream sindex) const
        {
          return has(static_cast<size_t>(sindex));
        }
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_1068911839867312161_1017526335__STREAMS_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

