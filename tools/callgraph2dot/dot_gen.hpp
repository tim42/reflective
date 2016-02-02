//
// file : dot_gen.hpp
// in : file:///home/tim/projects/reflective/tools/report/dot_gen.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 01/02/2016 16:23:55
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

#ifndef __N_15399050801962369526_900521243__DOT_GEN_HPP__
# define __N_15399050801962369526_900521243__DOT_GEN_HPP__

#include <ostream>
#include <map>
#include <reflective/introspect.hpp>
namespace neam
{
  namespace r
  {
    class callgraph_to_dot
    {
      public:
        /// \brief Export the callgraph to a dot graph format and write it to a stream
        /// \param[in,out] os The stream where the graph will be output
        /// \param[in] root The root introspect object to use. MUST be a contextualized introspect object.
        ///                 If not specified or nullptr, all roots are used
        bool write_to_stream(std::ostream &os, neam::r::introspect *root = nullptr);

        /// \brief Set whether or not the graph should contain information about failures
        /// (default is true)
        void write_failure_reasons(bool do_write_them)
        {
          out_error = do_write_them;
        }

      private:
        void walk_root(std::ostream &os, neam::r::introspect &root);
        void walk_get_max(neam::r::introspect &root);

      private:
        std::map<const char *, size_t> labels;
        float max = 1.f;
        size_t counter = 0;
        bool out_error = true;
    };
  } // namespace r
} // namespace neam

#endif /*__N_15399050801962369526_900521243__DOT_GEN_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

