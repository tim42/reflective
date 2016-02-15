
#include <fstream>
#include <ios>

#include "shell.hpp"
#include "exec.hpp"
#include "debug.hpp"
#include "flow_control.hpp"
#include "parse_command.hpp"

neam::r::shell::shell::shell()
{
  spack.streams.emplace_back(std::cin.rdbuf());
  spack.streams.emplace_back(std::cout.rdbuf());
  spack.streams.emplace_back(std::cerr.rdbuf());

  register_base_builtins();
}

int neam::r::shell::shell::run(const std::__cxx11::string &commands)
{
  return execute_no_context(commands);
}

int neam::r::shell::shell::execute_no_context(const std::string &commands)
{
  neam::r::shell::command_list ast;

  bool r = parse_command(commands, ast);
  if (!r)
    return 128; // syntax error

//   printer::print(ast); // DEBUG

  return exec::run(this, ast);
}

int neam::r::shell::shell::run_cmd(const std::string &invocation_name, stream_pack &streamp)
{
  // looks for functions
  {
    const command_list *cl = vstack.get_function(invocation_name);
    if (cl)
    {
      try
      {
        return exec::run(this, *cl);
      }
      catch (return_flow_control &r) { return r.retval; }
      catch (flow_control &fc)
      {
        streamp[stream::stderr] << invocation_name << ": bad usage of " << fc.name << std::endl;
        return 127;
      }
    }
  }

  // looks for builtins
  {
    int ret;
    if (bltmgr.call(invocation_name, vstack, streamp, ret))
      return ret;
  }

  // looks for binaries
  // TODO (??)

  // dies
  streamp[stream::stderr] << invocation_name << ": command not found" << std::endl;
  return 1;
}

void neam::r::shell::shell::register_base_builtins()
{
  builtin_builtin();
  builtin_echo();
  builtin_exit();
  builtin_help();
  builtin_nothing();
  builtin_return();
  builtin_shift();
  builtin_source();

  builtin_true();
  builtin_false();
  builtin_not();
}

void neam::r::shell::shell::builtin_builtin()
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    vs.shift_arguments();
    const std::deque<std::string> &args = vs.get_argument_array();
    if (args.empty())
      return 0;
    std::string forward_name = args[1];
    int ret = 1;
    bool called = bltmgr.call(forward_name, vs, streamp, ret);
    if (!called)
    {
      streamp[stream::stderr] << name << ": " << forward_name << ": not a builtin" << std::endl;
      return 1;
    }
    return ret;
  }, "run a builtin", "builtin [builtin-options]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin("builtin", blt);
}

void neam::r::shell::shell::builtin_help()
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &, stream_pack &streamp, boost::program_options::variables_map &vm) -> int
  {
    if (vm.count("builtin"))
    {
      std::string blt_name = vm["builtin"].as<std::string>();
      if (!bltmgr.print_help(blt_name, streamp))
      {
        streamp[stream::stderr] << "error: " << name << ": unknown builtin " << blt_name << std::endl;;
        return 1;
      }
    }
    else
    {
      bltmgr.print_builtin_summary(streamp);
    }
    return 0;
  }, "print help", "[builtin]");
  blt.add_options() ("builtin", boost::program_options::value<std::string>(), "a builtin whose help should be printed");
  blt.get_positional_options_description().add("builtin", 1);
  bltmgr.register_builtin("help", blt);
}

void neam::r::shell::shell::builtin_echo()
{
  builtin &blt = *new builtin([](const std::string &, variable_stack &, stream_pack &streamp, boost::program_options::variables_map &vm) -> int
  {
    if (!streamp.has(stream::stdout))
      return 1;

    bool newline = !vm.count("no-newline");
    bool escapes = vm.count("escapes");


    bool not_first = false;
    if (vm.count("string"))
    {
      for (const std::string & str : vm["string"].as<std::vector<std::string>>())
      {
        if (not_first)
          streamp[stream::stdout] << ' ';
        not_first = true;

        if (escapes)
        {
          std::string result;
          bool escaped = false;
          for (size_t i = 0; i < str.size(); ++i)
          {
            if (escaped)
            {
              switch (str[i])
              {
                case '\\': result += '\\'; break;
                case 'a': result += '\a'; break;
                case 'b': result += '\b'; break;
                case 'e': result += '\e'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'v': result += '\v'; break;
                case 'x':
                  if (i + 2 < str.size())
                    break;
                  {
                    char data[] = {str[i + 1], str[i + 2], 0};
                    i += 2;
                    char *_u;
                    char t = strtoul(data, &_u, 16);
                    result += t;
                  }
                  break;
                default: result += str[i]; break;
              }
            }
            else if (str[i] == '\\')
              escaped = true;
            else
              result += str[i];
          }
          streamp[stream::stdout] << result;
        }
        else
          streamp[stream::stdout] << str;
      }
    }

    if (newline)
      streamp[stream::stdout] << std::endl;

    return 0;
  }, "display text", "[options] string to print");
  blt.add_options() ("string", boost::program_options::value<std::vector<std::string>>(), "the string to print (should be omitted)")
                    ("no-newline,n", "do not output the trailing newline")
                    ("escapes,e", "enable interpretation of backslash escapes")
                    ("no-escapes,E", "disable interpretation of backslash escapes (default)");
  blt.get_positional_options_description().add("string", -1);
  bltmgr.register_builtin("echo", blt);
}

void neam::r::shell::shell::builtin_exit()
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    int ret = 0;
    const std::deque<std::string> &args = vs.get_argument_array();
    if (args.size() > 2)
    {
      ret = 128;
      streamp[stream::stderr] << name << ": too many arguments" << std::endl;
    }
    else if (args.size() == 2)
    {
      if (std::isdigit(args[1][0]))
        ret = atoi(args[1].c_str());
      else
      {
        streamp[stream::stderr] << name << ": numeric argument required" << std::endl;
        ret = 128;
      }
    }
    exit(ret);
  }, "exit the shell", "[exit value]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin("exit", blt);
}

void neam::r::shell::shell::builtin_nothing()
{
  builtin &blt = *new builtin([&](const std::string &, variable_stack &, stream_pack &, boost::program_options::variables_map &) -> int
  {
    return 0;
  }, "does nothing", "[whatever]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin(":", blt);
}

void neam::r::shell::shell::builtin_return()
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    int ret = 0;
    const std::deque<std::string> &args = vs.get_argument_array();
    if (args.size() > 2)
    {
      ret = 128;
      streamp[stream::stderr] << name << ": too many arguments" << std::endl;
    }
    else if (args.size() == 2)
    {
      if (std::isdigit(args[1][0]))
        ret = atoi(args[1].c_str());
      else
      {
        streamp[stream::stderr] << name << ": numeric argument required" << std::endl;
        ret = 128;
      }
    }
    throw return_flow_control(ret);
    return 0;
  }, "return to the parent scope", "[return value]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin("return", blt);
}

void neam::r::shell::shell::builtin_source()
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    const std::deque<std::string> &args = vs.get_argument_array();
    if (args.size() < 2)
    {
      streamp[stream::stderr] << name << ": missing filename parameter" << std::endl;
      return 1;
    }
    std::string filename = args[1];

    std::ifstream file(filename);

    if (!file)
    {
      streamp[stream::stderr] << name << ": unable to open " << filename << std::endl;
      return 1;
    }

    file.seekg(0, std::ios_base::end);
    long size = file.tellg();
    file.seekg(0, std::ios_base::beg);

    if (!size)
    {
      streamp[stream::stderr] << name << ": empty file: " << filename << std::endl;
      return 1;
    }

    char *memory = new char[size + 1];

    file.read(memory, size);
    memory[size] = 0;
    std::string content = memory;
    delete [] memory;

    int ret = execute_no_context(content);

    return ret;
  }, "read and execute commands from filename in the current shell environment", "filename [arguments]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin("source", blt);
  bltmgr.register_builtin(".", blt);
}

void neam::r::shell::shell::builtin_shift()
{
  builtin &blt = *new builtin([&](const std::string &name, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    const std::deque<std::string> &args = vs.get_argument_array();
    if (args.size() > 2)
    {
      streamp[stream::stderr] << name << ": too many parameters" << std::endl;
      return 1;
    }
    int ret = 0;

    size_t shift = 1;
    if (args.size() == 2)
    {
      if (!std::isdigit(args[1][0]))
      {
        streamp[stream::stderr] << name << ": wrong parameter type: must be a number" << std::endl;
        return 1;
      }
      shift = atoi(args[1].c_str());
    }

    // Well, a bit strange, but as we work on the parent context...
    vs.pop_context();
    ret = vs.shift_arguments(shift) ? 0 : 2;
    vs.push_context();

    return ret;
  }, "shift parameters down", "[n]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin("shift", blt);
}

void neam::r::shell::shell::builtin_true()
{
  builtin &blt = *new builtin([&](const std::string &, variable_stack &, stream_pack &, boost::program_options::variables_map &) -> int
  {
    return 0;
  }, "return true", "[whatever]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin("true", blt);
}

void neam::r::shell::shell::builtin_false()
{
  builtin &blt = *new builtin([&](const std::string &, variable_stack &, stream_pack &, boost::program_options::variables_map &) -> int
  {
    return 1;
  }, "return false", "[whatever]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin("false", blt);
}

void neam::r::shell::shell::builtin_not()
{
  builtin &blt = *new builtin([&](const std::string &, variable_stack &vs, stream_pack &streamp, boost::program_options::variables_map &) -> int
  {
    vs.shift_arguments();
    const std::deque<std::string> &args = vs.get_argument_array();
    if (args.empty())
      return 1;
    std::string forward_name = args[1];
    int ret = this->run_cmd(forward_name, streamp);
    return ret ? 0 : 1;
  }, "inverse the return value of a command", "[command [arguments]]");
  blt.do_not_use_program_options();
  bltmgr.register_builtin("!", blt);
}

