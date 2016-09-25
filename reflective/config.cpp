
#include <cstddef>

namespace neam
{
  namespace r
  {
    namespace conf
    {
      bool monitor_self_time = false;
      bool monitor_global_time = false;
      const char *out_file = "./.out.nr";
      bool disable_auto_save = false;
      bool cleanup_on_crash = true;

      bool watch_uncaught_exceptions = true;
      bool integrate_with_n_debug = true;
      bool print_fails_to_stdout = false;
      bool print_reports_to_stdout = false;

      bool sliding_average = true;
      size_t past_average_weight = 10;

      float progression_min_factor = 10.f;
      size_t max_progression_entries = 25;

      long max_stash_count = 5;
    } // namespace conf
  } // namespace r
} // namespace neam
