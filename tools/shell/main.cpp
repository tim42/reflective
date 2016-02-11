
#include <string>
#include <iostream>
#include <fstream>
#include "shell.hpp"

#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else // assuming UNIX/Linux is the current OS
#include <unistd.h>
#endif

/// \brief Handle the message printing
static int print_usage(const std::string &arg = "", bool fail = false);

/// \brief Forward some options to the shell/file
static void forward_argv(neam::r::shell::shell &shell, int argc, char **argv, int start_index);

/// \brief Run the shell from a file
static int shell_from_file(neam::r::shell::shell &shell, const std::string &file);

/// \brief Run the shell from a stdin
static int shell_from_stdin(neam::r::shell::shell &shell);

int main(int argc, char **argv)
{
  bool from_file = false;
  bool from_argv = false;
  bool forced_normal = false;

  // don't want boost program option to manage this: can't deal with short-option-only.
  if (argc > 1)
  {
    std::string a1(argv[1]);
    if (a1 == "-c")
      from_argv = true;
    else if (a1 == "-s")
      forced_normal = true;
    else if (a1 == "-h" || a1 == "--help")
      return print_usage();
    else if (a1[0] != '-')
      from_file = true;
    else if (a1 == "--")
      from_file = argc > 3;
    else
      return print_usage(a1, true);
  }

  // the shell
  neam::r::shell::shell shell;

  // from a command placed as argument
  if (from_argv)
  {
    if (argc < 2)
    {
      std::cerr << "The -c argument takes a command" << std::endl;
      print_usage();
      return 1;
    }
    return shell.run(argv[2]);
  }
  else if (from_file)
  {
    int idx = 1;
    if (std::string("--") == argv[1])
      idx = 2;
    forward_argv(shell, argc, argv, idx);
    return shell_from_file(shell, argv[idx]);
  }
  else if (forced_normal) // we got args, but that's not a file
    forward_argv(shell, argc, argv, 1);

  // read from stdin
  return shell_from_stdin(shell);

  return 0;
}

int print_usage(const std::string &arg, bool fail)
{
  int ret = 0;
  if (fail)
  {
    std::cerr << "error: unknown option " << arg << std::endl;
    ret = 1;
  }

  std::cout << "reflective shell: query informations from neam::reflective save files\n"
            << "Usage:\n"
            << "\trshell -s [argument...]\n"
            << "\trshell -c command\n"
            << "\trshell [--] [[file] [argument...]]"
            << std::endl;

  return ret;
}

void forward_argv(neam::r::shell::shell &shell, int argc, char **argv, int start_index)
{
  for (int i = start_index; i < argc; ++i)
    shell.get_variable_stack().push_argument(argv[i]);
}

int shell_from_file(neam::r::shell::shell &shell, const std::string &filename)
{
  std::ifstream file(filename);

  if (!file)
  {
    shell.get_shell_streampack()[neam::r::shell::stream::stderr] << "reflective shell: unable to open " << filename << std::endl;
    return 1;
  }

  file.seekg(0, std::ios_base::end);
  long size = file.tellg();
  file.seekg(0, std::ios_base::beg);

  if (!size)
  {
    shell.get_shell_streampack()[neam::r::shell::stream::stderr] << "reflective shell: empty file: " << filename << std::endl;
    return 1;
  }

  char *memory = new char[size + 1];

  file.read(memory, size);
  memory[size] = 0;
  std::string content = memory;
  delete [] memory;
  shell.run(content);
}

int shell_from_stdin(neam::r::shell::shell &shell)
{
  bool print_prompt = isatty(fileno(stdout));
  std::string line;
  if (print_prompt)
    shell.get_shell_streampack()[neam::r::shell::stream::stdout] << "reflective-shell> ";
  while (std::getline(shell.get_shell_streampack()[neam::r::shell::stream::stdin], line))
  {
    shell.run(line);
    if (print_prompt)
      shell.get_shell_streampack()[neam::r::shell::stream::stdout] << "reflective-shell> ";
  }
  std::cout << std::endl;

  return 0;
}

