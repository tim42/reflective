//
// file : graph_walker.hpp
// in : file:///home/tim/projects/reflective/tools/quick-report/graph_walker.hpp
//
// created by : Timothée Feuillet
// date: Fri Jan 20 2017 23:49:38 GMT-0500 (EST)
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

#ifndef __N_2151230272603410946_1941930275_GRAPH_WALKER_HPP__
#define __N_2151230272603410946_1941930275_GRAPH_WALKER_HPP__

#include <reflective/introspect.hpp>

namespace neam
{
  namespace rtools
  {
    class graph_walker
    {
      public:
        /// \brief Walk the whole callgraph
        /// \param f A function/callable that has a similar signature: f(const introspect &current, const std::deque< introspect >&stack)
        template<typename Func>
        static void walk(Func&& f)
        {
          std::deque<r::introspect> current_callstack;

          for (const r::introspect &root : r::introspect::get_root_function_list())
          {
            current_callstack = {root};
            subwalk(current_callstack, root, f);
          }
        }

    private:
        template<typename Func>
        static void subwalk(std::deque<r::introspect>& current_callstack, const r::introspect &current, Func&& f)
        {
          f(current, current_callstack);

          for (const r::introspect &callee : current.get_callee_list())
          {
            current_callstack.push_back(callee);
            subwalk(current_callstack, callee, f);
            current_callstack.pop_back();
          }
        }
    };
  } // namespace rtools
} // namespace neam

#endif // __N_2151230272603410946_1941930275_GRAPH_WALKER_HPP__

