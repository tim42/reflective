//
// file : persistence_metadata.hpp
// in : file:///home/tim/projects/reflective/reflective/persistence_metadata.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 29/01/2016 16:26:38
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

#ifndef __N_4809451401143654794_770423445__PERSISTENCE_METADATA_HPP__
# define __N_4809451401143654794_770423445__PERSISTENCE_METADATA_HPP__

#include <persistence/persistence/persistence.hpp>

#include "reason.hpp"
#include "call_info_struct.hpp"
#include "stack_entry.hpp"
#include "storage.hpp"


// This file will hold all serialization metadata for reflective
// This file should not be included except by storage.cpp

namespace neam
{
  namespace cr
  {
    // // reason // //
    NCRP_DECLARE_NAME(r__reason, type);
    NCRP_DECLARE_NAME(r__reason, message);
    NCRP_DECLARE_NAME(r__reason, file);
    NCRP_DECLARE_NAME(r__reason, line);
    NCRP_DECLARE_NAME(r__reason, hit);
    NCRP_DECLARE_NAME(r__reason, initial_timestamp);
    NCRP_DECLARE_NAME(r__reason, last_timestamp);
    template<typename Backend> class persistence::serializable<Backend, r::reason> : public persistence::serializable_object
    <
      Backend, // < the backend (here: all backends)

      r::reason, // < the class type to handle

      // simply list here the members you want to serialize / deserialize
      NCRP_NAMED_TYPED_OFFSET(r::reason, type, names::r__reason::type),
      NCRP_NAMED_TYPED_OFFSET(r::reason, message, names::r__reason::message),
      NCRP_NAMED_TYPED_OFFSET(r::reason, file, names::r__reason::file),
      NCRP_NAMED_TYPED_OFFSET(r::reason, line, names::r__reason::line),
      NCRP_NAMED_TYPED_OFFSET(r::reason, hit, names::r__reason::hit),
      NCRP_NAMED_TYPED_OFFSET(r::reason, initial_timestamp, names::r__reason::initial_timestamp),
      NCRP_NAMED_TYPED_OFFSET(r::reason, last_timestamp, names::r__reason::last_timestamp)
    > {};

    // // data // //
    NCRP_DECLARE_NAME(r__data, launch_count);
    NCRP_DECLARE_NAME(r__data, func_info);
    NCRP_DECLARE_NAME(r__data, callgraph);
    template<typename Backend> class persistence::serializable<Backend, r::internal::data> : public persistence::constructible_serializable_object
    <
      Backend, // < the backend (here: all backends)

      r::internal::data, // < the class type to handle

      // Embed in the template a call to the post-deserialization function
      // This function will be called just after the object has been deserialized
      N_CALL_POST_FUNCTION(r::internal::data),

      // simply list here the members you want to serialize / deserialize
      NCRP_NAMED_TYPED_OFFSET(r::internal::data, launch_count, names::r__data::launch_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::data, func_info, names::r__data::func_info),
      NCRP_NAMED_TYPED_OFFSET(r::internal::data, callgraph, names::r__data::callgraph)
    > {};

    // // call_info_struct // //
    NCRP_DECLARE_NAME(r__call_info_struct, name);
    NCRP_DECLARE_NAME(r__call_info_struct, name_hash);
    NCRP_DECLARE_NAME(r__call_info_struct, pretty_name);
    NCRP_DECLARE_NAME(r__call_info_struct, call_count);
    NCRP_DECLARE_NAME(r__call_info_struct, fail_count);
    NCRP_DECLARE_NAME(r__call_info_struct, average_self_time);
    NCRP_DECLARE_NAME(r__call_info_struct, average_self_time_count);
    NCRP_DECLARE_NAME(r__call_info_struct, average_global_time);
    NCRP_DECLARE_NAME(r__call_info_struct, average_global_time_count);
    NCRP_DECLARE_NAME(r__call_info_struct, fails);
    template<typename Backend> class persistence::serializable<Backend, r::internal::call_info_struct> : public persistence::serializable_object
    <
      Backend, // < the backend (here: all backends)

      r::internal::call_info_struct, // < the class type to handle

      // simply list here the members you want to serialize / deserialize
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, name, names::r__call_info_struct::name),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, name_hash, names::r__call_info_struct::name_hash),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, pretty_name, names::r__call_info_struct::pretty_name),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, fail_count, names::r__call_info_struct::fail_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, call_count, names::r__call_info_struct::call_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, average_self_time, names::r__call_info_struct::average_self_time),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, average_self_time_count, names::r__call_info_struct::average_self_time_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, average_global_time, names::r__call_info_struct::average_global_time),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, average_global_time_count, names::r__call_info_struct::average_global_time_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::call_info_struct, fails, names::r__call_info_struct::fails)
    > {};

    // // stack_entry // //
    NCRP_DECLARE_NAME(r__stack_entry, self_index);
    NCRP_DECLARE_NAME(r__stack_entry, stack_index);
    NCRP_DECLARE_NAME(r__stack_entry, call_structure_index);
    NCRP_DECLARE_NAME(r__stack_entry, hit_count);
    NCRP_DECLARE_NAME(r__stack_entry, fail_count);
    NCRP_DECLARE_NAME(r__stack_entry, average_self_time);
    NCRP_DECLARE_NAME(r__stack_entry, average_self_time_count);
    NCRP_DECLARE_NAME(r__stack_entry, average_global_time);
    NCRP_DECLARE_NAME(r__stack_entry, average_global_time_count);
    NCRP_DECLARE_NAME(r__stack_entry, parent);
    NCRP_DECLARE_NAME(r__stack_entry, children);
    template<typename Backend> class persistence::serializable<Backend, r::internal::stack_entry> : public persistence::serializable_object
    <
      Backend, // < the backend (here: all backends)

      r::internal::stack_entry, // < the class type to handle

      // simply list here the members you want to serialize / deserialize
//       NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, self_index, names::r__stack_entry::self_index),
//       NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, stack_index, names::r__stack_entry::stack_index),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, call_structure_index, names::r__stack_entry::call_structure_index),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, hit_count, names::r__stack_entry::hit_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, fail_count, names::r__stack_entry::fail_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, average_self_time, names::r__stack_entry::average_self_time),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, average_self_time_count, names::r__stack_entry::average_self_time_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, average_global_time, names::r__stack_entry::average_global_time),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, average_global_time_count, names::r__stack_entry::average_global_time_count),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, parent, names::r__stack_entry::parent),
      NCRP_NAMED_TYPED_OFFSET(r::internal::stack_entry, children, names::r__stack_entry::children)
    > {};
  } // namespace cr
} // namespace neam

#endif /*__N_4809451401143654794_770423445__PERSISTENCE_METADATA_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

