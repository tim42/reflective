//
// file : function_call.hpp
// in : file:///home/tim/projects/reflective/reflective/function_call.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 26/01/2016 18:15:46
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

#ifndef __N_1929495401479333861_60769628__FUNCTION_CALL_HPP__
# define __N_1929495401479333861_60769628__FUNCTION_CALL_HPP__

#include "tools/demangle.hpp"
#include "tools/embed.hpp"

#include "tools/macro.hpp"
#include "tools/chrono.hpp"

#include "id_gen.hpp"
#include "func_descriptor.hpp"
#include "reason.hpp"
#include "call_info_struct.hpp"
#include "storage.hpp"
#include "type.hpp"
#include "config.hpp"
namespace neam
{
  namespace r
  {
    class introspect;

    /// \brief The public interface for monitoring function call
    /// This class allow to monitor functions.
    /// \see introspect
    class function_call
    {
      private:
        void common_init();

      public:
        /// \brief Construct a function call object
        /// \see N_PRETTY_FUNCTION_INFO
        /// \code auto self_call = function_call(N_PRETTY_FUNCTION_INFO(my_class::my_function)); \endcode
        template<typename FuncType, FuncType Func>
        function_call(const func_descriptor &d, neam::embed::embed<FuncType, Func>)
          : call_info_index(0), call_info(internal::get_call_info_struct<FuncType, Func>(d, &call_info_index)),
            global(internal::get_global_data()), tl_data(internal::get_thread_data())
        {
          common_init();
        }

        /// \brief Construct a function call object for a [?]
        /// \note lambda are slower than normal functions 'cause the hash is computed at runtime and this needs RTTI (but it have a cache for call_info_index results)
        /// \see N_PRETTY_NAME_INFO
        /// \code neam::r::function_call self_call(N_PRETTY_NAME_INFO(my_lbd_variable)); \endcode
        template<typename FuncType>
        function_call(const func_descriptor &d, internal::type<FuncType>)
          : call_info_index(0), call_info(internal::get_call_info_struct<FuncType>(d, &call_info_index)),
            global(internal::get_global_data()), tl_data(internal::get_thread_data())
        {
          common_init();
        }

        /// \brief If you use this, you have to really know what you're doing...
        function_call(const char *const name)
          : function_call(func_descriptor{name, nullptr, nullptr, 0, name, internal::hash_from_str(name)}, internal::type<void>()) {}
        /// \brief If you use this, you have to really know what you're doing...
        function_call(const char *pretty_function, const char *const name)
          : function_call(func_descriptor{name, pretty_function, nullptr, 0, name, internal::hash_from_str(name)}, internal::type<void>()) {}

        /// \brief Get the current/active function call
        /// \warning The returned pointer is ONLY valid in the current scope and should never be stored
        /// \note Could return nullptr if no function_call is active on the current thread
        static inline function_call *get_active_function_call()
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

        /// \brief Report something (warning, info, status, ...)
        /// Reports are a bit slower than fails
        /// \note If enabled in the config, reports may be printed on the console
        void report(const std::string &mode, const reason &rsn);

        /// \brief Allow to put fail calls into return statements (or possibly into throw statements)
        template<typename Ret>
        inline Ret fail(const reason &rsn, Ret&& r)
        {
          fail(rsn);
          return (r);
        }

        /// \brief Create or clear a sequence.
        sequence &create_sequence(const std::string &name);
        /// \brief Return a sequence if it exists (if not, returns nullptr)
        sequence *get_sequence(const std::string &name);
        /// \brief Walk the stack searching if a given sequence exists in the callers.
        /// If it does find one, it returns it, else it returns 0
        sequence *get_sequence_callers(const std::string &name);
        /// \brief Remove a sequence
        void remove_sequence(const std::string &name);

        /// \brief Return a contextualized introspect object
        /// If you want stats, it's this way
        introspect get_introspect() const;

      private:
        size_t call_info_index;
        internal::call_info_struct &call_info;
        internal::data *global;
        internal::thread_local_data *tl_data;
        function_call *prev;
        cr::chrono self_chrono;
        cr::chrono global_chrono;
        internal::stack_entry *se = nullptr;
        bool self_time_monitoring = conf::monitor_self_time;
        bool global_time_monitoring = conf::monitor_global_time;

        bool has_exception = false; // has been constructed when an exception was active

        friend class measure_point;
    };
#ifdef _MSC_VER
#define _R_PRETTY_FUNC __FUNCSIG__
#else
#define _R_PRETTY_FUNC __PRETTY_FUNCTION__
#endif

/// \brief Workaround some C++ limitations. Also provide what is necessary for the name, hash and func parameters of neam::r::function_call()
/// \note Please provide the full hierarchy of namespaces if possible
/// \param f is a method or a function with the full hierarchy of namespaces
#define N_PRETTY_FUNCTION_INFO(f) neam::r::func_descriptor { N_EXP_STRINGIFY(f), _R_PRETTY_FUNC, __FILE__, __LINE__, N__I__FNAME(f), neam::r::internal::generate_id(N__I__FNAME(f), &f)}, neam::embed::embed<decltype(&f), &f>()

/// \brief Use this is if you have to monitor constructors, destructors, lambdas, strange things and awkward moments
/// \note Using this will work in every case (but could be a little bit slower than N_*FUNCTION_INFO as you don't have cache)
/// \note Using this, you will not be able to use the function with if_wont_fail and introspecting that function will not be possible directly
/// \param n is a C string. Better if the string is known at compile-time.
#ifdef _MSC_VER
#define N_PRETTY_NAME_INFO(n) neam::r::func_descriptor {n, _R_PRETTY_FUNC, __FILE__, __LINE__, N__I__NNAME, neam::r::internal::hash_from_str(N__I__NNAME)}, neam::r::internal::type<neam::r::internal::file_type<__COUNTER__, __LINE__>>()
#else
#define N_PRETTY_NAME_INFO(n) neam::r::func_descriptor {n, _R_PRETTY_FUNC, __FILE__, __LINE__, N__I__NNAME, neam::r::internal::hash_from_str(N__I__NNAME)}, neam::r::internal::type<neam::r::internal::file_type<neam::r::internal::hash_from_str(__FILE__), __LINE__>>()
#endif
/// \brief Use this with a method or a function that you monitor with N_PRETTY_FUNCTION_INFO
/// \param n is a C string. Better if the string is known at compile-time.
/// \param f is a method or a function with the full hierarchy of namespaces
#define N_FUNCTION(f)    neam::r::func_descriptor {N_EXP_STRINGIFY(f), std::string(), std::string(), 0, std::string(), 0}, neam::embed::embed<decltype(&f), &f>()

/// \brief
/// \param n is a C string. Better if the string is known at compile-time.
/// \param f is a method or a function with the full hierarchy of namespaces
#define N_NAME(n, f)    neam::r::func_descriptor {N_EXP_STRINGIFY(f), std::string(), std::string(), 0, std::string(), 0}, neam::r::internal::type<decltype(&f)>()

#if 0
/// \brief Workaround some C++ limitations. Also provide what is necessary for the name, hash and func parameters of neam::r::function_call()
/// \note Please provide the full hierarchy of namespaces if possible
/// \param f is a method or a function with the full hierarchy of namespaces
#define N_FUNCTION_INFO(f) neam::r::func_descriptor { N_EXP_STRINGIFY(f), nullptr, nullptr, 0, N__I__FNAME(f), neam::r::internal::generate_id(N__I__FNAME(f), &f)}, neam::embed::embed<decltype(&f), &f>()

/// \brief Workaround some C++ limitations. Also provide what is necessary for the name, hash and func parameters of neam::r::function_call()
/// \note Please provide the full hierarchy of namespaces if possible
/// \param n is a C string
#define N_NAME_INFO(n) neam::r::func_descriptor {n, nullptr, nullptr, 0, N__I__NNAME, neam::r::internal::hash_from_str(N__I__NNAME)}, neam::r::internal::type<void>()

/// \brief Use this is if everything else fails or gives awkward results
/// \param n is a C string
/// \note This should be your last resort as it is totally arbitrary
#define N_PRETTY_ARBITRARY_INFO(n) neam::r::func_descriptor {n, _R_PRETTY_FUNC, __FILE__, __LINE__, n, neam::r::internal::hash_from_str(n)}, neam::r::internal::type<void>()
#endif

#define N__I__FNAME(f)    __FILE__ ":" N_EXP_STRINGIFY(__LINE__) "#" N_EXP_STRINGIFY(f)
#define N__I__NNAME       __FILE__ ": " N_EXP_STRINGIFY(__LINE__)
  } // namespace r
} // namespace neam

#endif /*__N_1929495401479333861_60769628__FUNCTION_CALL_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

