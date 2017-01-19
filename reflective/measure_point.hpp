//
// file : measure_point.hpp
// in : file:///home/tim/projects/reflective/reflective/measure_point.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 15/02/2016 13:16:18
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

#ifndef __N_9812925241253721972_117284647__MEASURE_POINT_HPP__
# define __N_9812925241253721972_117284647__MEASURE_POINT_HPP__

#include "tools/chrono.hpp"

namespace neam
{
  namespace r
  {
    struct defer_start_t {};
    constexpr static defer_start_t defer_start __attribute__((unused)) = defer_start_t();

    /// \brief Some additional, named, chrono information to a monitored function
    /// \note only a measure_point that has been started and stopped
    ///       (could be by the constructor and the destructor) will be saved.
    class measure_point
    {
      public:
        /// \brief Construct (and activate) a measure point
        /// \param[in] name The name of the measure point
        /// \note name should not a dynamically allocated string, else it has to live in
        ///       the same scope as the instance.
        measure_point(const char *_name) : name(_name)
        {
          start();
        }

        /// \brief Construct (but DOES NOT activate) a measure point
        /// \param[in] name The name of the measure point
        /// \note name should not a dynamically allocated string, else it has to live in
        ///       the same scope as the instance.
        measure_point(const char *_name, defer_start_t) : name(_name) {}

        /// \brief destructor
        ~measure_point()
        {
          stop();
        }

        /// \brief Stop the measure point chrono (if not already stopped)
        /// \note a stopped measure point cannot be re-started.
        void stop()
        {
          if (running)
          {
            value = chrono.get_accumulated_time();
            running = false;
            stopped = true;
            _save();
          }
        }

        /// \brief Start the measure point chrono (if not already started)
        void start()
        {
          if (started)
            return;
          started = true;
          running = true;
          chrono.reset();
        }

        /// \brief Returns the average time of the measure_point
        double get_average_time() const;

        /// \brief Return the elapsed time as measured by the instance.
        /// If the instance has run and is currently stopped, the returned value will be the final value
        /// If the instance has not run yet, it will return 0.
        double get_elapsed_time() const
        {
          if (running)
            return chrono.get_accumulated_time();
          return value;
        }

      private:
        void _save();

      private:
        const char *name;
        cr::chrono chrono;
        double value = 0;

        bool started = false; ///< \brief True if the instance has run
        bool running = false; ///< \brief True if the instance is running
        bool stopped = false; ///< \brief True if the instance is stopped
    };
  } // namespace r
} // namespace neam

#endif /*__N_9812925241253721972_117284647__MEASURE_POINT_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

