
#include <algorithm>

#include "tools/logger/logger.hpp"
#include "call_info_struct.hpp"
#include "function_call.hpp"
#include "measure_point.hpp"
#include "config.hpp"

void neam::r::measure_point::_save()
{
  function_call *cfc = function_call::get_active_function_call();
  if (!cfc)
  {
    neam::cr::out.debug() << LOGGER_INFO << "could not save measure point '" << name << "': no active function_call object." << std::endl;
    return;
  }

  internal::call_info_struct &cis = cfc->call_info;
  measure_point_entry &mpe = cis.measure_points[name];

  size_t mcount = mpe.hit_count;
  if (conf::sliding_average)
    mcount = std::min(mcount, conf::past_average_weight);

  // update the stored value
  mpe.value = (mpe.value * mcount + value) / (mcount + 1);
  ++mpe.hit_count;
}

double neam::r::measure_point::get_average_time() const
{
  function_call *cfc = function_call::get_active_function_call();
  if (!cfc)
    return 0.;

  internal::call_info_struct &cis = cfc->call_info;
  measure_point_entry &mpe = cis.measure_points[name];

  return mpe.value;
}
