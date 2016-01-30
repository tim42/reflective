//
// file : function_call.hpp
// in : file:///home/tim/projects/reflective/reflective/function_call.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 26/01/2016 18:15:46
//
//
// Copyright (C) 2014 Timothée Feuillet
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

#ifndef __N_1929495401479333861_60769628__FUNCTION_CALL_HPP__
# define __N_1929495401479333861_60769628__FUNCTION_CALL_HPP__

#include <tools/demangle.hpp>

#include <tools/macro.hpp>
#include <tools/chrono.hpp>

#include "id_gen.hpp"
#include "reason.hpp"
#include "call_info_struct.hpp"
#include "storage.hpp"
#include "if_wont_fail.hpp"

namespace neam
{
  namespace r
  {
    namespace internal
    {
      /// \brief only present for type deduction
      template<typename T> struct type { using t = T; };
    } // namespace internal

    /// \brief The public interface of function call
    class function_call
    {
      private:
        void common_init();

      public:
        /// \brief Construct a function call object
        /// \see N_PRETTY_FUNCTION_INFO
        /// \code auto self_call = function_call(N_PRETTY_FUNCTION_INFO(my_class::my_function)); \endcode
        template<typename FuncType, FuncType Func>
        function_call(const char *pretty_function, const char *const name, uint32_t hash, neam::embed::embed<FuncType, Func>)
          : call_info_index(0), call_info(internal::get_call_info_struct<FuncType, Func>(hash, name, pretty_function, &call_info_index)),
            global(internal::get_global_data()), tl_data(internal::get_thread_data())
        {
          common_init();
        }

        /// \brief Construct a function call object for a lambda
        /// \note lambda are slower than normal functions 'cause the hash is computed at runtime and this needs RTTI (but it have a cache for call_info_index results)
        /// \see N_PRETTY_LAMBDA_INFO
        /// \code auto self_call = function_call(N_PRETTY_LAMBDA_INFO(my_lbd_variable)); \endcode
        template<typename FuncType>
        function_call(const char *pretty_function, const char *const name, uint32_t hash, internal::type<FuncType>)
          : call_info_index(0), call_info(internal::get_call_info_struct<FuncType>(hash, name, pretty_function, &call_info_index)),
            global(internal::get_global_data()), tl_data(internal::get_thread_data())
        {
          common_init();
        }

        /// \brief Construct a function call object, without the pretty function thing
        /// \see N_FUNCTION_INFO
        /// \code auto self_call = function_call(N_FUNCTION_INFO(my_class::my_function)); \endcode
        template<typename FuncType, FuncType Func>
        function_call(const char *const name, uint32_t hash, neam::embed::embed<FuncType, Func>, FuncType &f) : function_call(nullptr, name, hash, neam::embed::embed<FuncType, Func>(), f) {}

        /// \brief Construct a function call object for a lambda, without the pretty function thing
        /// \see N_LAMBDA_INFO
        /// \code auto self_call = function_call(N_LAMBDA_INFO(my_lbd_variable)); \endcode
        template<typename FuncType>
        function_call(const char *const name, uint32_t hash, internal::type<FuncType>) : function_call(nullptr, name, hash, internal::type<FuncType>()) {}
        function_call(const char *const name, uint32_t hash) : function_call(nullptr, name, hash, internal::type<void>()) {}
        function_call(const char *pretty_function, const char *const name, uint32_t hash) : function_call(pretty_function, name, hash, internal::type<void>()) {}

        /// \brief Get the current/active function call
        /// \note Could return nullptr if no function_call is active on the current thread
        static inline function_call *get_top_level_function_call()
        {
          return internal::get_thread_data()->top;
        }

        ~function_call();

        /// \brief Start monitoring the time consumed by this very function
        /// \note All calls that are monitored by reflective doesn't add to this time
        void monitor_self_time() { self_time_monitoring = true; }

        /// \brief Start monitoring the time consumed by this function
        void monitor_global_time() { global_time_monitoring = true; }

        /// \brief Report a failure
        void fail(const reason &rsn);

        /// \brief Allow to put fail calls into return statements (or possibly into throw statements)
        template<typename Ret>
        inline Ret fail(const reason &rsn, Ret r)
        {
          fail(rsn);
          return std::forward<Ret>(r);
        }

        // // statistic / reports // //

        /// \brief Return the probability of a incoming failure
        /// \note This call is contextualized, meaning that it only account fails in the current stack
        /// \note This call only handle the current function, not any of its possible calls
        /// \see get_global_failure_rate()
        float get_failure_ratio() const
        {
          if (!se) return get_global_failure_ratio();
          return float(se->fail_count) / float(se->hit_count);
        }

        /// \brief Return the probability of a incoming failure
        /// \note Unlike get_failure_rate(), it will account all calls of the current function (not only in the current stack)
        /// \note This call only handle the current function, not any of its possible calls
        float get_global_failure_ratio() const
        {
          return float(call_info.fail_count) / float(call_info.call_count);
        }

        /// \brief This function tests if the given function will be likely to fail (fail ratio > 0.5 by default) and returns an object with some properties
        /// to do some kind of conditional execution based on fails
        /// \note It takes a N_FUNCTION_INFO(your_function_here) as parameter. Do not intend to fill those parameters by yourself
        /// \note It use a get_failure_rate() -like way to compute the failure rate (contextualized with the current stack)
        /// \note It has a training time, it will return true until a certain amount of call has been reached. This amount can be set the with returned object.
        /// \note You can change the maximum ratio by setting it in the returned object.
        /// \see if_wont_fail_global()
        /// \todo Add support for lambdas
        template<typename FuncType, FuncType Func>
        internal::if_wont_fail<FuncType, Func> if_wont_fail(const char *const name, uint32_t hash, neam::embed::embed<FuncType, Func>) const
        {
          size_t o_call_info_index;
          internal::call_info_struct &o_call_info = internal::get_call_info_struct<FuncType, Func>(hash, name, nullptr, &o_call_info_index);
          internal::stack_entry *o_se = nullptr;

          float ratio = 0.f;
          size_t count = 0;

          if (se && (o_se = se->get_children_stack_entry(o_call_info_index))) // stack-based failure rate
          {
            ratio = float(o_se->fail_count) / float(o_se->hit_count);
            count = o_se->hit_count;
          }
          else // globally based failure rate (either we don't monitor stack traces, or we don't have called the function yet)
          {
            ratio = float(o_call_info.fail_count) / float(o_call_info.call_count);
            count = o_call_info.call_count;
          }

          return internal::if_wont_fail<FuncType, Func>(count, ratio);
        }

        /// \brief This function tests if the given function will be likely to fail (fail ratio > 0.5 by default) and returns an object with some properties
        /// to do some kind of conditional execution based on fails
        /// \note It takes a N_FUNCTION_INFO(your_function_here) as parameter. Do not intend to fill those parameters by yourself
        /// \note It use a get_global_failure_rate() -like way to compute the failure rate (NOT contextualized with the current stack)
        /// \note It has a training time, it will return true until a certain amount of call has been reached. This amount can be set the with returned object.
        /// \note You can change the maximum ratio by setting it in the returned object.
        /// \see if_wont_fail()
        /// \todo Add support for lambdas
        template<typename FuncType, FuncType Func>
        static internal::if_wont_fail<FuncType, Func> if_wont_fail_global(const char *const name, uint32_t hash, neam::embed::embed<FuncType, Func>)
        {
          size_t o_call_info_index;
          internal::call_info_struct &o_call_info = internal::get_call_info_struct<FuncType, Func>(hash, name, nullptr, &o_call_info_index);

          float ratio = float(o_call_info.fail_count) / float(o_call_info.call_count);
          size_t count = o_call_info.call_count;

          return internal::if_wont_fail<FuncType, Func>(count, ratio);
        }

      private:
        size_t call_info_index;
        internal::call_info_struct &call_info;
        internal::data *global;
        internal::thread_local_data *tl_data;
        function_call *prev;
        cr::chrono self_chrono;
        cr::chrono global_chrono;
        internal::stack_entry *se = nullptr;
        bool self_time_monitoring = false;
        bool global_time_monitoring = false;
    };


/// \brief Workaround some C++ limitations. Also provide what is necessary for the name, hash and func parameters of neam::r::function_call()
/// \note Please provide the full hierarchy of namespaces if possible
#define N_FUNCTION_INFO(f)  N__I__FNAME(f), ::neam::r::internal::generate_id(N__I__FNAME(f), &f), neam::embed::embed<decltype(&f), &f>()

/// \brief Workaround some C++ limitations. Also provide what is necessary for the name, hash and func parameters of neam::r::function_call()
/// \note Please provide the full hierarchy of namespaces if possible
#define N_LAMBDA_INFO(f)    N__I__LNAME(f), ::neam::r::internal::generate_id(N__I__LNAME(f), &f), ::neam::r::internal::type<decltype(f)>()
// #define N_FUNCTION_INFO(f)  N__I__FNAME(f), ::neam::r::internal::force_uint32_t<::neam::r::internal::hash_from_str(N__I__FNAME(f))>::value, neam::embed::embed<decltype(&f), f>()

/// \brief Workaround some C++ limitations. Also provide what is necessary for the name, hash and func parameters of neam::r::function_call()
/// \note Please provide the full hierarchy of namespaces if possible
#define N_PRETTY_FUNCTION_INFO(f)  __PRETTY_FUNCTION__, N_FUNCTION_INFO(f)

/// \brief Workaround some C++ limitations. Also provide what is necessary for the name, hash and func parameters of neam::r::function_call()
/// \note Please provide the full hierarchy of namespaces if possible
#define N_PRETTY_LAMBDA_INFO(f)    __PRETTY_FUNCTION__, N_LAMBDA_INFO(f)

// #define N__I__FNAME(f)    __FILE__ ":" N_EXP_STRINGIFY(__LINE__) "#" N_EXP_STRINGIFY(f) // if_wont_fail will not be possible
#define N__I__FNAME(f)    N_EXP_STRINGIFY(f)
#define N__I__LNAME(f)    typeid(f).name()    // works only for lambdas
  } // namespace r
} // namespace neam

#endif /*__N_1929495401479333861_60769628__FUNCTION_CALL_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

