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
#ifdef _MSC_VER
      reason() = default;
      explicit reason(const std::string &_type, const std::string &_message = std::string(), const std::string &_file = std::string(), size_t _line = 0)
       : type(_type), message(_message), file(_file), line(_line)
      {}
      explicit reason(const std::string &_type, const std::string &_message, const std::string &_file, size_t _line, size_t _hit, size_t _it, size_t _lt)
       : type(_type), message(_message), file(_file), line(_line), hit(_hit), initial_timestamp(_it), last_timestamp(_lt)
      {}
   #endif
      std::string type = std::string();
      std::string message = std::string();

      std::string file = std::string();
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
      reason operator() (const std::string &_file, size_t _line) const { return reason { type, message, _file, _line }; }
      reason operator() (const std::string &_file, size_t _line, const std::string &_message) const { return reason { type, _message, _file, _line }; }

      /// \brief Equality operator
      bool operator == (const reason &o) const
      {
        return o.line == this->line
               && (o.file == this->file)
               && (o.message == this->message)
               && (o.type == this->type);
      }

      /// \brief Inequality operator
      bool operator != (const reason &o) const
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
    static const reason out_of_memory_reason = reason {"out of memory"};
    static const reason bad_allocation_reason = reason {"bad allocation"};
    static const reason syscall_failed_reason = reason {"syscall failed"};
    static const reason exception_reason = reason {"exception"};
    static const reason assert_reason = reason {"assertion"};

    static const reason file_not_found_reason = reason {"file not found"};
    static const reason error_reason = reason {"error"}; // hum.

    static const reason segfault_reason = reason {"SIGSEGV"};
    static const reason abort_reason = reason {"SIGABRT"};
    static const reason floating_point_exception_reason = reason {"SIGFPE"};
    static const reason illegal_instruction_reason = reason {"SIGILL"};
    static const reason keyboard_interrupt_reason = reason {"SIGINT"};
    static const reason unknown_signal_reason = reason {"unknown signal"};

    static const reason unknown_reason = reason {"unknown cause"};
    static const reason should_not_happen_reason = reason {"that should not happen"};

    static const reason lazy_programmer_reason = reason {"lazy programmer"};
  } // namespace r
} // namespace neam

#endif /*__N_180725618345790760_1525595175__REASON_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

