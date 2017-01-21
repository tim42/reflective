
#include <algorithm>
#include <ctime>
#include <exception>
#include "tools/logger/logger.hpp"
#include "function_call.hpp"
#include "introspect.hpp"

void neam::r::function_call::common_init()
{
  {
    std::lock_guard<internal::mutex_type> _u0(global->lock);
    ++call_info.call_count;
  }

  prev = tl_data->top;
  se = nullptr;
  if (prev)
  {
    if (prev->self_time_monitoring)
      prev->self_chrono.pause(); // pause the previous self-chrono

    // stack_entry things
    if (prev->se)
      se = &prev->se->push_children_call_info(call_info_index);
  }
  else
    se = &internal::stack_entry::initial_get_stack_entry(call_info_index);

  if (conf::watch_uncaught_exceptions && std::uncaught_exception())
    has_exception = true;

  tl_data->top = this;
}

neam::r::function_call::~function_call()
{
  if (tl_data->top != this)
    return;

  size_t ts = std::time(nullptr);

  // Save the time monitoring (global & self)
  // TODO: thread safety
  if (self_time_monitoring)
  {
    const double delta = self_chrono.get_accumulated_time();
    {
      std::lock_guard<internal::mutex_type> _u0(global->lock);
      size_t mcount = call_info.average_self_time_count;
      if (conf::sliding_average)
        mcount = std::min(mcount, conf::past_average_weight);

      call_info.average_self_time = (call_info.average_self_time * mcount + delta) / (mcount + 1.);
      ++call_info.average_self_time_count;
    }
    if (se)
    {
      size_t mcount = se->average_self_time_count;
      if (conf::sliding_average)
        mcount = std::min(mcount, conf::past_average_weight);

      se->average_self_time = (se->average_self_time * mcount + delta) / (mcount + 1.);
      ++se->average_self_time_count;

      if (se->self_time_progression.empty())
        se->self_time_progression.push_back(duration_progression{ts, se->average_self_time});
      else if ((se->self_time_progression.back().value < se->average_global_time && se->self_time_progression.back().value * conf::progression_min_factor < se->average_self_time)
               || (se->self_time_progression.back().value > se->average_global_time && se->self_time_progression.back().value > se->average_self_time * conf::progression_min_factor))
        se->self_time_progression.push_back(duration_progression{ts, se->average_self_time});
      if (se->self_time_progression.size() > conf::max_progression_entries)
      {
        size_t diff = se->self_time_progression.size() - conf::max_progression_entries;
        se->self_time_progression.erase(se->self_time_progression.begin(), se->self_time_progression.begin() + diff);
      }
    }
  }
  if (global_time_monitoring)
  {
    const double delta = global_chrono.get_accumulated_time();
    {
      std::lock_guard<internal::mutex_type> _u0(global->lock);

      size_t mcount = call_info.average_global_time_count;
      if (conf::sliding_average)
        mcount = std::min(mcount, conf::past_average_weight);
      call_info.average_global_time = (call_info.average_global_time * mcount + delta) / (mcount + 1.);
      ++call_info.average_global_time_count;
    }
    if (se)
    {
      size_t mcount = se->average_global_time_count;
      if (conf::sliding_average)
        mcount = std::min(mcount, conf::past_average_weight);
      se->average_global_time = (se->average_global_time * mcount + delta) / (mcount + 1.);
      ++se->average_global_time_count;

      if (se->global_time_progression.empty())
        se->global_time_progression.push_back(duration_progression{ts, se->average_global_time});
      else if ((se->global_time_progression.back().value < se->average_global_time && se->global_time_progression.back().value * conf::progression_min_factor < se->average_global_time)
               || (se->global_time_progression.back().value > se->average_global_time && se->global_time_progression.back().value > se->average_global_time * conf::progression_min_factor))
        se->global_time_progression.push_back(duration_progression{ts, se->average_global_time});
      if (se->self_time_progression.size() > conf::max_progression_entries)
      {
        size_t diff = se->self_time_progression.size() - conf::max_progression_entries;
        se->self_time_progression.erase(se->self_time_progression.begin(), se->self_time_progression.begin() + diff);
      }
    }
  }

  if (conf::watch_uncaught_exceptions && std::uncaught_exception() && !has_exception)
  {
    fail(exception_reason(call_info.descr.file, call_info.descr.line, "function termination due to an uncaught exception in this function"));
    if (prev)
      prev->has_exception = true;
  }

  // restore the previous context
  if (prev && prev->self_time_monitoring)
    prev->self_chrono.resume();
  else
    internal::stack_entry::dispose_initial();

  if (tl_data->top == this)
    tl_data->top = prev;

  if (!prev && !conf::disable_auto_save)
    neam::r::sync_data_to_disk(conf::out_file); // there's nothing after us, sync data to a file
}

void neam::r::function_call::fail(const neam::r::reason &rsn)
{
  if (conf::print_fails_to_stdout)
  {
    neam::cr::out.error() << LOGGER_INFO_TPL(rsn.file, rsn.line) << rsn.type << ": "  << rsn.message << std::endl;
  }

  if (se)
    se->fail_count++;

  // Avoid huge reports if we always hit the same error
  // We don't test the whole array 'cause we want to keep the ordering
  size_t ts = std::time(nullptr);
  {
    std::lock_guard<internal::mutex_type> _u0(global->lock);

    call_info.fail_count++;
  }

  if (!se) return;

  if (se->fails.size() && se->fails.back() == rsn)
  {
    ++se->fails.back().hit;
    se->fails.back().last_timestamp = ts;
  }
  else
    se->fails.push_back(neam::r::reason {rsn.type, rsn.message, rsn.file, rsn.line, 1, ts, ts});
}

void neam::r::function_call::report(const std::string &mode, const neam::r::reason &rsn)
{
  if (conf::print_reports_to_stdout)
  {
    neam::cr::out.log() << LOGGER_INFO_TPL(rsn.file, rsn.line) << mode << ": " << rsn.type << ": " << rsn.message << std::endl;
  }

  // Avoid huge reports if we always hit the same thing
  // We don't test the whole array 'cause we want to keep the ordering
  size_t ts = std::time(nullptr);

  if (!se) return;

  auto &vct = se->reports[mode];

  if (vct.size() && vct.back() == rsn)
  {
    ++vct.back().hit;
    vct.back().last_timestamp = ts;
  }
  else
    vct.push_back(neam::r::reason {rsn.type, rsn.message, rsn.file, rsn.line, 1, ts, ts});
}

neam::r::sequence &neam::r::function_call::create_sequence(const std::string &name)
{
  // TODO(tim): fix the possible null se pointer
  neam::r::sequence &ret = se->sequences[name];
  ret.clear_sequence();
  return ret;
}

neam::r::sequence *neam::r::function_call::get_sequence(const std::string &name)
{
  if (!se)
    return nullptr;
  auto it = se->sequences.find(name);
  if (it != se->sequences.end())
    return &it->second;
  return nullptr;
}

neam::r::sequence *neam::r::function_call::get_sequence_callers(const std::string &name)
{
  for (neam::r::function_call *it = tl_data->top; it; it = it->prev)
  {
    neam::r::sequence *ptr = it->get_sequence(name);
    if (ptr)
      return ptr;
  }
  return nullptr;
}

void neam::r::function_call::remove_sequence(const std::string &name)
{
  se->sequences.erase(name);
}

neam::r::introspect neam::r::function_call::get_introspect() const
{
  return neam::r::introspect(call_info, call_info_index, se);
}

void neam::r::internal::__addr__() {}
