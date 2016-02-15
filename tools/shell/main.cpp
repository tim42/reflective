
#include <string>
#include <iostream>
#include <fstream>

#include <tools/logger/logger.hpp>
#include "shell.hpp"
#include "reflective_builtins.hpp"

#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else // assuming UNIX/Linux is the current OS
#include <unistd.h>
#endif

#include <reflective.hpp>

/// \brief Handle the message printing
static int print_usage(boost::program_options::options_description &desc);

/// \brief Forward some options to the shell/file
static void forward_argv(neam::r::shell::shell &shell, const std::vector<std::string> &params, size_t start_index = 0);

/// \brief Run the shell from a file
static int shell_from_file(neam::r::shell::shell &shell, const std::string &file);

/// \brief Run the shell from a stdin
static int shell_from_stdin(neam::r::shell::shell &shell);

int main(int argc, char **argv)
{
  // setup the conf
  neam::r::conf::disable_auto_save = true;
  neam::r::conf::out_file = "";

  // setup boost po
  bool from_file = false;
  bool from_argv = false;
  bool forced_normal = false;
  bool load_only = true;
  std::vector<std::string> params;
  boost::program_options::options_description desc;
  desc.add_options()
    ("help,h", "print this help and exit")
    ("load,l", boost::program_options::value<std::string>(), "pre-load a reflective save file")
    ("stdin,s", "read commands from the standard input")
    ("command,c", "read  commands  from the command_string operand. no commands shall be read from the standard input.")
    ("param", boost::program_options::value<std::vector<std::string>>(&params),
              "positional parameters. Can hold the command_string, (if -c specified), "
              "the file to execute and its arguments (if -s NOT specified) "
              "or shell arguments that will be in $1..$N (if -s specified). "
              "can be ignored: you can simply put the value without the --param");
  boost::program_options::positional_options_description pod;
  pod.add("param", -1);
  boost::program_options::variables_map vm;
  try
  {
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(pod).allow_unregistered().run(), vm);
    boost::program_options::notify(vm);
  }
  catch (std::exception &e)
  {
    std::cerr << argv[0] << ": " << e.what() << std::endl;
    print_usage(desc);
    return 1;
  }

  // handle options
  from_argv = vm.count("command");
  forced_normal = vm.count("stdin");
  from_file = !from_argv && !forced_normal && params.size();

  if (from_argv && !params.size())
  {
    std::cerr << argv[0] << ": -c requires a command_string" << std::endl;
    return 1;
  }
  if (vm.count("help"))
  {
    print_usage(desc);
    return 0;
  }

  load_only = !vm.count("load");

  // the shell
  neam::r::shell::shell shell;

  if (!load_only)
  {
    if (!neam::r::shell::load_reflective_file(shell, vm["load"].as<std::string>()))
    {
      std::cerr << ": load: could not load " << vm["load"].as<std::string>() << std::endl;
      return 1;
    }
  }
  else
    neam::r::shell::register_reflective_builtins(shell);

  // set $0
  if (!from_file)
    shell.get_variable_stack().push_argument(argv[0]);

  // set $1..$N (or $0..$N if it's from a file)
  forward_argv(shell, params, from_argv ? 1 : 0);

  // run the shell
  if (from_argv)
    return shell.run(params[0]);
  else if (from_file)
    return shell_from_file(shell, params[0]);

  // read from stdin
  return shell_from_stdin(shell);
}

int print_usage(boost::program_options::options_description &desc)
{
  std::cout << "reflective shell: query informations from neam::reflective save files\n"
            << "Usage:\n"
            << "\trshell [-l file] -s [argument...]\n"
            << "\trshell [-l file] -c command\n"
            << "\trshell [-l file] [--] [[file] [argument...]]\n"
            << "\nOptions:\n"
            << desc << std::endl;

  return 0;
}

void forward_argv(neam::r::shell::shell &shell, const std::vector<std::string> &params, size_t start_index)
{
  size_t i = 0;
  for (const std::string &it : params)
  {
    if (i++ >= start_index)
      shell.get_variable_stack().push_argument(it);
  }
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
  return shell.run(content);
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

