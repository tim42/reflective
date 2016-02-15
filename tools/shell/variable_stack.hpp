//
// file : variable_stack.hpp
// in : file:///home/tim/projects/reflective/tools/shell/variable_stack.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 10/02/2016 16:51:31
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

#ifndef __N_1251124832855879829_1882966428__VARIABLE_STACK_HPP__
# define __N_1251124832855879829_1882966428__VARIABLE_STACK_HPP__

#include <map>
#include <deque>
#include <functional>

#include "data_structure.hpp"

namespace neam
{
  namespace r
  {
    namespace shell
    {
      /// \brief A class that handles stacking context each one holding multiple variables / functions.
      /// \note This is a simple working solution, nothing more !
      class variable_stack
      {
        public:
          using special_variable_handler_sig = std::string(const std::string &); ///< \brief The special variable handler signature

        public:
          variable_stack();
          ~variable_stack() = default;

          /// \brief Create a new context (like when calling a function)
          /// \param[in] argument_list if the current context have to handle an argument_list
          /// \param[in] copy_on_write_context create a copy-on-write context (a bit like a subshell).
          ///                                  All modification is done the in the current context
          void push_context(bool argument_list = true, bool copy_on_write_context = false);

          /// \brief Remove the current context (like when returning from a function)
          void pop_context();

          /// \brief Return the context depth
          size_t get_context_depth() const
          {
            return context_stack.size();
          }

          /// \brief Return a variable value
          /// \note If not found, it returns an empty string
          std::string get_variable(const std::string &name) const;

          /// \brief Check if a variable exists
          bool has_variable(const std::string &name) const;

          /// \brief Create a new variable
          /// \note in_parent_context indicate if the variable should be created in the root context (the context of the shell)
          ///       instead of the current one.
          void set_variable(const std::string &name, const std::string &value = "", bool in_root_context = false);

          /// \brief Remove a variable
          /// \note In a cow context, it does nothing if the variable is not in the current context
          void unset_variable(const std::string &name);

          /// \brief Return a function
          /// \note If not found, it returns nullptr
          const command_list *get_function(const std::string &name) const;

          /// \brief Check if a function exists
          bool has_function(const std::string &name) const;

          /// \brief Create a new function
          /// \note in_parent_context indicate if the function should be created in the root context (the context of the shell)
          ///       instead of the current one.
          void set_function(const std::string &name, const command_list &func, bool in_root_context = false);

          /// \brief Remove a function
          /// \note In a cow context, it does nothing if the function is not in the current context
          void unset_function(const std::string &name);

          /// \brief Shift the argument list
          bool shift_arguments(size_t shift_count = 1);

          /// \brief Add a new argument at the end of the list
          void push_argument(const std::string &value);

          /// \brief Return the argument list as a C array as main can receive it
          /// \note it is null terminated
          /// \see get_argument_count()
          /// \see free_argument_c_array()
          char **get_argument_as_c_array() const;

          /// \brief Returns a reference to the argument vector.
          /// \note The reference is valid until you call pop_context();
          const std::deque<std::string> &get_argument_array() const;

          /// \brief Free the array returned by get_argument_as_c_array()
          /// \see get_argument_as_c_array()
          static void free_argument_c_array(char **argv);

          /// \brief Return the argument count
          size_t get_argument_count() const;

          /// \brief Create a special handler for a variable
          /// the provided function will be called each time the variable is referenced
          /// \note the handler is not able to use a stream_pack and thus should **not** print anything
          ///       (except, maybe, in case of critical things using `neam::cr::out.{error,critical}()...` )
          void set_variable_special_handler(const std::string &name, const std::function<special_variable_handler_sig> &hdlr);

          /// \brief Remove the special handler for a variable
          void unset_variable_special_handler(const std::string &name);

        private:
          struct context;
          void cow_copy_argument_list(context *ctx, size_t index);

        private:
          /// \brief A context
          struct context
          {
            bool has_argument_list;
            bool is_copy_on_write;

            std::deque<std::string> argument_list;

            std::map<std::string, std::string> variable_list;
            std::map<std::string, command_list> function_list;
          };

        private:
          std::deque<context> context_stack;
          std::map<std::string, std::function<special_variable_handler_sig>> special_variable_list;
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_1251124832855879829_1882966428__VARIABLE_STACK_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

