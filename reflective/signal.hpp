//
// file : signal.hpp
// in : file:///home/tim/projects/reflective/reflective/signal.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 31/01/2016 23:20:21
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

#ifndef __N_1503693175951222115_1733287383__SIGNAL_HPP__
# define __N_1503693175951222115_1733287383__SIGNAL_HPP__

#include <initializer_list>

namespace neam
{
  namespace r
  {
    /// \brief If you install a custom signal handler, but still want reflective to reports signals
    /// Just call this.
    /// \note the same notes and warning from install_default_signal_handler applies to this function
    /// \see install_default_signal_handler
    void on_signal(int sig);

    /// \brief Install the default signal handler on the given signal list
    /// \note The default signal handler behavior is to "report, save and die"
    /// \warning If your program crashed because of a segfault, an uncaught exception, ...
    ///          the heap may be in an invalid state and the serialization will trigger a segfault,
    ///          hard-crashing the program. The reported error will then be in either reflective or persistence,
    ///          with no way to lookup the real crash with the additional problem that no reflective file will be generated.
    ///
    /// \warning Reflective may use dynamic allocation to perform its final save, and if the heap / stack are corrupted, this may lead to
    ///          a crash within a crash handler. Using an external tool (like valgrind or gdb) and deactivating reflective signal handlers
    ///          will most probably help you narrow the issue.
    ///
    /// \note If the conf::cleanup_on_crash config variable is true, an additional file is created "conf::out_file + '.bak'" with the data "as-is",
    ///       as the stack cleanup will most probably lead to another crash
    void install_default_signal_handler(std::initializer_list<int> signals);
  } // namespace r
} // namespace neam

#endif /*__N_1503693175951222115_1733287383__SIGNAL_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

