
### reflective shell

Reflective comes with a nice little shell (that has a basical, shell-like, syntax) to query and do some operations on reflective save files.

### syntax

the shell is a subset of the shell syntax. Currently, you can do:
 - `cmd1 && cmd2 && ... && cmd3`
 - `cmd1 || cmd2 || ... || cmd3`
 - `cmd1 ; cmd2; cmd3`
 - any imbrication of the previous items like in `cmd1 && cmd2 || cmd3;cmd1 || cmd2;`
 - `$my_var` or `${my_var}` and the non-standard `${$my_var_ptr}` (and so on)
 - `my_var="my value"` and the non-standard `${my_var_ptr}="my value"`
 - `my_var="my value" cmd args`. This has a non-standard effect: if there is variable substitution in the parameters, the substitution accounts the previous affectation.
   As in a shell, the variable affectation affects only the following command.
 - `{ cmd1; cmd2; cmd3 && cmd4; }` : chain commands
 - `(cmd1; cmd2; cmd3 && cmd4)` : the subshell variant: operations done in the subshell are only valid in the subshell (like variable affectation, function declaration, ...)
 - `function toto { cmd1; cmd2 && cmd3 || { my_var=1; echo $2; } || return $my_var; return 0; }`, you can declare functions (can be multiline if declared in a file)
   the shell does not supports `"$@"` nor `$*`
 - capture stdout: `echo $(cmd1 && cmd2)` or `my_var=$(cmd1)`. Also works using `.

There are some differences with _"real"_ shells, notably on variable substitution:
 - `my_var="things with spaces"; cmd $my_var` will expand to `cmd 'things with spaces'` and not to `cmd things with spaces`.
 - the same goes for `$(cmd)`
 - the shell does not supports `"$@"` nor `$*`
 - the shell does not supports `"${my_var:n:m}"` nor `${my_var##*/}` (and likes)
 - no support for globings
 - no support for `if`, `for`, `while` and `case`
 - echo does not supports the `\c` escape nor the `\0[0-7]+` octal escape
 - no redirections
Those differences are known and may be fixed one day.

Also, this is a forkless shell, so you cannot run external programs nor use pipes. It does not intend to replace a shell, but to provide
a familiar environment that has enough flexibility to be somehow usable. The choice of being a forkless shell has been made for, at least, two reasons:
 - windows does not like forks at all (what a shame). But I don't use windows, so this is a false reason.
 - we have a simulated filesystem, the current working directory may not exists and is in fact purely virtual. How the (s)hell would handle a command that read or write files ?
   How would the shell handle commands that depend on the current working directory or operate on "real" files ? Does that have a meaning worth the (not so hard) effort ?
 - The shell has been created to handle the save file as if it were _chrooted_ into that file and to
   operate like a mariadb command line client would do (except that I do not like SQL, so I gone the *sh way)

### builtins

The shell handle some "standard" builtins and all of them have a associated help section in the shell,
so I will only describe the `help` one:
 - `help` will list all the builtins with a short description
 - `help builtin` will show the "documentation" of that builtin.

the shell also comes with "non-standard" builtins (both builtins that doesn't exists in other shells or
builtins that have the same name but could have a different effect/output than you would have expected)
So in a non-alphabetical order:

#### load

`load reflective-file`: The reflective-file is the (real) path of a reflective output/save file. This command is mandatory in order to use all the other ones.
The `load` builtin also remove itself if the load is successful.

You can also start the shell with a `-l` option that loads a neam::reflective file.

The variable `${R_ACTIVE_FILE}` contains the name of the loaded (active) neam::reflective file.

#### mode

Two modes are present in the reflective shell. A mode is the way the virtual filesystem is generated
 - `file`: The default mode, gather function by files, and files by directories.
 - `function`: In this mode, you walk the callgraph as if it were a directory tree.

The variable `${R_MODE}` contain the current mode. You can also query the current mode by calling `mode` without any argument.

**NOTE**: the information returned for the same function can change from one mode to the other.
In the `function` mode, the information is contextualized: the `pwd` is the context of the current function and the information is what reflective
has monitored for the function when called as this place (`pwd`) in the callgraph whereas the `file` mode display non-contextualized information about the function
(information from all possible invocations).

#### cd

Change the current working "directory" (depends on which mode is active).
`cd` without any argument return to the root (`/`).

#### pwd

Print the current working directory. The current working directory can also be queried by the substitution of the variable `${CWD}`.

#### ls

List the contents of a "directory". (or if the mode is `function`, the functions called by the current function).
If you are in the `file` mode, the `-f` option will force the printing of functions

The `-l` will make `ls` output in a long listing format (a bit like a "real" `ls` would do).
The long format is the following:
`[ hit-count / error-count ]` `s: self-time` `xs: see-below` `g: global-time` `xg: see-below` `file/function name`

`hit-count` is the number of time the file or function has been "hit" (called for a function and for a file, the number of called functions).

The `xs` entry is `self-time` time `hit-count`. It gives an estimated time spent in that file/function *only* in a whole run.
This is a quite precious indicator when trying to find bottlenecks as you get the time consumed by the function, and only that function
(the chronometer is paused when that function calls another monitored function), and the time is not a per-call average but a per-program launch average.

The `xg` entry is the same as `xs` except it's with the global time. `xs` is a better indicator than `xg` in most cases.

#### info

This is the most important command the shell has. It allows the user to get *all* the information about monitored functions neam::reflective stores.

`help info` may give more information about option this command has. See also the example below.

#### example

Here's transcript you can use on the save file generated by the `test` sample:

run multiple time the `test` sample, then launch the shell with `reflective-shell -l sample.nr`
You can then enter:
```shell
mode function   # we switch to the function mode
cd main         # we enter in the main function
ls              # we print the function called by the main().
                # in function mode, we walk the callgraph as if functions were
                # directories
info -ae -c10 s::d  # we print the information about the method d() of the
                    # class s. The -a flag tells info to print everything,
                    # the -e flag tells it to also print errors,
                    # the -c10 tells it to print at most 10 different errors
                    # s::d is simply the name of the method, as printed by ls
```

This will give you a similar output:

for `ls`:
```
s::d           fnc
```

for `info -ae -c 10 s::d`:
```
pretty name: void s::d()
usual name: s::d
is contextualized: true
file: /home/tim/projects/reflective/samples/test/main.cpp: 22
average global time: 2.3152ms
total global time (per-launch): 2.3152ms
average self time: 2.31413ms
total self time (per-launch): 2.31413ms
self duration progression:
  at 2016-02-15 22:04:16: 2ms
global duration progression:
  at 2016-02-15 22:04:16: 2ms
measure points:
  function_call-constructor-time: 242.829ns
  half-time: 475.925us
number of failures: 0 (ratio: 0%)
errors: (including errors from other contexts)
  lazy programmer: '[can't touch this]'
  | file: /home/tim/projects/reflective/samples/test/main.cpp line 40
  | number of time reported: 2871
  |  from: 2016-02-15 22:04:16
  |__to:   2016-02-15 22:05:33

```

the `is contextualized` entry tell that the information info will be the information about
`/main/s::d`, or as when `s::d()` is called by `main()`. To get global information, you can simply set the `-g` flag to info.
Interesting fields that are not present in the `ls` output includes
 - `{self,global} duration progression` (if the monitoring of fluctuation in self time and global time is activated):
  A new entry is created each time a significant change is done in the self/global time. With this filed, you can know if things get slower with time.
 - `measure points`: The user can create arbitrary measure points (with the `neam::r::measure_point` class). A measure point monitor the time spent in
   an arbitrary section of the code.
 - `errors`: It report the errors of the current function (activated with the -e flag).
   an important thing to note, the error messages are not contextualized: here the report says that we have a number of failures (aka errors) equals to 0
   so in this context (see the explanation of `is contextualized`) the function does **not** produce any error.
   About the error reporting: we first have the error type (`lazy programmer`) then a message (`'[can't touch this]'`),
   the file:line where the error has been reported, the number of time that error has been **consecutively** reported.
   The date range is the date the error has been first reported and last reported.

   Errors are *always* ordered in time with the guarantee that no date range can overlap.


#### NOTE

As I am not a native English speaker, some (most ?) of what is written here may not be English at all.
I also write readmes last (and mostly late at night) and that doesn't help improving my skills.
