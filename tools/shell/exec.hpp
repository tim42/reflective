//
// file : exec.hpp
// in : file:///home/tim/projects/reflective/tools/shell/exec.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 09/02/2016 20:43:20
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

#ifndef __N_95754376954093910_2037324498__EXEC_HPP__
# define __N_95754376954093910_2037324498__EXEC_HPP__

#include <sstream>
#include "data_structure.hpp"
#include "shell.hpp"
#include "flow_control.hpp"

namespace neam
{
  namespace r
  {
    namespace shell
    {
      class exec : public boost::static_visitor<int>
      {
        private:
          class stringificator : public boost::static_visitor<std::string>
          {
            public:
              stringificator(shell *_sh, stream_pack &_streamp) : sh(_sh), streamp(_streamp) {}
              stringificator(shell *_sh) : sh(_sh), streamp(sh->get_shell_streampack()) {}

              std::string operator () (const var_expansion &ve)
              {
                std::string vname;
                for (auto &e : ve.variable_name)
                  vname += boost::apply_visitor(*this, e);

                return sh->get_variable_stack().get_variable(vname);
              }

              std::string operator()(const command_node &cn)
              {
                stream_pack tsp;
                std::stringstream capout;
                tsp.streams.emplace_back(streamp[0].rdbuf()); // in
                tsp.streams.emplace_back(capout.rdbuf());     // out
                tsp.streams.emplace_back(streamp[2].rdbuf()); // err

                (exec(sh, tsp))(cn);

                return capout.str();
              }

              std::string operator()(const std::string &s)
              {
                return s;
              }

              shell *sh;
              stream_pack &streamp;
          };

        private:
          exec(shell *_sh, stream_pack &_streamp) : sh(_sh), streamp(_streamp) {}
          exec(shell *_sh) : sh(_sh), streamp(sh->get_shell_streampack()) {}

        public:
          static int run(shell *sh, const command_node &cn)
          {
            exec ex(sh);
            return boost::apply_visitor(ex, cn);
          }

          // capture stdout into a string
          static std::string to_string(shell *sh, const node_list &nl)
          {
            std::string ret;
            for (auto &it : nl)
              ret += to_string(sh, it);
            return ret;
          }

          // capture stdout into a string
          static std::string to_string(shell *sh, const node &n)
          {
            stringificator strf(sh);
            return boost::apply_visitor(strf, n);
          }

          // --- //

          int operator()(const command_node &cn)
          {
            return boost::apply_visitor(*this, cn);
          }
          int operator()(const command_list &cl)
          {
            int last = 0;
            for (auto &c : cl.list)
              last = boost::apply_visitor(*this, c);
            return last;
          }
          int operator()(const subshell &ss)
          {
            raii_var_context(sh->get_variable_stack(), false, true);

            int last = 0;
            for (auto &c : ss.list)
              last = boost::apply_visitor(*this, c);
            return last;
          }
          int operator()(const command &c)
          {
            // we create a specific context for all that is related to pre-affectations (the context is cow but without args)
            raii_var_context _u0;
            if (c.affectations.size())
            {
              sh->get_variable_stack().push_context(false, true);
              _u0.handle_context(sh->get_variable_stack());
            }
            for (auto & af : c.affectations)
              (*this)(af);

            // we may have parent agruments as arguments
            stringificator strf(sh);
            std::vector<std::string> args;
            for (auto & ar : c.arguments)
            {
              std::string arg;
              for (auto & e : ar)
                arg += boost::apply_visitor(strf, e);
              args.emplace_back(std::move(arg));
            }

            // push the new context
            raii_var_context _u1(sh->get_variable_stack(), true, false);

            std::string command_invocation;
            for (auto & e : c.command_name)
              command_invocation += boost::apply_visitor(strf, e);
            sh->get_variable_stack().push_argument(command_invocation);

            for (auto & ar : args)
              sh->get_variable_stack().push_argument(ar);
            int ret = sh->run_cmd(command_invocation, streamp);

            return ret;
          }
          int operator () (const var_affectation &af)
          {
            std::string var_name;
            stringificator strf(sh);
            for (auto & e : af.variable_name)
              var_name += boost::apply_visitor(strf, e);
            std::string value;
            for (auto & e : af.value)
              value += boost::apply_visitor(strf, e);

            sh->get_variable_stack().set_variable(var_name, value);
            return 0;
          }
          int operator()(const function_declaration &f)
          {
            std::string func_name;
            stringificator strf(sh);
            for (auto & e : f.name)
              func_name += boost::apply_visitor(strf, e);

            sh->get_variable_stack().set_function(func_name, f.body);
            return 0;
          }

        private:
          shell *sh;
          stream_pack &streamp;
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_95754376954093910_2037324498__EXEC_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

