//
// file : debug.hpp
// in : file:///home/tim/projects/reflective/tools/shell/debug.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 06/02/2016 18:38:12
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

#ifndef __N_1854243817245572116_997221446__DEBUG_HPP__
# define __N_1854243817245572116_997221446__DEBUG_HPP__

#include <iomanip>
#include "data_structure.hpp"

namespace neam
{
  namespace r
  {
    namespace shell
    {
      class printer : public boost::static_visitor<void>
      {
        private:
          static void print_indent(unsigned int level)
          {
            const unsigned int spaces_per_indent = 2;
            for (size_t i = 0; i < level * spaces_per_indent; ++i)
              std::cout << ' ';
          }
          unsigned int indent;
          printer(unsigned int _indent = 0) : indent(_indent) {}
        public:
          static void print(const command_list &t)
          {
            std::cout << std::endl;
            (printer(0))(t);
            std::cout << std::endl;
          }

          void operator () (const var_expansion &ve) const
          {
            std::cout << "${";
            for (auto &e : ve.variable_name)
              boost::apply_visitor(printer(indent), e);
            std::cout << "}";
          }
          void operator () (const var_affectation &af) const
          {
            for (auto &e : af.variable_name)
              boost::apply_visitor(printer(indent), e);
            std::cout << "=";
            for (auto &e : af.value)
              boost::apply_visitor(printer(indent), e);
          }
          void operator()(const command_node &cn) const
          {
            boost::apply_visitor(printer(indent), cn);
          }
          void operator()(const command_list &cl) const
          {
            std::cout << "{\n";
            for (auto &c : cl.list)
            {
              print_indent(indent + 1);
              boost::apply_visitor(printer(indent + 1), c);
              std::cout << "\n";
            }
            print_indent(indent);
            std::cout << "}";
          }
          void operator()(const and_command_list &cl) const
          {
            bool first = true;
            std::cout << " [and> ";
            for (auto &c : cl.list)
            {
              if (!first)
                std::cout << " && ";
              first = false;
              boost::apply_visitor(printer(indent + 1), c);
            }
            std::cout << " <and] ";
          }
          void operator()(const or_command_list &cl) const
          {
            bool first = true;
            std::cout << " [or> ";
            for (auto &c : cl.list)
            {
              if (!first)
                std::cout << " || ";
              first = false;
              boost::apply_visitor(printer(indent + 1), c);
            }
            std::cout << " <or] ";
          }
          void operator()(const subshell &cl) const
          {
            std::cout << "(\n";
            for (auto &c : cl.list)
            {
              print_indent(indent + 1);
              boost::apply_visitor(printer(indent + 1), c);
              std::cout << "\n";
            }
            print_indent(indent);
            std::cout << ")";
          }
          void operator()(const command &c) const
          {
            std::cout << "$(";
            for (auto &af : c.affectations)
            {
              (printer(indent + 1))(af);
              std::cout << " ";
            }
            if (c.command_name.size())
            {
              std::cout << '<';
              for (auto & e : c.command_name)
                boost::apply_visitor(printer(indent + 1), e);
              std::cout << '>';
            }
            for (auto &ar : c.arguments)
            {
              std::cout << " ";
              for (auto & e : ar)
                boost::apply_visitor(printer(indent + 1), e);
            }
            std::cout << ")";
          }
          void operator()(const function_declaration &f) const
          {
            std::cout << "function ";
            for (auto &e : f.name)
              boost::apply_visitor(printer(indent), e);
            std::cout << " ";
            (printer(indent))(f.body);
          }
          void operator()(const std::string &s) const
          {
            std::cout << std::quoted(s);
          }
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_1854243817245572116_997221446__DEBUG_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

