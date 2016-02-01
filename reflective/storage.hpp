//
// file : storage.hpp
// in : file:///home/tim/projects/reflective/reflective/storage.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 26/01/2016 18:16:09
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

#ifndef __N_1052045765701734561_8562785__STORAGE_HPP__
# define __N_1052045765701734561_8562785__STORAGE_HPP__

#include <deque>
#include <mutex>
#include <stack_entry.hpp>
#include <call_info_struct.hpp>

namespace neam
{
  namespace cr
  {
    class persistence;
  } // namespace cr

  namespace r
  {
    class function_call;

    namespace internal
    {
      /// \brief The type of mutex used by the data class
      using mutex_type = std::mutex;

      /// \brief Holds some information about things that happen somewhere
      class data
      {
        public: // methods

        public: // attributes
          size_t launch_count = 1;

          mutex_type lock; // only used if global == this (else, the structure is per-thread, no need to lock)
          std::deque<call_info_struct> func_info; // protected by the mutex lock

          std::deque<std::deque<stack_entry>> callgraph; // only insertions &lookups are permitted, protected by the mutex lock

        private:
          /// \brief Called after the deserialization (thus the name)
          void post_deserialization()
          {
            new (&lock) mutex_type(); // placement new for lock
          }

        private:
          friend class neam::cr::persistence;
      };

      /// \brief This is a purely thread local thing
      struct thread_local_data
      {
        function_call *top = nullptr;
      };

      /// \brief Get the thread-local data
      thread_local_data *get_thread_data();
      /// \brief Get the global data
      data *get_global_data();

      /// \brief Do not use directly, please use get_call_info_struct() instead
      /// This version is the one that perform the search. get_call_info_struct() will look in a cache to see if the structure has already been found/created
      /// \see get_call_info_struct
      call_info_struct &_get_call_info_struct(uint32_t hash, const char *const name, const char *const pretty_name, long &index);
      /// \brief Return the call_info_struct at a given index
      /// \see get_call_info_struct
      call_info_struct &get_call_info_struct_at_index(long index);

      /// \brief This function will not create the structure if nothing is found
      call_info_struct *_get_call_info_struct_search_only(uint32_t hash, const char *const name, const char *const pretty_name, long &index);

      /// \brief Get (or create) the call_info_struct for a given hash/name + possibly set the pretty_name attribute
      /// \note we use some cache here, but it will work most probably work on a per-translation unit basis (except, probably, with some compiler flags to merge globals/statics)
      /// Nevertheless this will increase the speed a little, but will also increase the final executable size (of at least 8bit per monitored function)
      template<typename FuncType, FuncType Func>
      call_info_struct &get_call_info_struct(uint32_t hash, const char *const name, const char *const pretty_name = nullptr, size_t *_index = nullptr, bool search_only = false)
      {
        static long index = -1; // This is an invalid index

        // Both perform the search-or-create and setup the index
        if (index == -1)
        {
          call_info_struct *ret;
          if (!search_only)
            ret = &_get_call_info_struct(hash, name, pretty_name, index);
          else
          {
            ret = _get_call_info_struct_search_only(hash, name, pretty_name, index);
            if (!ret)
              throw std::runtime_error("reflective: could not found the requested call_info_struct");
          }
          if (_index)
            *_index = index;
          return *ret;
        }
        if (_index)
          *_index = index;
        return get_call_info_struct_at_index(index);
      }

      /// \brief Get (or create) the call_info_struct for a given hash/name + possibly set the pretty_name attribute
      /// \note we use some cache here, but it will work most probably work on a per-translation unit basis (except, probably, with some compiler flags to merge globals/statics)
      /// Nevertheless this will increase the speed a little, but will also increase the final executable size (of at least 8bit per monitored function)
      template<typename FuncType>
      call_info_struct &get_call_info_struct(uint32_t hash, const char *const name, const char *const pretty_name = nullptr, size_t *_index = nullptr, bool search_only = false)
      {
        static long index = -1; // This is an invalid index

        // Both perform the search-or-create and setup the index
        if (index == -1 || std::is_same<FuncType, void>::value)
        {
          call_info_struct *ret;
          if (!search_only)
            ret = &_get_call_info_struct(hash, name, pretty_name, index);
          else
          {
            ret = _get_call_info_struct_search_only(hash, name, pretty_name, index);
            if (!ret)
              throw std::runtime_error("reflective: could not found the requested call_info_struct");
          }
          if (_index)
            *_index = index;
          return *ret;
        }
        if (_index)
          *_index = index;
        return get_call_info_struct_at_index(index);
      }
    } // namespace internal

    /// \brief Write everything to the disk
    void sync_data_to_disk(const std::string &file);

    /// \brief Load from the disk
    void load_data_from_disk(const std::string &file);

  } // namespace r
} // namespace neam

#endif /*__N_1052045765701734561_8562785__STORAGE_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

