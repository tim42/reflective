//
// file : reason.hpp
// in : file:///home/tim/projects/reflective/reflective/reason.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 27/01/2016 15:58:00
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

#ifndef __N_180725618345790760_1525595175__REASON_HPP__
# define __N_180725618345790760_1525595175__REASON_HPP__

#include <cstring>

namespace neam
{
  namespace r
  {
    /// \brief A failure reason
    struct reason
    {
      const char *type;
      const char *message = nullptr;

      const char *file = nullptr;
      size_t line = 0;

      // --- //

      size_t hit = 1; ///< \brief Do not touch this.
      size_t initial_timestamp = 0; ///< \brief Do not touch this.
      size_t last_timestamp = 0; ///< \brief Do not touch this.

      /// \brief Used to append the filename & the line to a predefined reason (like those below)
      /// \see REASON_INFO
      /// \code
      /// self_call.fail(neam::r::out_of_memory_reason(REASON_INFO));
      ///  // or
      /// self_call.fail(neam::r::out_of_memory_reason(N_REASON_INFO, "failed to allocate memory for my_huge_array"));
      /// \endcode
      constexpr reason operator() (const char *_file, size_t _line) const { return reason { type, message, _file, _line }; }
      constexpr reason operator() (const char *_file, size_t _line, const char *_message) const { return reason { type, _message, _file, _line }; }

      /// \brief Equality operator
      constexpr bool operator == (const reason &o) const
      {
        return o.line == this->line
               && ((o.file == nullptr && o.file == this->file) || !strcmp(o.file ? o.file : "", this->file ? this->file : ""))
               && ((o.message == nullptr && o.message == this->message) || !strcmp(o.message ? o.message : "", this->message ? this->message : ""))
               && ((o.type == nullptr && o.type == this->type) || !strcmp(o.type ? o.type : "", this->type ? this->type : ""));
      }

      /// \brief Inequality operator
      constexpr bool operator != (const reason &o) const
      {
        return !(*this == o);
      }
    };

/// \brief To be used to fill the operator() automatically
/// \code
/// return self_call.fail(neam::r::out_of_memory_reason(N_REASON_INFO), nullptr);
///  // or
/// return self_call.fail(neam::r::out_of_memory_reason(N_REASON_INFO, "failed to allocate memory for my_huge_array"), nullptr);
/// \endcode
#define N_REASON_INFO __FILE__, __LINE__

    // predefined reasons
    static constexpr reason out_of_memory_reason = reason {"out of memory"};
    static constexpr reason bad_allocation_reason = reason {"bad allocation"};
    static constexpr reason syscall_failed_reason = reason {"syscall failed"};
    static constexpr reason exception_reason = reason {"exception caught"};
    static constexpr reason assert_reason = reason {"assertion is false"};

    static constexpr reason file_not_found_reason = reason {"file not found"};
    static constexpr reason error_reason = reason {"error"}; // hum.

    static constexpr reason segfault_reason = reason {"SIGSEGV"};
    static constexpr reason abort_reason = reason {"SIGABRT"};
    static constexpr reason floating_point_exception_reason = reason {"SIGFPE"};
    static constexpr reason illegal_instruction_reason = reason {"SIGILL"};
    static constexpr reason unknown_signal_reason = reason {"unknown signal"};

    static constexpr reason unknown_reason = reason {"unknown cause"};
    static constexpr reason should_not_happen_reason = reason {"that should not happen"};

    static constexpr reason lazy_programmer_reason = reason {"lazy programmer"};
  } // namespace r
} // namespace neam

#endif /*__N_180725618345790760_1525595175__REASON_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

