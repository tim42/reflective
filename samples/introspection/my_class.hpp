
//
// file : my_class.hpp
// in : file:///home/tim/projects/reflective/samples/advanced/my_class.hpp
//
// created by : Timothée Feuillet
// date: Sun Sep 25 2016 17:04:07 GMT-0400 (EDT)
//
//
// Copyright (c) 2016 Timothée Feuillet
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

#ifndef __N_6786159733020931759_112493575_MY_CLASS_HPP__
#define __N_6786159733020931759_112493575_MY_CLASS_HPP__

#include <vector>

namespace sample
{
  /// \brief This is a sample class that demonstrate how to use reflective on
  /// for a class-wide monitoring
  class my_class
  {
    public:
      my_class();
      my_class(const my_class &o);
      my_class &operator = (const my_class &o);
      ~my_class();

      void my_function();
      void my_function(bool value);

      bool get_top_value() const;
      void push(bool value);

      void may_throw(int i);

    private:
      std::vector<bool> value;
  };
} // namespace sample

#endif // __N_6786159733020931759_112493575_MY_CLASS_HPP__
