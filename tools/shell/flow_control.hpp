//
// file : flow_control.hpp
// in : file:///home/tim/projects/reflective/tools/shell/flow_control.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 11/02/2016 22:25:31
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

#ifndef __N_1196918432140149232_201606371__FLOW_CONTROL_HPP__
# define __N_1196918432140149232_201606371__FLOW_CONTROL_HPP__

#include <string>
#include "variable_stack.hpp"

namespace neam
{
  namespace r
  {
    namespace shell
    {
      /// \brief flow control related exception
      /// (yes, I use exception for that)
      struct flow_control { std::string name; };

      /// \brief return flow control
      struct return_flow_control : public flow_control
      {
        explicit return_flow_control(int _retval = 0) : flow_control{"return"}, retval(_retval) {};

        int retval = 0;
      };


      /// \brief raii contexts
      class raii_var_context
      {
        public:
          /// \see variable_stack::push_context()
          raii_var_context(variable_stack &_vs, bool argument_list = true, bool copy_on_write = false) : vs(&_vs)
          {
            vs->push_context(argument_list, copy_on_write);
          }

          /// \brief Allow late-initialization
          raii_var_context() : vs(nullptr) {}

          ~raii_var_context()
          {
            if (vs)
              vs->pop_context();
          }

          /// \brief late initialization
          void handle_context(variable_stack &_vs) { vs = &_vs; }

        private:
          variable_stack *vs;
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_1196918432140149232_201606371__FLOW_CONTROL_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

