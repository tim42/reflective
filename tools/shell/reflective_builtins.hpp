//
// file : reflective_builtins.hpp
// in : file:///home/tim/projects/reflective/tools/shell/reflective_builtins.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 12/02/2016 15:24:38
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

#ifndef __N_15264679522122705370_1500870462__REFLECTIVE_BUILTINS_HPP__
# define __N_15264679522122705370_1500870462__REFLECTIVE_BUILTINS_HPP__

#include <string>

namespace neam
{
  namespace r
  {
    namespace shell
    {
      class shell;

      /// \brief Register reflective builtins.
      /// \param[in] load_only If true, only the load builtin will be registered, if false, all builtins except load will be registered.
      void register_reflective_builtins(shell &sh, bool load_only = true);

      /// \brief Load a reflective file
      bool load_reflective_file(shell &sh, const std::string &file);
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_15264679522122705370_1500870462__REFLECTIVE_BUILTINS_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

