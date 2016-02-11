//
// file : builtin_mgr.hpp
// in : file:///home/tim/projects/reflective/tools/shell/builtin_mgr.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 10/02/2016 21:04:15
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

#ifndef __N_1160740042847802731_1504792624__BUILTIN_MGR_HPP__
# define __N_1160740042847802731_1504792624__BUILTIN_MGR_HPP__

#include <string>
#include <map>

#include "data_structure.hpp"
#include "variable_stack.hpp"
#include "streams.hpp"
#include "builtin.hpp"

namespace neam
{
  namespace r
  {
    namespace shell
    {
      /// \brief A simple way to handle / manage builtins
      /// \note builtins are a bit specials 'cause they should never be forked
      ///       and thus handle streams and redirections transparently
      class builtin_mgr
      {
        public:
          builtin_mgr();

          /// \brief Add a builtin to the manager
          bool register_builtin(const std::string &name, builtin &blt);

          /// \brief Remove a builtin from the manager
          void unregister_builtin(const std::string &name);

          /// \brief Remove a builtin from the manager
          void unregister_builtin(builtin &blt);

          /// \brief Check if a builtin exists
          bool has_builtin(const std::string &name) const;

          /// \brief Return a builtin
          builtin &get_builtin(const std::string &name);

          /// \brief Return a builtin
          const builtin &get_builtin(const std::string &name) const;

          /// \brief Call a builtin
          /// \param[out] ret the builtin return value
          /// \return true if a builtin has been called, false otherwise
          bool call(const std::string &name, variable_stack &stack, stream_pack &streamp, int &ret) const;

          /// \brief Print the help section of a builtin
          bool print_help(const std::string &name, stream_pack &streamp) const;

          /// \brief Print the summary message of all builtins
          /// (a bit like a help builtin would do)
          void print_builtin_summary(stream_pack &streamp) const;

        private:
          std::map<std::string, builtin &> builtin_list;
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_1160740042847802731_1504792624__BUILTIN_MGR_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

