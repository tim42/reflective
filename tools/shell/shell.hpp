//
// file : shell.hpp
// in : file:///home/tim/projects/reflective/tools/shell/shell.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 11/02/2016 14:40:24
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

#ifndef __N_2586361471611187627_206167030__SHELL_HPP__
# define __N_2586361471611187627_206167030__SHELL_HPP__

#include "streams.hpp"
#include "variable_stack.hpp"
#include "builtin_mgr.hpp"
#include "builtin.hpp"

namespace neam
{
  namespace r
  {
    namespace shell
    {
      class shell
      {
        public:
          shell();
          ~shell() = default;

          /// \brief return the builtin manager
          builtin_mgr &get_builtin_manager() { return bltmgr; }

          /// \brief return the builtin manager
          const builtin_mgr &get_builtin_manager() const { return bltmgr; }

          /// \brief return the builtin manager
          variable_stack &get_variable_stack() { return vstack; }

          /// \brief return the builtin manager
          const variable_stack &get_variable_stack() const { return vstack; }

          /// \brief return the shell own streampack
          stream_pack &get_shell_streampack() { return spack; }

          /// \brief return the shell own streampack
          const stream_pack &get_shell_streampack() const { return spack; }

          /// \brief run the shell (either an entire file or a simple command-line)
          void run(const std::string &commands);

          /// \brief invoke a command. You must have setup the variable_stack correctly before
          ///        calling this.
          /// If you don't know hot to handle that task, use run() with your full command
          /// \see run()
          /// \note this is a quite internal method
          int run_cmd(const std::string &invocation_name, stream_pack &streamp);

        private:
          /// \brief Execute commands in the current variable_stack context
          int execute_no_context(const std::string &commands);

        private:
          /// \brief register the :, help, exit, return, builtin, . || source builtins
          void register_base_builtins();

          void builtin_builtin();
          void builtin_echo();
          void builtin_exit();
          void builtin_help();
          void builtin_nothing();
          void builtin_return();
          void builtin_shift();
          void builtin_source();

        private:
          variable_stack vstack;
          stream_pack spack;
          builtin_mgr bltmgr;
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_2586361471611187627_206167030__SHELL_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

