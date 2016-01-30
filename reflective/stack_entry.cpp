
#include "storage.hpp"
#include "stack_entry.hpp"


thread_local size_t thread_index = 0;


inline long int neam::r::internal::stack_entry::get_children_stack_entry_index(size_t call_info_struct_index) const
{
  data *global = get_global_data();

  // we don't lock anything ON PURPOSE.

  for (auto &it : children)
  {
    if (global->callgraph[thread_index][it].call_structure_index == call_info_struct_index)
      return (long)it;
  }
  return -1;
}

neam::r::internal::stack_entry &neam::r::internal::stack_entry::push_children_call_info(size_t call_info_struct_index)
{
  data *global = get_global_data();

  std::lock_guard<mutex_type> _u0(global->lock); // lock 'cause we do a search and create if not present.

  long index = get_children_stack_entry_index(call_info_struct_index);

  if (index < 0)
  {
    index = global->callgraph[thread_index].size();

    global->callgraph[thread_index].emplace_back(stack_entry{(size_t)index, call_info_struct_index, self_index});
    children.push_back(index);
    return global->callgraph[thread_index].back();
  }
  else
  {
    global->callgraph[thread_index][index].hit_count++;
    return global->callgraph[thread_index][index];
  }
}

neam::r::internal::stack_entry *neam::r::internal::stack_entry::get_children_stack_entry(size_t call_info_struct_index) const
{
  data *global = get_global_data();

  std::lock_guard<mutex_type> _u0(global->lock); // lock 'cause we do a search and create if not present.

  long index = get_children_stack_entry_index(call_info_struct_index);

  if (index >= 0)
    return &global->callgraph[thread_index][index];
  return nullptr;
}

neam::r::internal::stack_entry &neam::r::internal::stack_entry::initial_get_stack_entry(size_t call_info_struct_index)
{
  size_t index = 0;
  data *global = get_global_data();

  std::lock_guard<mutex_type> _u0(global->lock); // lock 'cause we do a search and create if not present.

  // the root stack_entry always have the first index (0), so we only loop over all first-level entries at test if the root has the good call_info_struct_index
  for (auto &it : global->callgraph)
  {
    if (it[0].call_structure_index == call_info_struct_index)
    {
      it[0].hit_count++; // increment the hit count
      thread_index = index;
      return it[0];
    }
    ++index;
  }

  // not found: create the new entry
  index = global->callgraph.size();
  thread_index = index;
  global->callgraph.emplace_back(); // call the default constructor

  // create the root element
  global->callgraph.back().emplace_back(stack_entry{0, call_info_struct_index, 0});

  return global->callgraph.back().back();
}

void neam::r::internal::stack_entry::dispose_initial()
{
  thread_index = 0;
}
