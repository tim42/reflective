
`reflective` (or `neam::r` ) is a program introspection/profiling library written in C++. ( **NOT** a language reflection framework, that would quite impossible in plain C++ only).

It can be used to create bug reports, monitor programs across multiple runs, making self-introspecting programs, profiling, ...

### note

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
All you need is a c++14 compiler, a c++14 STL, `sigaction()` (or `signal()`) and `time()` support in your OS

**important note**: because of the nature of constructors and destructors, it is really difficult to get correct graph when the call is done in a initializer list / default {de,con}structor.
The call is attributed to the caller. (that could have weird effects on the graph when a function delete polymorphic objects and end-up being reported as calling different methods that in fact are called implicitly by destructors).
If you can't do without a 100% correct callgraph (and mostly the fact that a call can be assigned to a caller instead of a {de,con}structor, well, use a slower tool).
Currently reflective only uses the STL and some utilities that I've made (that only uses STL), and you can't get correct results on {de,con}structors with only that.
I am working on a solution to fix this, but if it add too much complexity to reflective it may well never see daylight.

I've run reflective with YägGLer (a weird renderer that I wrote) in a multi-threaded, CPU intensive program, and there was no framerate drop (moreover the scheduler wasn't late to finish a frame).


### features

 - profiling: reflective generate a lot of timing data: time spent in the function itself (excluding any sub-calls), time spent in the function, including sub-calls, number of time called,
   timing progression (useful if things get slower with time), a complete callgraph with timing information can be generated (see the picture below), ...
   The user can even monitor specific code sections to get a more complete profile information.
 - error reporting: reflective can report non-fatal failures as well as fatal ones (like segmentation fault, abort() calls, ...).
   This way, if a function spuriously triggers a segmentation fault or generates a failure most of the time, you will know it.
 - persistent: data is saved and loaded to/from a file. This way to can get comprehensive crash reports (or bug reports, or performance issue reports) even when the user don't
   have any profiling tool on his machine. The data can even be stashed when an update of the binary is performed (this way you don't loose previous analysis).
 - introspection: the program can know about himself (a bit like when using gprof, valgrind, ... the program could change its behavior at runtime from the data of those tools)
   All the tools are written using the introspection API **only**.
 - Tools. reflective have tools (to generate a callgraph, to get specific information about a specific function, ...). Bonus, your program can generate itself its callgraph.
 - multi-threading support: how can this even be an option ?
 - and it is fast. On my computer (the cpu is an Intel i7 3630QM), the slowest operation (creating a `function_call` object) takes on average 100 to 300 nanoseconds:
   Only the first call of a function costs something.

### reflective versus valgrind (memcheck, ca{che,ll}grind)

Reflective is faster, but can't do the same job: valgrind memcheck will track memory errors, a task that reflective can't do;
cachegrind will profile the cache whereas reflective don't;
callgrind will create a nice callgraph without any modification of your program (except running it in debug mode, and way slower),
reflective will also create a callgraph but will ask the dev to add some code to make it working (one line of code per function to monitor).

But if you have a program with a lot of computing (like a game), running in valgrind is sometime not an option (more notably when the game is run by alpha/beta testers).
But you still want to track down bugs and performance issues. (and who never stumbled upon a crash/... that randomly appear, but NEVER in a memory error detector/profiling tool ?)
A program can also be used with reflective while being build in release mode.

reflective also "dumps" its memory into a file, expanding it as the program run, allowing it to gather and compile a lot of information about how the program run.
This information is also available at runtime, while the program is running.

The user can also report custom errors or warnings, expanding the range of possibilities or reflective.

### reflective versus gprof / oprofile

Reflective may be a tiny bit slower, but could load and save its data, expanding it at each time the program runs. It also saves a full callgraph and does not use statistical sampling.
Reflective also report errors, call count, average self/global times on instrumented functions and can only monitor a limited set of functions.
A program can also be used with reflective while being build in release mode, heavy inlining enabled, while having the generated data available for the program to use at runtime.

### tools

#### callgraph2dot

`callgraph2dot` transforms a reflective output file into a dot graph, adding to it some information from reflective (like the call count, self time, global time).
It can perform some branch pruning to remove superfluous branches in the callgraph (this help doing performance analysis) and can also report errors and their path inside the callgraph.

this tool is important as it allows a visual preview of the data stored on-disk by persistence.

Example of an output dot graph (with branch pruning activated): ![callgraph](http://i.imgur.com/npRY6gQ.png)

#### reflective-shell

`reflective-shell` allow an user to get fine grained information from a reflective save/out file. Every piece of information that is collected by reflective is made available by this tool.
It has a readme explaining how to use it [here](tools/shell/README.md)

Why a shell ? I needed something easy to do, that has enough flexibility and was easy enough to use _and_ provide all the information someone could want to have.
 - A GUI is not something easy to create. Moreover, you can't decently put a lot of option and keep the UX good enough.
 - A SQL-like language would have been OK, but this would have generated a lot more work.
The shell is simple (cd, ls, info, mode), easy to use (a directory-like structure), easy to create, with enough functionality (--options) and possible customizations (scripts).

Moreover, in a near future you will be able to completely automate the process with scripts.


### future / TODO

- add warning and info level reports (like there is "fail" level reports)
- add conditional execution based on the average duration time.
- benchmark the impact on the worst case of reflective (calling repetitively one empty function and calling a lot of different empty functions)

### author

Timothée Feuillet (_neam_ or _tim42_).
