//
// file : sequence.hpp
// in : file:///home/tim/projects/reflective/reflective/sequence.hpp
//
// created by : Timothée Feuillet
// date: Fri Jan 20 2017 22:16:23 GMT-0500 (EST)
//
//
// Copyright (c) 2017 Timothée Feuillet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef __N_2362151062312724466_63129047_SEQUENCE_HPP__
#define __N_2362151062312724466_63129047_SEQUENCE_HPP__

#include <string>
#include <deque>
#include <memory>

namespace neam
{
  namespace cr
  {
    class persistence;
  } // namespace cr

  namespace r
  {
#define N_SEQUENCE_ENTRY_INFO __FILE__, __LINE__

    /// \brief Represent/Store a sequence of actions.
    /// Actions have a name, a description, file and line and are guaranteed to be in order
    class sequence
    {
      public:
        /// \brief A sequence entry
        /// \see N_SEQUENCE_ENTRY_INFO
        struct entry
        {
#ifdef _MSC_VER
          entry(const std::string &_file = std::string(), size_t _line = 0, const std::string &_name = std::string(), const std::string &_desc = std::string())
            : file(_file), line(_line), name(_name), description(_desc)
          {}
#endif
          std::string file = std::string();
          size_t line = 0;
          std::string name = std::string();
          std::string description = std::string();
        };

      public:
        /// \brief Add a new entry to the sequence
        /// \see N_SEQUENCE_ENTRY_INFO
        void add_entry(entry &&ent) { entries.emplace_back(std::move(ent)); }

        /// \brief Add a new entry to the sequence
        /// \see N_SEQUENCE_ENTRY_INFO
        void add_entry(const entry &ent) { entries.push_back(std::move(ent)); }

        /// \brief Return the entries of that sequence
        const std::deque<entry> &get_entries() const { return entries; }

        /// \brief Clear the sequence
        void clear_sequence() { entries.clear(); }

      private:
        std::deque<entry> entries;

        friend neam::cr::persistence;
    };
  } // namespace r
} // namespace neam

#endif // __N_2362151062312724466_63129047_SEQUENCE_HPP__

