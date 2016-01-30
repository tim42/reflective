//
// file : id_gen.hpp
// in : file:///home/tim/projects/reflective/reflective/id_gen.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 30/01/2016 21:33:23
//
//
// Copyright (C) 2014 Timothée Feuillet
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

#ifndef __N_147554771727415468_1812229998__ID_GEN_HPP__
# define __N_147554771727415468_1812229998__ID_GEN_HPP__

#include <stdint.h>
#include <tools/ct_string.hpp>

namespace neam
{
  namespace r
  {
    namespace internal
    {
      void __addr__(); // empty, implemented in function_call.cpp

      template<typename Class, typename Ret, typename... Args>
      static inline uint32_t hash_from_ptr(Ret(Class::*ptr)(Args...))
      {
        const long addr = reinterpret_cast<long>(&__addr__);
        Ret(*ptr2)(Class *, Args...);
        ptr2 = reinterpret_cast<Ret( *)(Class *, Args...)>(ptr);
        return uint32_t(addr - reinterpret_cast<long>(reinterpret_cast<void *>(ptr2)));
      }
      template<typename Ret, typename... Args>
      static inline uint32_t hash_from_ptr(Ret(*ptr)(Args...))
      {
        const long addr = reinterpret_cast<long>(&__addr__);
        return (uint32_t(addr - reinterpret_cast<long>((void *)ptr))) & 0xFFFE;
      }

      /// \brief This hash is guaranteed to be and consistent across program launch
      /// \note This hash is quite fast AND \b NOT INTENDED TO BE SECURE. Its sole purpose is to allow a quite faster equality test for strings
      /// \note This hash is made to be run at build time (and all decent compiler will not execute it at runtime)
      /// \note This is C++14
      constexpr inline uint32_t hash_from_str(const char *const string)
      {
        const size_t len = neam::ct::strlen(string);
        uint32_t hash = 0x005A55AA | ((len % 256) << 24); // first byte is (len % 255), and only the remaining 3 bytes are for the hash

        for (size_t i = 0; i < len; ++i)
        {
          uint8_t byte = static_cast<uint8_t>(string[i]);

          // rotate
          byte = (byte << (i % 8)) | (byte >> (8 - (i % 8)));

          // xor it in the hash. Nothing more...
          hash ^= (byte << (8 * (i % 3)));
        }

        return hash | 1;
      }

      /// \brief Generate an id for a string / ptr
      template<typename Ret, typename... Args>
#ifdef N_R_XBUILD_COMPAT
      constexpr inline
#endif
      uint32_t generate_id(const char *const string, Ret (*ptr)(Args...))
      {
#ifdef N_R_XBUILD_COMPAT
        (void)ptr;
        return hash_from_str(string);
#else
        (void)string;
        return hash_from_ptr(ptr);
#endif
      }
      /// \brief Generate an id for a string / ptr
      template<typename Ret, typename Class, typename... Args>
      constexpr inline uint32_t generate_id(const char *const string, Ret (Class::*)(Args...))
      {
        return hash_from_str(string);
      }
    } // namespace internal
  } // namespace r
} // namespace neam

#endif /*__N_147554771727415468_1812229998__ID_GEN_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

