
`reflective` (or `neam::r` ) is a program introspection library written in C++. ( **NOT** a language reflection framework, that would quite impossible in plain C++ only).

It can be used to create bug reports, monitor running programs across multiple runs, making self-introspecting programs, profiling, ...

### note

**reflective is not even in alpha stage and was started at most a week ago.**

to setup the cloned repository:
```
git submodule init
git submodule update
```

to build (on linux / macOS):
```
mkdir build
cd build
cmake ..
make
```

tested with gcc 5.2.1 and clang 3.7.0
All you need is a c++14 compiler, a c++14 STL, `sigaction()` and `time()` support in your OS (so, no MS windows as it does not provides `sigaction`)

### versus valgrind (memcheck, ca{che,ll}grind)

Reflective is faster, but can't do the same job: valgrind memcheck will track memory errors, a task that reflective can't do;
cachegrind will profile the cache whereas reflective don't;
callgrind will create a nice callgraph without any modification of your program (except running it in debug mode, and way slower),
reflective will also create a callgraph but asking the dev to add some code to make it working (one line of code per function to monitor).

But if you have a program with a lot of computing (like a game), running in valgrind is sometime not an option (more notably when the game is run by alpha/beta testers).
But you still want to track down bugs and performance issues. (and who never stumbled upon a crash/... that randomly appear, but NEVER in a memory error detector/profiling tool ?)
A program can also be used with reflective while being build in release mode.

reflective also "dumps" its memory into a file, expanding it as the program run, allowing it to gather and compile a lot of information about how the program run.
This information is also available at runtime, while the program is running.

The user can also report custom errors or warnings, expanding the range of possibilities or reflective.

### versus gprof

Reflective is probably slower, but load and save its data, expanding it at each time the program runs. It also saves a full callgraph and does not use statistical sampling.
Reflective also report errors, call count, average self/global times on instrumented functions and only monitor a limited set of functions.
A program can also be used with reflective while being build in release mode, and the generated data is available for the program to use at runtime.

### code examples

```c++
#include <reflective/reflective.hpp>

namespace a
{
  class b
  {
    public:
      void g();

      void f()
      {
        // this line is what makes reflective working
        neam::r::function_call self_call(N_PRETTY_FUNCTION_INFO(a::b::f));
        // NOTE: you should always provide the whole path to the function, like `a::b::f`

        g();
      }
  };
} // namespace a
```

```c++
#include <vector>
#include <reflective/reflective.hpp>

void f()
{
  // create the introspect object, used for introspecting functions and methods
  neam::r::introspect itp("a::b::f");
  // could as well be `neam::r::introspect(N_FUNCTION(a::b::f))` which is faster
  // (but less flexible as the definition of a::b::f is needed)

  // get the time consumed by the function (less the time consumed by the functions it calls)
  itp.get_average_self_duration();

  // retrieve from the callgraph all the functions that call `a::b::f`
  std::vector<neam::r::introspect> itp.get_caller_list();

  // conditional execution based on the past failures
  itp.if_wont_fail(N_FUNCTION(a::b::g))
    .call() // call a::b::g() if it's OK
    .otherwise([&]() // do something else if it's not.
    {
      my_safer_function();
    });

  // retrieve the last 100 failure reasons of `a::b::f`
  // a failure reason (the `neam::r::reason` object) contain the failure type
  //  (death by signal, syscall failure, file not found, exception, ...),
  // the file and line where the error has occurred, a message, the number of
  // consecutive time the error has been raised and the timestamp range.
  // the list of failure is guaranteed to be strictly ordered in time
  // (no timestamp range can overlap).
  std::vector<neam::r::reason> failure_reasons = itp.get_failure_reasons(100);
}

```

### future / TODO

- create tools to read and output data/information from reflective save file
- add introspection abilities for monitoring regression on duration times
- add conditional execution based on the average duration time.
- fix the lambda support
- fix the multi-threading support (add mutexes)
- benchmark the impact on the worst case of reflective (calling repetitively one empty function and calling a lot of different empty functions)

### author

Timothée Feuillet (_neam_ or _tim42_).
