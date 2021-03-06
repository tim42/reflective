//
// file : reflective_manager.hpp
// in : file:///home/tim/projects/reflective/tools/shell/reflective_manager.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 12/02/2016 15:52:12
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

#ifndef __N_10789379651389197750_1440974315__REFLECTIVE_MANAGER_HPP__
# define __N_10789379651389197750_1440974315__REFLECTIVE_MANAGER_HPP__

#include <string>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <boost/filesystem.hpp>

#include <reflective.hpp>

namespace neam
{
  namespace r
  {
    /// \brief Information about about a file
    struct file_info
    {
      std::string path; ///< \brief path
      std::vector<introspect> functions; ///< \brief Functions located in that file/folder (context free)
      std::set<std::string> files; ///< \brief files/folders located in that folder

      size_t hit_count = 0;
      size_t error_count = 0;
      float average_self_time = 0.f;
      size_t average_self_time_count = 0;
      float average_global_time = 0.f;
      size_t average_global_time_count = 0;
    };

    /// \brief A nice class to handle more interestingly the data generated by reflective
    class manager
    {
      public:
        manager();
        ~manager();

        /// \brief (re)load the data from the reflective source
        /// The constructor call this method
        /// \note can be slow if the reflective file contains a lot of data
        void load_data();

        /// \brief Get the possible information for the current path
        file_info get_info_for_path(std::string path = "");

        /// \brief change the path (in file mode). could either be a relative path or an absolute one.
        bool change_path(const std::string &new_path);

        /// \brief Return the current working directory/file
        std::string get_current_path() const { return file_path.generic_string(); }


        // --- //

        /// \brief change the path (in function mode). could either be a relative path or an absolute one.
        bool change_introspect_path(const std::string &new_path);

        /// \brief Return the top introspect object or nullptr if there's no top introspect
        introspect *get_top_introspect();

        /// \brief Get all the possible introspect for the current path
        std::vector<introspect> get_child_introspect() const;

        /// \brief Change the current working introspect (used when in introspect/function mode)
        void push_introspect(const introspect &cwi);

        /// \brief Unstack count introspects and return the new current working introspect
        /// \note if you pop all introspects, it will return nullptr
        introspect *pop_introspect(size_t count = 1);

        /// \brief Return the current introspect path
        std::string get_current_introspect_path() const;

      private:
        std::deque<introspect> itp_path; ///< \brief The current path, in term of introspect
        boost::filesystem::path file_path = "/"; ///< \brief The current cwd in file mode

        std::map<std::string, file_info> per_file_info; ///< \brief hold the information relative to each files
    };
  } // namespace r
} // namespace neam

#endif /*__N_10789379651389197750_1440974315__REFLECTIVE_MANAGER_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

