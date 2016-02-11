//
// file : id_gen.hpp
// in : file:///home/tim/projects/reflective/reflective/id_gen.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 30/01/2016 21:33:23
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

#ifndef __N_147554771727415468_1812229998__ID_GEN_HPP__
# define __N_147554771727415468_1812229998__ID_GEN_HPP__

#include <stdint.h>
#include <tools/ct_string.hpp>

/// \file id_gen.hpp
/// \brief Generate IDs from things
/// A simple note about why I need a ID generator and not typeid. typeid hashes are not unique AND not consistent
/// between two invocation of the same program. (There **absolutely** no support for different builds by different compilers).
/// But reflective being what it is, I needed a ID generator for generating consistent ID and the easiest way to do so was to
/// compute a hash from a string. Hashes being hashes, they are not unique, but that was allowing a faster string/string comparison
/// (even if it make the program calling 10 time strcmp for 10,000 functions, that was enough low to not use a complex structure like std::map).
/// Moreover the hash can be computed at compile time and there is a (simple) cache system for memoizing results. So in term of complexity,
/// for a program consisting of \e n functions called \e m time, you will search n time the DB consisting of n entries (complexity O(n²))
/// but for a big enough \e m, you'll get an amortized complexity of O(1) because of the cache (and searching the cache consist of a single if()).
/// That's why I set-up this system.
///
/// A possibility to generate unique hashes could be to use __builtin_return_address (gcc, clang) (or _ReturnAddress, msvc) in a non-inlineable function
///
/// And why not using std::{unordered_,}map from the start ? I needed to have an indexable DB with the guarantee that the address
/// will NEVER be changed by insertions.

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
        const size_t len = neam::ct::safe_strlen(string);
        uint32_t hash = 0x005A55AA | ((len % 256) << 24); // first byte is (len % 255), and only the remaining 3 bytes are for the hash

        for (size_t i = 0; i < len; ++i)
        {
          uint8_t byte = static_cast<uint8_t>(string[i]);

          // rotate
          byte = (byte << (i % 8)) | (byte >> (8 - (i % 8)));

          // xor it in the hash.
          hash ^= (byte << (8 * (i % 3)));
        }

        return hash | 1;
      }
#define N_R_XBUILD_COMPAT
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

      /// \brief Generate an id for a string / ptr
      template<typename Type>
      constexpr inline uint32_t generate_id(const char *const string, Type)
      {
        return hash_from_str(string);
      }
      template<>
      constexpr inline uint32_t generate_id<const char *>(const char *const, const char *const other_string)
      {
        return hash_from_str(other_string);
      }
    } // namespace internal
  } // namespace r
} // namespace neam

#endif /*__N_147554771727415468_1812229998__ID_GEN_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

