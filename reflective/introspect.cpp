
#include "stack_entry.hpp"
#include "introspect.hpp"

neam::r::introspect::introspect(const neam::r::introspect &o)
  : call_info_index(o.call_info_index), call_info(o.call_info), global(o.global), context(o.context)
{
}

neam::r::introspect &neam::r::introspect::operator=(const neam::r::introspect &o)
{
  call_info = o.call_info;
  call_info_index = o.call_info_index;
  global = o.global;
  context = o.context;
  return *this;
}


void neam::r::introspect::reset()
{
  call_info.fail_count = 0;
  call_info.call_count = 1;
  call_info.average_global_time_count = call_info.average_global_time_count ? 1 : 0;
  call_info.average_self_time_count = call_info.average_self_time_count ? 1 : 0;
  call_info.fails.clear();

  // reset in all the callgraph entries
  std::lock_guard<internal::mutex_type> _u0(global->lock); // lock 'cause we do a lot of nasty things.

  for (auto &graph_it : global->callgraph)
  {
    for (internal::stack_entry &it : graph_it)
    {
      if (it.call_structure_index == call_info_index)
      {
        it.fail_count = 0;
        it.hit_count = 1;
        it.average_global_time_count = it.average_global_time_count ? 1 : 0;
        it.average_self_time_count = it.average_self_time_count ? 1 : 0;
      }
    }
  }
}

std::vector<neam::r::introspect> neam::r::introspect::get_callee_list() const
{
  std::vector<neam::r::introspect> ret;

  std::lock_guard<internal::mutex_type> _u0(global->lock); // lock 'cause we do a lot of nasty things.

  if (!context) // the global version
  {
    for (auto & graph_it : global->callgraph)
    {
      for (internal::stack_entry & it : graph_it)
      {
        if (it.call_structure_index == call_info_index)
        {
          for (size_t callee_idx : it.children)
          {
            internal::stack_entry &callee = graph_it[callee_idx];

            ret.emplace_back(introspect(internal::get_call_info_struct_at_index(callee.call_structure_index), callee.call_structure_index, &callee));
          }
        }
      }
    }
  }
  else
  {
    for (size_t &callee_idx : context->children)
    {
      internal::stack_entry &callee = global->callgraph[context->stack_index][callee_idx];

      ret.emplace_back(introspect(internal::get_call_info_struct_at_index(callee.call_structure_index), callee.call_structure_index, &callee));
    }
  }
  return ret;
}

std::vector<neam::r::introspect> neam::r::introspect::get_caller_list() const
{
  std::vector<neam::r::introspect> ret;

  std::lock_guard<internal::mutex_type> _u0(global->lock); // lock 'cause we do a lot of nasty things.

  if (!context) // the global version
  {
    for (auto & graph_it : global->callgraph)
    {
      for (internal::stack_entry & it : graph_it)
      {
        if (it.call_structure_index == call_info_index)
        {
          size_t caller_idx = it.parent;
          internal::stack_entry &caller = graph_it[caller_idx];

          ret.emplace_back(introspect(internal::get_call_info_struct_at_index(caller.call_structure_index), caller.call_structure_index, &caller));
        }
      }
    }
  }
  else if (context->parent == context->self_index || context->self_index == 0)
  {
    internal::stack_entry &caller = global->callgraph[context->stack_index][context->parent];

    ret.emplace_back(introspect(internal::get_call_info_struct_at_index(caller.call_structure_index), caller.call_structure_index, &caller));
  }
  return ret;
}

std::vector<neam::r::introspect> neam::r::introspect::get_root_function_list()
{
  std::vector<neam::r::introspect> ret;
  internal::data *global = internal::get_global_data();

  std::lock_guard<internal::mutex_type> _u0(global->lock); // lock 'cause we do a lot of nasty things.

  for (auto &graph_it : global->callgraph)
  {
    if (graph_it.size())
    {
      ret.emplace_back(introspect(internal::get_call_info_struct_at_index(graph_it[0].call_structure_index), graph_it[0].call_structure_index, &graph_it[0]));
    }
  }
  return ret;
}

// --- //

bool neam::r::introspect::set_context(const neam::r::introspect &caller)
{
  if (!caller.context)
    return false;

  context = caller.context->get_children_stack_entry(call_info_index);
  return (!!context);
}


std::vector<neam::r::reason> neam::r::introspect::get_errors(size_t count)
{
  std::vector<neam::r::reason> ret;
  count = std::min(count, call_info.fails.size());
  ret.reserve(count);

  for (size_t i = call_info.fails.size() - count; i < call_info.fails.size(); ++i)
    ret.push_back(call_info.fails[i]);

  return ret;
}

