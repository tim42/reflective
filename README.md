
**NOTE:** REFLECTIVE (or `neam::r` ) is **NOT** a reflection framework for C++ like it could exists in Java, PHP, ..., ... _(It would be kind of horrific in term of code and usability)_

----

**REFLECTIVE** is a framework providing to a program the ability to know about himself, its fails and bugs, its performance flaws.
It _is_ a reflection framework, but for compiled code behavior only.


## features

- values are stored in a database (see the project _neam::persistence_ )
- program speed tracer (compare speed across runs, and allow making assumption on it)
- program fail tracer (log and save failures, allows making assumptions on it)

## examples

### using function calls for testing success and failures
```C++
// for C-like functions
void my_function(int arg, char c, double f)
{
  neam::r::function_call<my_function> self_call(arg, c, f);

  void *buff = malloc(100); // yep, malloc in C++ :)
  if (!buff)
    return self_call.has_failed<malloc>(neam::r::reason, "allocation failure");
  //...
  return self_call.success();
}

struct my_struct
{
  void *my_function(int i, float f)
  {
    neam::r::function_call<my_struct::my_function> self_call(this, arg, c, f);

    void *buff = malloc(100 * (i * i + 1)); // yep, malloc in C++ :)
    if (!buff)
      return self_call.fail<malloc>(neam::r::reason, "allocation failure", nullptr);

    // ...
    return self_call.success(buff);
  }


  // using if_wont_fail() and may_fail:
  void my_fnc_2()
  {
    neam::r::function_call<my_struct::my_fnc_2> self_call(this);

    // test whether or not the call of my_struct::my_function would fail (faillure rate > neam::r::reflective::max_failure_rate)
    void *buff = self_call.if_call_wont_fail<my_struct::my_function>(this, 104242, -4.2f).call();

    if (self_call.call_wont_fail<malloc>(neam::r::reason, "allocation failure"))
    {
      void *sec_buff = malloc(100000);
      if (!sec_buff)
      {
        free(sec_buff);
        return self_call.fail<malloc>(neam::r::reason, "allocation failure");
      }
    }

    // ...

    free(buff);

    return self_call.success();
  }
};
```

### unsing function_call for estimating speed, ETA, slow-downs across runs

First way: test the whole function time
```C++
void my_func()
{
  neam::r::function_call<my_func> self_call;
  self_call.disable_success_reports();  // here we don't care about success or failure
  self_call.enable_delay_reports();     // but we want all the delay, speed reporting stuff

  // post reports
  neam::r::speed_report report = self_call.report_speed([&]()
  {
    my_slow_function(/* with some parameters */);
  });

  if (report.is_becoming_slower())
    std::cout << "dear develloper: please do something for your program speed !!!" << std::endl;

  // guessing reports
  auto func = [&]()
  {
    my_other_slow_function(/* with some parameters */);
  };
  neam::r::speed_report pre_report = self_call.guess_speed_report(func);

  if (pre_report.get_elapsed_time() < 0.5)
  {
    neam::r::speed_report post_report = self_call.report_speed(func);
    if (post_report.is_becoming_slower(pre_report))
    {
      std::cout << "dear develloper: please do something for your program speed !!!"
                << "(guessing delay: " << pre_report.get_elapsed_time()
                << ", real: " << post_report.get_elapsed_time() << ")" << std::endl;
    }
  }
}
```
