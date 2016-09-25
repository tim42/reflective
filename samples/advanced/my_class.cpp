
#include "my_class.hpp"

#include <reflective/reflective.hpp> // The reflective header


sample::my_class::my_class()
{
  neam::r::function_call self_call(N_PRETTY_NAME_INFO("sample::my_class::my_class()"));
}

sample::my_class::my_class(const sample::my_class &o)
 : value(o.value)
{
  neam::r::function_call self_call(N_PRETTY_NAME_INFO("sample::my_class::my_class(my_class &)"));
}

sample::my_class &sample::my_class::operator=(const sample::my_class &o)
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(sample::my_class::operator=));

  if (this != &o)
  {
    value = o.value;

    try
    {
      my_function(get_top_value());
    }
    catch (std::exception &e)
    {
      self_call.fail(neam::r::exception_reason(N_REASON_INFO, e.what()));
    }
    catch (...)
    {
      self_call.fail(neam::r::exception_reason(N_REASON_INFO, "unknown exception caught"));
    }
  }
  return *this;
}

sample::my_class::~my_class()
{
  neam::r::function_call self_call(N_PRETTY_NAME_INFO("sample::my_class::~my_class"));
}


void sample::my_class::my_function()
{
  neam::r::function_call self_call(N_PRETTY_NAME_INFO("sample::my_class::my_function()"));

  try
  {
    my_function(true);
  }
  catch (std::exception &e)
  {
    self_call.fail(neam::r::exception_reason(N_REASON_INFO, e.what()));
  }
  catch (...)
  {
    self_call.fail(neam::r::exception_reason(N_REASON_INFO, "unknown exception caught"));
  }
}

void sample::my_class::my_function(bool value)
{
  neam::r::function_call self_call(N_PRETTY_NAME_INFO("sample::my_class::my_function(bool)"));

  if (value == get_top_value())
  {
    may_throw(0);
  }
}

bool sample::my_class::get_top_value() const
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(sample::my_class::get_top_value));
  return value.back();
}

void sample::my_class::push(bool val)
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(sample::my_class::push));
  value.push_back(val);
}

void sample::my_class::may_throw(int i)
{
  neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(sample::my_class::may_throw));
  value.at(i) = !value.at(i + 1);
}

