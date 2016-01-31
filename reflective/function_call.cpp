
#include <ctime>
#include "function_call.hpp"
#include "introspect.hpp"

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
  // TODO: thread safety
  if (self_time_monitoring)
  {
    call_info.average_self_time = (call_info.average_self_time * call_info.average_self_time_count + self_chrono.delta()) / (call_info.average_self_time_count + 1.);
    ++call_info.average_self_time_count;
    if (se)
    {
      se->average_self_time = (se->average_self_time * se->average_self_time_count + self_chrono.delta()) / (se->average_self_time_count + 1.);
      ++se->average_self_time_count;
    }
  }
  if (global_time_monitoring)
  {
    call_info.average_global_time = (call_info.average_global_time * call_info.average_global_time_count + global_chrono.delta()) / (call_info.average_global_time_count + 1.);
    ++call_info.average_global_time_count;
    if (se)
    {
      se->average_global_time = (se->average_global_time * se->average_global_time_count + global_chrono.delta()) / (se->average_global_time_count + 1.);
      ++se->average_global_time_count;
    }
  }

  // restore the previous context
  if (prev)
    prev->self_chrono.resume();
  else
    internal::stack_entry::dispose_initial();

  tl_data->top = prev;


  if (!prev) // TODO: conf for a real name
    neam::r::sync_data_to_disk(conf::out_file); // there's nothing after us, sync data to a file
}

void neam::r::function_call::fail(const neam::r::reason &rsn)
{
  call_info.fail_count++; // TODO: a more threadsafe thing

  if (se)
    se->fail_count++;

  // Avoid huge reports if we always hit the same error
  // We don't test the whole array 'cause we want to keep the ordering
  size_t ts = std::time(nullptr);
  if (call_info.fails.size() && call_info.fails.back() == rsn)
  {
    ++call_info.fails.back().hit;
    call_info.fails.back().last_timestamp = ts;
  }
  else
    call_info.fails.push_back(neam::r::reason{rsn.type, rsn.message, rsn.file, rsn.line, 1, ts, ts});
}

neam::r::introspect neam::r::function_call::get_introspect() const
{
  return neam::r::introspect(call_info, call_info_index, se);
}

void neam::r::internal::__addr__() {}
