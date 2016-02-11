//
// file : builtin.hpp
// in : file:///home/tim/projects/reflective/tools/shell/builtin.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 10/02/2016 21:07:42
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

#ifndef __N_304596874464078732_928258202__BUILTIN_HPP__
# define __N_304596874464078732_928258202__BUILTIN_HPP__

#include <string>
#include <functional>

#include <boost/program_options.hpp>

#include "streams.hpp"
#include "variable_stack.hpp"

namespace neam
{
  namespace r
  {
    namespace shell
    {
      class builtin
      {
        public:
          using callback_function_sig = int(const std::string &, variable_stack &, stream_pack &, boost::program_options::variables_map &);

        public:
          /// \brief Construct the builtin object
          /// \param[in] message_banner A short descriptif of the builtin
          builtin(std::function<callback_function_sig> _callback, const std::string &message_banner = "");

          /// \brief Construct the builtin object
          /// \param[in] message_banner A short descriptif of the builtin
          /// \param[in] usage_message The descriptif of the options the builtin takes
          ///                          could be "[options] input-file"
          ///                          or "[options] command [command-options]"
          ///                          Should be an one-liner
          builtin(std::function<callback_function_sig> _callback, const std::string &message_banner, const std::string &usage_message);

          ~builtin() = default;

          /// \brief shortcut for adding options to the builtin
          /// \see boost::program_options
          inline auto add_options() { return desc.add_options(); }

          /// \brief handle positional options
          /// \see boost::program_options
          inline boost::program_options::positional_options_description &get_positional_options_description()
          {
            return pod;
          }

          /// \brief Disable boost's program_options on this builtin. The default is to use it.
          void do_not_use_program_options(bool dontuseit = true) { skip_program_options = dontuseit; }

          /// \brief Disallow unknown parameters. The default is to allow them.
          void disallow_unknow_parameters(bool disallowthem = true) { no_unknown_params = disallowthem; }

          /// \brief Call the builtin
          int call(const std::string &name, neam::r::shell::variable_stack &stack, neam::r::shell::stream_pack &streamp);

          /// \brief Print the help section of the builtin
          void help(const std::string &name, neam::r::shell::stream_pack &streamp);

          /// \brief Return the help message banner
          inline const std::string &get_help_message_banner() const
          {
            return help_message_banner;
          }

          /// \brief Return the usage message
          inline const std::string &get_usage_message() const
          {
            return help_usage_message;
          }

        private:
          std::string help_message_banner;
          std::string help_usage_message;

          std::function<callback_function_sig> callback;

          boost::program_options::options_description desc;
          boost::program_options::positional_options_description pod;
          boost::program_options::variables_map vm;

          bool skip_program_options = false;
          bool no_unknown_params = false;
      };
    } // namespace shell
  } // namespace r
} // namespace neam

#endif /*__N_304596874464078732_928258202__BUILTIN_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

