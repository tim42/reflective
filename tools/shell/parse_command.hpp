//
// file : parse_command.hpp
// in : file:///home/tim/projects/reflective/tools/shell/parse_command.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 11/02/2016 17:35:24
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

#ifndef __N_7862938051710451439_1386810612__PARSE_COMMAND_HPP__
# define __N_7862938051710451439_1386810612__PARSE_COMMAND_HPP__

#include <string>
#include "data_structure.hpp"

namespace neam
{
  namespace r
  {
    namespace shell
    {
      /// \brief parse a command and output the result into ast
      /// \note I've split this command in another source file 'cause the parser
      ///       takes forever to build
      bool parse_command(const std::string &cmd, neam::r::shell::command_list &ast);
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_7862938051710451439_1386810612__PARSE_COMMAND_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

