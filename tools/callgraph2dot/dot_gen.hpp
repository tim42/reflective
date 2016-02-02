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
        /// default is to include them and trace the full error path
        void include_failure_reasons(bool do_include_them, bool trace_full_path = true)
        {
          out_error = do_include_them;
          if (out_error)
            trace_full_error_path = trace_full_path;
        }

        /// \brief Remove branches that are not significants
        /// (under ten callcount, small time impact)
        /// default is to keep them
        /// \note an insignificant call will be kept if it has a child that is not insignificant or lead to an error (and error are included)
        void remove_insignificant_branch(bool do_remove_them, size_t _min_call_count = 0, float _min_gbl_time = -1.f)
        {
          remove_insignificant = do_remove_them;
          if (remove_insignificant)
          {
            if (_min_call_count > 0)
              min_call_count = _min_call_count;
            if (_min_gbl_time > 0)
              min_gbl_time = _min_gbl_time;
          }
        }

        /// \brief Change the way the weight is computed
        void set_weight_properties(bool with_global_time = true, bool with_callcount = true)
        {
          weight_with_global_time = with_global_time;
          weight_with_callcount = with_callcount;
        }

      private:
        void walk_root(std::ostream &os, neam::r::introspect &root, float &error_factor, bool &insignificant);
        void walk_get_max(neam::r::introspect &root);
        void output_reason(std::ostream &os, size_t idx, neam::r::reason &r);
        size_t get_idx_for_introspect(const neam::r::introspect &itr);

      private:
        std::map<std::string, size_t> idxs;
        std::map<neam::r::reason, size_t> reason_idxs;
        std::map<size_t, neam::r::introspect> introspect_labels;
        float max_count = 1.f;
        float max_self = 0.000001f;
        size_t max_self_count = 0;
        size_t counter = 0;
        bool average_call_count = true; // average the number of call count over the number of time the program has been launched
        bool weight_with_global_time = true;
        bool weight_with_callcount = true;
        bool out_error = true;
        bool trace_full_error_path = true;
        bool remove_insignificant = true;
        size_t min_call_count = 10;
        float min_gbl_time = 0.001; // 1ms
    };
  } // namespace r
} // namespace neam

#endif /*__N_15399050801962369526_900521243__DOT_GEN_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

