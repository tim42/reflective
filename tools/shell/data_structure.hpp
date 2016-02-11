//
// file : data_structure.hpp
// in : file:///home/tim/projects/reflective/tools/shell/data_structure.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 10/02/2016 18:25:09
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

#ifndef __N_20383540332043849254_1297765537__DATA_STRUCTURE_HPP__
# define __N_20383540332043849254_1297765537__DATA_STRUCTURE_HPP__

#include <boost/variant.hpp>

namespace neam
{
  namespace r
  {
    namespace shell
    {
      // 'cause we can recurse in multiple kind of thing
      struct command;
      struct command_list;
      struct subshell;
      struct var_expansion;
      struct function_declaration;
      struct var_affectation;
      using command_node = boost::variant
      <
        boost::recursive_wrapper<var_affectation>,
        boost::recursive_wrapper<command>,
        boost::recursive_wrapper<command_list>,
        boost::recursive_wrapper<subshell>,
        boost::recursive_wrapper<function_declaration>
      >;
      using node = boost::variant
      <
        std::string,
        boost::recursive_wrapper<var_expansion>,
        boost::recursive_wrapper<command_node>
      >;

      using node_list = std::vector<node>;

      // when expanding variables
      struct var_expansion
      {
        node_list variable_name;
      };

      // when affecting variables
      struct var_affectation
      {
        node_list variable_name;
        node_list value;
      };

      // a command
      struct command
      {
        std::vector<var_affectation> affectations;
        node_list command_name;
        std::vector<node_list> arguments;
      };
      struct command_list
      {
        std::vector<command_node> list;
      };
      struct subshell
      {
        std::vector<command_node> list;
      };

      struct function_declaration
      {
        node_list name;
        command_list body;
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_20383540332043849254_1297765537__DATA_STRUCTURE_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

