
#include "function_call.hpp"

void neam::r::function_call::common_init()
{
  ++call_info.call_count; // TODO: a more threadsafe thing

  prev = tl_data->top;
  if (prev)
  {
    prev->self_chrono.pause(); // pause the previous self-chrono

    // stack_entry things
    if (prev->se)
      se = &prev->se->push_children_call_info(call_info_index);
  }
  else
    se = &internal::stack_entry::initial_get_stack_entry(call_info_index);

  tl_data->top = this;

}

neam::r::function_call::~function_call()
{
  // Save the time monitoring (global & self)
  if (self_time_monitoring)
  {
    call_info.average_self_time = (call_info.average_self_time * call_info.average_self_time_count + self_chrono.delta()) / (call_info.average_self_time_count + 1.);
    ++call_info.average_self_time_count;
  }
  if (global_time_monitoring)
  {
    call_info.average_global_time = (call_info.average_global_time * call_info.average_global_time_count + global_chrono.delta()) / (call_info.average_global_time_count + 1.);
    ++call_info.average_global_time_count;
  }

  // restore the previous context
  if (prev)
    prev->self_chrono.resume();
  else
    internal::stack_entry::dispose_initial();

  tl_data->top = prev;


  if (!prev) // TODO: conf for a real name
    neam::r::sync_data_to_disk("file.xr"); // there's nothing after us, sync data to a file
}

void neam::r::function_call::fail(const neam::r::reason &rsn)
{
  call_info.fail_count++; // TODO: a more threadsafe thing

  if (se)
    se->fail_count++;

  // Avoid huge reports if we always hit the same error
  // We don't test the whole array 'cause we want to keep the ordering
  if (call_info.fails.size() && call_info.fails.back() == rsn)
    ++call_info.fails.back().hit;
  else
    call_info.fails.push_back(rsn);
}


void neam::r::internal::__addr__() {}
