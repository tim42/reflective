//
// file : introspect.hpp
// in : file:///home/tim/projects/reflective/reflective/introspect.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 31/01/2016 14:00:13
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

#ifndef __N_468510512048024221_1859630211__INTROSPECT_HPP__
# define __N_468510512048024221_1859630211__INTROSPECT_HPP__

#include <cstdint>
#include <vector>

#include <tools/embed.hpp>

#include "storage.hpp"
#include "reason.hpp"
#include "call_info_struct.hpp"
#include "type.hpp"
#include "id_gen.hpp"
#include "if_wont_fail.hpp"

namespace neam
{
  namespace r
  {
    class function_call;

    /// \brief The main class for introspection
    /// \see function_call
    /// \see function_call::get_introspect()
    class introspect
    {
      private:
        introspect(internal::call_info_struct &_call_info, size_t index, internal::stack_entry *_context)
          : call_info_index(index), call_info(_call_info), global(internal::get_global_data()), context(_context)
        {
        }

      public:
        /// \brief Construct a function call object
        /// \see N_FUNCTION_INFO
        /// \code neam::r::introspect info(N_FUNCTION_INFO(my_function)); \endcode
        /// \throw std::runtime_error if the function is not found
        template<typename FuncType, FuncType Func>
        introspect(const func_descriptor &d, neam::embed::embed<FuncType, Func>)
          : call_info_index(0), call_info(internal::get_call_info_struct<FuncType, Func>(d, &call_info_index, true)),
            global(internal::get_global_data())
        {
        }

        /// \brief Construct a function call object for a [?]
        /// \note this is "slower" than normal functions 'cause the hash is computed at runtime (but it have a cache for call_info_index results)
        /// \throw std::runtime_error if the function is not found
        template<typename FuncType>
        introspect(const func_descriptor &d, internal::type<FuncType>)
          : call_info_index(0), call_info(internal::get_call_info_struct<FuncType>(d, &call_info_index, true)),
            global(internal::get_global_data())
        {
        }

        /// \brief Allow you to query at runtime arbitrary functions or methods
        /// \note This is the \b ONLY recommended constructor: the other are \b NOT guaranteed to work as they can rely on information unavailable in the current context for the user
        /// \note this is slower (no hash computed (a slower search is done), no cache), but this allows more possibilities
        /// \param[in] name The name of the function plus all the namespaces encapsulating it (like: "neam::r::introspect::common_init")
        /// \throw std::runtime_error if the function is not found
        /// \note that create a context-free introspect object (that can be latter contextualized)
        explicit introspect(const char *const name) : introspect(func_descriptor {name}, internal::type<void>()) {}

        /// \brief Copy constructor (no move constructor, 'cause it wouldn't improve anything)
        introspect(const introspect &o);

        /// \brief Copy operator
        introspect &operator = (const introspect &o);

        // ---- // Things that interact with the callgraph // ---- //

        /// \brief Reset the gathered statistics for this particular function
        /// \note slow, as it has to iterate over ALL the callgraph
        /// \note As its name implies it, this is a destructive operation
        void reset();

        /// \brief Walks the callgraph to provide the callee list (the list of function that are called by this one)
        /// \note As "walk" can induce it, it's a quite slow operation
        /// \note The returned introspect ARE contextualized
        /// \note If this method is called on a contextualized introspect, it will only retrieve the "local" callees, and this is much faster than the other thing
        std::vector<introspect> get_callee_list() const;

        /// \brief Walks the callgraph to retrieve the caller list (the list of function that call this one)
        /// \note As "walk" can induce it, it's a quite slow operation
        /// \note The returned introspect ARE contextualized
        /// \note If this method is called on a contextualized introspect, it will only retrieve the "local" callers, and this is VERY much faster than the other thing
        std::vector<introspect> get_caller_list() const;

        /// \brief Return the list of root functions (like, main(), the functions used to start threads, ...)
        /// \note this is a static function, yo.
        static std::vector<introspect> get_root_function_list();

        // ---- // operations/getters on the method/function // ---- //

        /// \brief Return whether or not a context is in use or not
        inline bool is_contextual() const
        {
          return context != nullptr;
        }

        /// \brief Remove the context
        inline void remove_context()
        {
          context = nullptr;
        }

        /// \brief Return a copy without the context
        introspect copy_without_context() const
        {
          return introspect(call_info, call_info_index, nullptr);
        }

        /// \brief Set the context for this introspect object
        /// \param[in] caller MUST be a caller (as returned by get_caller_list()) and it MUST be contextualized.
        /// \return true if it's OK, false otherwise
        /// \note You can't contextualize root functions (as they haven't any callers).
        ///       The only way to do get a contextualized root is to retrieve it with get_root_function_list()
        bool set_context(const introspect &caller);

        /// \brief If this function returns false, results can be safely discarded
        /// It is based on the number of time the function has been called
        /// \note The return value is indicative only, BUT as it expect a minimum of 5 call, herm, I suppose that is a real strict minimum
        inline bool is_faithfull() const
        {
          if (context)
            return context->hit_count >= 5;
          return call_info.call_count >= 5;
        }

        /// \brief Return the number of time the function has been called
        inline size_t get_call_count() const
        {
          if (context)
            return context->hit_count;
          return call_info.call_count;
        }

        /// \brief Return the pretty name (if available, else, fall back to the crappy name)
        /// DO NOT DELETE / FREE the returned pointer
        const char *get_pretty_name() const
        {
          if (call_info.descr.pretty_name)
            return call_info.descr.pretty_name;
          return call_info.descr.name;
        }

        /// \brief Return the file
        /// \see get_line()
        /// \note could be nullptr if nothing is stored
        /// DO NOT DELETE / FREE the returned pointer
        const char *get_file() const
        {
          return call_info.descr.file;
        }

        /// \brief Return the line
        /// \see get_file()
        size_t get_line() const
        {
          return call_info.descr.line;
        }

        /// \brief Return the user-defined name (short-hand name)
        /// DO NOT DELETE / FREE the returned pointer
        /// \see get_pretty_name()
        /// \see set_name()
        const char *get_name() const
        {
          return call_info.descr.name;
        }

        /// \brief Set the shorthand name
        /// \note please don't touch the pointer after
        /// \note avoid using too much heap-allocated strings
        void set_name(const char *name)
        {
          call_info.descr.name = name;
        }

        /// \brief Return the function descriptor of the function. You could copy it but NEVER, NEVER change it.
        const func_descriptor &get_function_descriptor() const
        {
          return call_info.descr;
        }

        /// \brief return the duration progression of the self time
        std::deque<duration_progression> get_self_duration_progression() const
        {
          if (!context)
            return std::deque<duration_progression>();
          else
            return context->self_time_progression;
        }

        /// \brief return the duration progression of the global time
        std::deque<duration_progression> get_global_duration_progression() const
        {
          if (!context)
            return std::deque<duration_progression>();
          else
            return context->global_time_progression;
        }

        /// \brief Return the probability of a incoming failure
        /// \note Unlike get_failure_rate(), it will account all calls of the function
        /// \note This call only handle the function, not any of its possible sub-calls
        inline float get_failure_ratio() const
        {
          if (context)
            return float(context->fail_count) / float(context->hit_count);
          return float(call_info.fail_count) / float(call_info.call_count);
        }

        /// \brief Return the average duration of the function (and only that function, without the time consumed by sub calls)
        inline float get_average_self_duration() const
        {
          if (context)
            return context->average_self_time;
          return call_info.average_self_time;
        }

        /// \brief Return the average duration of the function (including all sub calls (functions called into this function)
        inline float get_average_duration() const
        {
          if (context)
            return context->average_global_time;
          return call_info.average_global_time;
        }

        /// \brief Return the last \e count errors for the function, most recent last
        /// \param[in] count The number of errors to return
        /// \note this method IS NOT context dependent, but always return errors from the global error list
        std::vector<reason> get_failure_reasons(size_t count = 10);

        /// \brief This function tests if the given function will be likely to fail (fail ratio > 0.5 by default) and returns an object with some properties
        /// to do some kind of conditional execution based on fails
        /// \note It use a get_failure_ratio() -like way to compute the failure rate (contextualized with the current stack, if any)
        /// \note It has a training time, it will return true until a certain amount of call has been reached. This amount can be set the with returned object.
        /// \note You can change the maximum ratio by setting it in the returned object.
        template<typename FuncType>
        internal::if_wont_fail<FuncType> if_wont_fail(const char *const name, FuncType func) const
        {
          size_t o_call_info_index;
          internal::call_info_struct &o_call_info = internal::get_call_info_struct<void>(func_descriptor {name}, &o_call_info_index);
          internal::stack_entry *o_context = nullptr;
          if (context)
            o_context = context->get_children_stack_entry(o_call_info_index);

          introspect child(o_call_info, o_call_info_index, o_context);
          if (!child.is_faithfull())
            child.remove_context();

          float ratio = child.get_failure_ratio();
          size_t count = child.get_call_count();

          return internal::if_wont_fail<FuncType>(count, ratio, func);
        }

      private:
        size_t call_info_index;
        internal::call_info_struct &call_info;
        internal::data *global;

        internal::stack_entry *context = nullptr;

        friend class function_call;
    };
  } // namespace r
} // namespace neam

#endif /*__N_468510512048024221_1859630211__INTROSPECT_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

