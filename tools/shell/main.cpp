
// #include "parser.hpp"
// #include "debug.hpp"
// #include "exec.hpp"

#include "shell.hpp"

int main(int argc, char **argv)
{
  neam::r::shell::shell shell;

  std::string line;
  std::cout << "reflective-shell> ";
  while (std::getline(shell.get_shell_streampack()[neam::r::shell::stream::stdin], line))
  {
    shell.run(line);
    std::cout << "reflective-shell> ";
  }
  std::cout << std::endl;

  return 0;
}
