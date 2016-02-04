//
// file : if_wont_faill.hpp
// in : file:///home/tim/projects/reflective/reflective/if_wont_faill.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 30/01/2016 15:18:59
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

#ifndef __N_12525294001703727933_157237601__IF_WONT_FAILL_HPP__
# define __N_12525294001703727933_157237601__IF_WONT_FAILL_HPP__

#include <memory>
#include "reason.hpp"

namespace neam
{
  namespace r
  {
    namespace internal
    {
      /// \brief Internal helper struct, used only by if_wont_fail
      template<typename FuncType> struct _func_call // Default
      {
        template<typename... Args>
         auto _call(FuncType func, Args... args) { return func(std::forward<Args>(args)...); }
      };

      void report_exception(std::exception &e);
      void report_unknown_exception();

      /// \brief A nice class to condionally call function based on the failure rate
      template<typename FuncType>
      class if_wont_fail : public _func_call<FuncType>
      {
        public:
          if_wont_fail(size_t _count, float _ratio, FuncType _func) : count(_count), ratio(_ratio), func(_func) {}

          /// \brief Change the minimum ratio
          if_wont_fail &set_max_ratio(float _max_ratio) { max_ratio = _max_ratio; return *this; }
          /// \brief Change the minimum ratio
          if_wont_fail &set_min_count(float _max_ratio) { max_ratio = _max_ratio; return *this; }

          /// \brief Conditionally call the function/method/...
          /// \note \e default_ret MUST be 'castable' into the return type
          /// \see call()
          template<typename Ret, typename... Args>
          auto call_with_ret(Ret default_ret, Args... args)
          {
            if (is_ok())
              return _call(std::forward<Args>(args)...);
            return std::forward<Ret>(default_ret);
          }

          /// \brief Conditionally call the function/method/...
          /// \note The return value is discarded.
          /// \see call_with_ret()
          template<typename... Args>
          if_wont_fail &call(Args... args)
          {
            if (is_ok())
              this->_call(func, std::forward<Args>(args)...);
            return *this;
          }

          /// \brief Chain an action to be done if the thing will NOT fail
          /// \param fnc A function (that returns nothing and takes nothing) that will be called, or not
          /// \see otherwise
          template<typename F>
          if_wont_fail &then(F fnc)
          {
            if (is_ok())
            {
              try
              {
                fnc();
              }
              catch (std::exception &e)
              {
                report_exception(e);
                throw;
              }
              catch (...)
              {
                report_unknown_exception();
                throw;
              }
            }
            return *this;
          }

          /// \brief Chain an action to be done if the thing WILL fail
          /// \param fnc A function (that returns nothing and takes nothing) that will be called, or not
          /// \see then
          template<typename F>
          if_wont_fail &otherwise(F fnc)
          {
            if (!is_ok())
            {
              try
              {
                fnc();
              }
              catch (std::exception &e)
              {
                report_exception(e);
                throw;
              }
              catch (...)
              {
                report_unknown_exception();
                throw;
              }
            }
            return *this;
          }

          /// \brief Used to have boolean values
          /// \see is_ok()
          operator bool() const
          {
            return is_ok();
          }

          /// \brief true if the thing won't fail, false if it will
          inline bool is_ok() const
          {
            return count < min_count || ratio <= max_ratio;
          }

        private:
          size_t count;
          float ratio;
          FuncType func;
          size_t min_count = 100;
          float max_ratio = 0.5f;
      };

      // _func_call specialisations
      template<typename Class, typename... Args, typename Ret>
      struct _func_call<Ret (Class::*)(Args...)>
      {
        static Ret _call(Ret (Class::*func)(Args...), Class *ths, Args... args)
        {
          return (ths->*func)(std::forward<Args>(args)...);
        }
      };
      template<typename Class, typename... Args, typename Ret>
      struct _func_call<Ret (Class::*)(Args...)const>
      {
        static Ret _call(Ret (Class::*func)(Args...)const, Class *ths, Args... args)
        {
          return ths->*func(std::forward<Args>(args)...);
        }
      };
      template<typename Class, typename... Args, typename Ret>
      struct _func_call<Ret (Class::*)(Args...)volatile>
      {
        static Ret _call(Ret (Class::*func)(Args...)volatile, Class *ths, Args... args)
        {
          return ths->*func(std::forward<Args>(args)...);
        }
      };
      template<typename Class, typename... Args, typename Ret>
      struct _func_call<Ret (Class::*)(Args...)const volatile>
      {
        Ret _call(Ret (Class::*func)(Args...)const volatile, Class *ths, Args... args)
        {
          return ths->*func(std::forward<Args>(args)...);
        }
      };
    } // namespace internal
  } // namespace r
} // namespace neam

#endif /*__N_12525294001703727933_157237601__IF_WONT_FAILL_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

