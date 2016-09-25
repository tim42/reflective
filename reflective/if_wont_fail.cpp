
#include "function_call.hpp"
#include "if_wont_fail.hpp"

void neam::r::internal::report_exception(std::exception &e)
{
  // If you've came here because of an error, well, the only function calling this one is if_wont_fail<...>::then and if_wont_fail<...>::otherwise
  // It is called when the lambda / function / ... throws an exception, it reportis the error and then the exception continue its walk
  function_call::get_active_function_call()->fail(exception_reason(N_REASON_INFO, e.what()));
}

void neam::r::internal::report_unknown_exception()
{
  // If you've came here because of an error, well, the only function calling this one is if_wont_fail<...>::then and if_wont_fail<...>::otherwise
  // It is called when the lambda / function / ... throws an exception, it reportis the error and then the exception continue its walk
  function_call::get_active_function_call()->fail(exception_reason(N_REASON_INFO, "unknown exception caught"));
}

