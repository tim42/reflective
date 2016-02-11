//
// file : backtrace.hpp
// in : file:///home/tim/projects/reflective/reflective/backtrace.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 27/01/2016 17:15:28
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

#ifndef __N_417152675875903797_1694273986__BACKTRACE_HPP__
# define __N_417152675875903797_1694273986__BACKTRACE_HPP__

#include <cstdint>
#include <cstddef>
#include <vector>
#include <deque>

namespace neam
{
  namespace r
  {
    /// \brief Holds a progression entry
    struct duration_progression
    {
      size_t timestamp; ///< \brief When the progression "has occured"
      double value;     ///< \brief The value at the timestamp time
    };

    namespace internal
    {
      /// \brief Hold an entry
      struct stack_entry
      {
        const size_t self_index; ///< \brief Hold the index of the stack_entry structure
        const size_t stack_index; ///< \brief Hold the current stack index (index in the callgraph deque)
        const size_t call_structure_index; ///< \brief Holds the index of the call struct

        const size_t parent; ///< \brief Parent index
        // ----- //
        std::vector<size_t> children = std::vector<size_t>(); ///< \brief Children indexes

        size_t hit_count = 1; ///< \brief The number of time the entry has been hit
        size_t fail_count = 0; ///< \brief The number of time that function failed

        double average_self_time = 0; ///< \brief The average time consumed by the function and only this function
        size_t average_self_time_count = 0; ///< \brief Number of time the self_time has been monitored
        std::deque<duration_progression> self_time_progression = std::deque<duration_progression>(); ///< \brief Hold the progression of the self_time average
        double average_global_time = 0; ///< \brief The average time consumed by the whole function call (including all its children)
        size_t average_global_time_count = 0; ///< \brief Number of time the global_time has been monitored
        std::deque<duration_progression> global_time_progression = std::deque<duration_progression>();


        // ----- //


        /// \brief push (or increment hit_count) into children a new call_info_struct.
        /// \note If not already in children, it will create a new stack_entry and insert its index at the end of children
        /// \return the reference of the stack_entry corresponding to the call_info_struct
        stack_entry &push_children_call_info(size_t call_info_struct_index);

        /// \brief Unlike push_children_call_info(), this method won't create anything / modify anithing
        /// It will simply lookup in the children array for a stack_entry for the call_info_struct_index call_info_struct
        /// If nothing is found, it returns nullptr
        stack_entry *get_children_stack_entry(size_t call_info_struct_index) const;

        /// \brief Returns a children stack_entry index corresponding to a given call_info_struct index
        /// \return -1 if nothing found
        long get_children_stack_entry_index(size_t call_info_struct_index) const;

        /// \brief Start a stack
        static stack_entry &initial_get_stack_entry(size_t call_info_struct_index);
        /// \brief End a stack
        static void dispose_initial();
      };
    } // namespace internal
  } // namespace r
} // namespace neam

#endif /*__N_417152675875903797_1694273986__BACKTRACE_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

