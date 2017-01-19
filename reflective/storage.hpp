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
#include <set>
#include "stack_entry.hpp"
#include "call_info_struct.hpp"
#include "type.hpp"

namespace neam
{
  namespace cr
  {
    struct persistence;
  } // namespace cr

  namespace r
  {
    class function_call;

    /// \brief This is internal data. If you touch anything from here,
    /// please expect reflective to either crash, be corrupted or simply doesn't
    /// work correctly.
    namespace internal
    {
      /// \brief Holds some information about things that happen somewhere
      class data
      {
        public: // methods
          data(const data &o)
          : launch_count(o.launch_count), func_info(o.func_info),
            callgraph(o.callgraph), name(o.name), timestamp(o.timestamp)
          {}
          data() = default;
          ~data() = default;

        public: // attributes
          size_t launch_count = 1;

          mutex_type lock; // only used if global == this (else, the structure is per-thread, no need to lock)
          std::deque<call_info_struct> func_info; // protected by the mutex lock

          std::deque<std::deque<stack_entry>> callgraph; // only insertions &lookups are permitted, protected by the mutex lock

          // when stashed only //
          std::string name;
          long timestamp;

#ifndef _MSC_VER
        private:
#endif
          /// \brief Called after the deserialization (thus the name)
          void post_deserialization()
          {
            new (&lock) mutex_type(); // placement new for lock
          }

        private:
          friend struct neam::cr::persistence;
      };

      /// \brief This is a purely thread local thing
      struct thread_local_data
      {
        thread_local_data();
        ~thread_local_data();

        function_call *top = nullptr;
      };

      /// \brief Get the thread-local data
      thread_local_data *get_thread_data();
      /// \brief Get the global data
      data *get_global_data();

      /// \brief Return the local data from all threads
      std::set<thread_local_data *> &get_all_thread_data();

      /// \brief Cleanup currently active function_calls.
      /// If you call it without exiting right after, you may crash or have corrupted
      /// data. (in fact, you will crash as soon as any active function_call will be destructed)
      /// You should also have found a way to stop any active thread.
      void cleanup_reflective_data();

      /// \brief Do not use directly, please use get_call_info_struct() instead
      /// This version is the one that perform the search. get_call_info_struct() will look in a cache to see if the structure has already been found/created
      /// \see get_call_info_struct
      call_info_struct &_get_call_info_struct(const func_descriptor &d, long int &index);
      /// \brief Return the call_info_struct at a given index
      /// \see get_call_info_struct
      call_info_struct &get_call_info_struct_at_index(long index);

      /// \brief This function will not create the structure if nothing is found
      call_info_struct *_get_call_info_struct_search_only(const func_descriptor &d, long &index);

      /// \brief Get (or create) the call_info_struct for a given hash/name + possibly set the pretty_name attribute
      /// \note we use some cache here, but it will work most probably work on a per-translation unit basis (except, probably, with some compiler flags to merge globals/statics)
      /// Nevertheless this will increase the speed a little, but will also increase the final executable size (of at least 8bit per monitored function)
      template<typename FuncType, FuncType Func>
      call_info_struct &get_call_info_struct(const func_descriptor &d, size_t *_index = nullptr, bool search_only = false)
      {
        static long index = -1; // This is an invalid index

        if (index != -1)
        {
          if (_index)
            *_index = index;
          call_info_struct &ret = get_call_info_struct_at_index(index);
          if (ret.descr == d)
            return ret;
        }
        // Both perform the search-or-create and setup the index
        call_info_struct *ret;
        if (!search_only)
          ret = &_get_call_info_struct(d, index);
        else
        {
          ret = _get_call_info_struct_search_only(d, index);
          if (!ret)
            throw std::runtime_error("reflective: could not find the requested call_info_struct");
        }
        if (_index)
          *_index = index;
        return *ret;
      }

      /// \brief Get (or create) the call_info_struct for a given hash/name + possibly set the pretty_name attribute
      /// \note we use some cache here, but it will work most probably work on a per-translation unit basis (except, probably, with some compiler flags to merge globals/statics)
      /// Nevertheless this will increase the speed a little, but will also increase the final executable size (of at least 8bit per monitored function)
      template<typename FuncType>
      call_info_struct &get_call_info_struct(const func_descriptor &d, size_t *_index = nullptr, bool search_only = false)
      {
        static long index = -1; // This is an invalid index

        if (index != -1)
        {
          if (_index)
            *_index = index;
          call_info_struct &ret = get_call_info_struct_at_index(index);
          if (ret.descr == d)
            return ret;
        }

        // Both perform the search-or-create and setup the index
        call_info_struct *ret;
        if (!search_only)
          ret = &_get_call_info_struct(d, index);
        else
        {
          ret = _get_call_info_struct_search_only(d, index);
          if (!ret)
            throw std::runtime_error("reflective: could not find the requested call_info_struct [slow/way]");
        }
        if (_index)
          *_index = index;
        return *ret;
      }
    } // namespace internal

    /// \brief Write everything to the disk
    void sync_data_to_disk(const std::string &file);

    /// \brief Load from the disk
    bool load_data_from_disk(const std::string &file);

    /// \brief Return the number of time the program has been launched
    static inline size_t get_launch_count()
    {
      return internal::get_global_data()->launch_count;
    }

    /// \brief Stash the current data and start a new, clear, one
    /// \note You may want to call this before any function_call has been created
    ///       (or after every single ones have been destructed):
    ///       you may end having corrupted/half merged data...
    void stash_current_data(const std::string &name);

    /// \brief Load a past/archived reflective data from the stash.
    /// \note You may want to call this before any function_call has been created
    ///       (or after every single ones have been destructed):
    ///       you may end having corrupted/half merged data...
    bool load_data_from_stash(const std::string &data_name);

    /// \brief Stash the current data and start a new, clear, one
    /// \param[in] name the name of the new stash, if created. A new stash is created if this parameter
    ///                 is not the same as the name of the last stash.
    /// \return true if the current data has been saved to a new stash.
    /// \see N_DEFAULT_STASH_NAME
    /// \note You may want to call this before any function_call has been created
    ///       (or after every single ones have been destructed):
    ///       you may end having corrupted/half merged data...
    bool auto_stash_current_data(const std::string &name);

    /// \brief Return the names of the entries currently in the stash
    std::vector<std::string> get_stashes_name();

    /// \brief Return the timestamp of the entries currently in the stash
    std::vector<long> get_stashes_timestamp();

    /// \brief Return the index of the active data
    size_t get_active_stash_index();

/// \brief A default stash name that goes well with auto_stash_current_data()
/// \see auto_stash_current_data()
#define N_DEFAULT_STASH_NAME "stash/" __DATE__ " " __TIME__

  } // namespace r
} // namespace neam

#endif /*__N_1052045765701734561_8562785__STORAGE_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

