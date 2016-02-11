
#include "parse_command.hpp"
#include "parser.hpp"

bool neam::r::shell::parse_command(const std::__cxx11::string &cmd, neam::r::shell::command_list &ast)
{
  neam::r::shell::grammar<std::string::const_iterator> grammar;

  std::string::const_iterator iter = cmd.begin();
  std::string::const_iterator end = cmd.end();

  return boost::spirit::qi::phrase_parse(iter, end, grammar, boost::spirit::ascii::space, ast);
}
