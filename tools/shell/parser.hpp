//
// file : parser.hpp
// in : file:///home/tim/projects/reflective/tools/shell/parser.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: 05/02/2016 22:18:27
//
//
// Copyright (C) 2016 Timothée Feuillet
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#ifndef __N_9958282391962871894_1413829201__PARSER_HPP__
# define __N_9958282391962871894_1413829201__PARSER_HPP__

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/phoenix/stl/algorithm.hpp>
#include <boost/phoenix/stl/container.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <tools/logger/logger.hpp>

#include "data_structure.hpp"

BOOST_FUSION_ADAPT_STRUCT(
  neam::r::shell::var_affectation,
  (neam::r::shell::node_list, variable_name)
  (neam::r::shell::node_list, value)
)
BOOST_FUSION_ADAPT_STRUCT(
  neam::r::shell::var_expansion,
  (neam::r::shell::node_list, variable_name)
)
BOOST_FUSION_ADAPT_STRUCT(
  neam::r::shell::command_list,
  (std::vector<neam::r::shell::command_node>, list)
)
BOOST_FUSION_ADAPT_STRUCT(
  neam::r::shell::or_command_list,
  (std::vector<neam::r::shell::command_node>, list)
)
BOOST_FUSION_ADAPT_STRUCT(
  neam::r::shell::and_command_list,
  (std::vector<neam::r::shell::command_node>, list)
)
BOOST_FUSION_ADAPT_STRUCT(
  neam::r::shell::subshell,
  (std::vector<neam::r::shell::command_node>, list)
)
BOOST_FUSION_ADAPT_STRUCT(
  neam::r::shell::function_declaration,
  (neam::r::shell::node_list, name)
  (neam::r::shell::command_list, body)
)

BOOST_FUSION_ADAPT_STRUCT(
  neam::r::shell::command,
  (std::vector<neam::r::shell::var_affectation>, affectations)
  (neam::r::shell::node_list, command_name)
  (std::vector<neam::r::shell::node_list>, arguments)
)

namespace neam
{
  namespace r
  {
    namespace shell
    {
      namespace internal
      {
        // convert two hex to one char
        static inline char _hex_to_char(char c1, char c2)
        {
          char r = 0;
          char c[] = {c1, c2};
          for (size_t i = 0; i < 2; ++i)
          {
            if (c[i] >= '0' && c[i] <= '9')
              r = c[i] - '0';
            else
              r = 10 + (std::tolower(c[i]) - 'a');
            r <<= 4;
          }
          return r;
        }
        struct hex_to_char_impl
        {
          using result_type = char;
          template <typename Arg1, typename Arg2>
          char operator()(Arg1 arg1, Arg2 arg2) const
          {
            return _hex_to_char(arg1, arg2);
          }
        };
        boost::phoenix::function<hex_to_char_impl> hex_to_char;

        // print an error
        struct print_error_impl
        {
          using result_type = void;
          template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
          void operator()(const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4) const
          {
            Arg1 it = arg1;

            // get line number
            size_t line = 1;
            for (; it < arg3; ++it)
            {
              if (*it == '\n')
                ++line;
            }

            // get line content
            std::string line_content;
            for (; it < arg2; ++it)
            {
              if (*it == '\n')
                break;
              line_content.push_back(*it);
            }

            std::string info;
            {
              std::ostringstream os;
              os << arg4;
              std::string _info = os.str();
              info.reserve(_info.size());
              for (size_t i = 0; i < _info.size(); ++i)
              {
                switch (_info[i])
                {
                  case '\n': info += "\\n"; break;
                  case '\t': info += "\\t"; break;
                  case '\r': info += "\\r"; break;
                  default: info.push_back(_info[i]);
                }
              }
            }

            neam::cr::out.error() << LOGGER_INFO_TPL("[shell script]", line) << "expected " << (info) << " got:" << neam::cr::newline
                                  << line_content << std::endl;
          }
        };
        boost::phoenix::function<print_error_impl> print_error;
      } // namespace internal

      namespace fusion = boost::fusion;
      namespace phoenix = boost::phoenix;
      namespace qi = boost::spirit::qi;
      namespace ascii = boost::spirit::ascii;

      /// \brief An ultra basic shell-like grammar
      /// It handles comments, escape sequences, quoted strings, variables, `...` and $(...) expression, { ...; } command list (including && and ||), functions
      template <typename Iterator>
      struct grammar
        : qi::grammar<Iterator, command_list()>
      {
        grammar()
          : grammar::base_type(script, "script")
        {
          using ascii::char_;
          using namespace qi::labels;

          // // simple rules (mostly lexemes)
          comment %= '#' >> *(char_ - '\n');
          keyword %= qi::lit("function") | '{' | '}';
          white_space %= qi::lit(' ') | qi::lit('\t');
          command_end %= qi::lit(';') | qi::lit('\n') | qi::eoi; // explicit termination characters
          escape_seq = qi::lit('\\') >
                (
                    qi::lit('n') [_val = '\n']    // escape sequences
                  | qi::lit('t') [_val = '\t']
                  | qi::lit('v') [_val = '\v']
                  | qi::lit('f') [_val = '\f']
                  | qi::lit('r') [_val = '\r']
                  | qi::lit('a') [_val = '\a']
                  | qi::lit('b') [_val = '\b']
                  | qi::lit('e') [_val = '\e']
                  | qi::lit('\\') [_val = '\\']   // escaped special chars
                  | qi::lit('\n') [_val = '\n']   // NOTE: this one differ in bash, where in non-quoted strings it is substituted by nothing
                  | qi::lit('\t') [_val = '\t']
                  | qi::lit(' ') [_val = ' ']
                  | qi::lit('"') [_val = '"']    // quotation marks
                  | qi::lit('\'') [_val = '\'']
                  | qi::lit('`') [_val = '`']
                  | qi::lit(';') [_val = ';']     // command-end
                  | qi::lit('=') [_val = '=']     // affectation
                  | qi::lit('#') [_val = '#']     // comment
                  | qi::lit('(') [_val = '('] | qi::lit(')') [_val = ')']     // bracket
                  | qi::lit('{') [_val = '{'] | qi::lit('}') [_val = '}']     // curly-brace
                  | qi::lit('&') [_val = '&']
                  | qi::lit('|') [_val = '|']
                  | (qi::lit('x') > qi::repeat(2)[char_("0-9a-fA-F")] [_val = internal::hex_to_char(phoenix::ref(_1)[0], phoenix::ref(_1)[1])]) // hex strings
                );

          special_char %= white_space | command_end | '"' | '\'' | '`' | '=' | '$' | '(' | ')' | '&' | '|';     // chars that can't appears unescaped in a not-quoted string

          // // the actual grammar
          nq_string %= +(escape_seq | (char_ - (special_char | '\\')));                             // not quoted strings
          dq_sub_string %= +(escape_seq | (char_ - (qi::lit('"') | qi::lit('\\') | qi::lit('$')))); // double quoted strings (the simple string+escape part)
          dq_string %= '"' > *(dq_sub_string | variable_expansion | bq_string) > '"';               // double quoted strings (the var + simple string part)
          sq_string %= '\'' > +(char_ - (qi::lit('\''))) > '\'';                                    // single quoted strings (no escape char handled)
          bq_string = (qi::eps(qi::_a == phoenix::val(false)) >> qi::lit('`')[qi::_a = true] >> (+command_gen)[phoenix::at_c<0>(_val)=_1] >> *white_space >> qi::lit('`')[qi::_a = false])
                    | ("$(" > (+command_gen)[phoenix::at_c<0>(_val)=_1] >> *white_space > ')');                  // sub commands (back-quotes strings or $(...))

          variable_id %= char_("a-zA-Z0-9_#-") >> *char_("a-zA-Z0-9_#-");                           // matches a "legal" variable ID

          variable_expansion %= ("${" > (variable_id|variable_expansion) > '}') | ('$' >> !char_("{(") > variable_id); // matches $MY_VAR or ${MY_VAR} (variable expansion / substitution)


          // An argument (also matches the command name) (can't start with '#')
          argument %= !char_('#') >> +(nq_string | dq_string | sq_string | bq_string | variable_expansion);

          // A variable affectation ( my_var="my value" or my_var= or $myvarptr="...")
          variable_affectation %= +(variable_id|variable_expansion) >> '=' >> -argument >> *white_space;

          // A single command (can't start with '#')
          single_command %= !char_('#') >> !char_('{') >> !char_('(')
                  >> *(*white_space >> variable_affectation)
                  >> *white_space >> !keyword >> argument
                  >> *(+white_space >> argument)
                  >> *white_space;

          // A command list (between { and })
          command_chain %= qi::skip(+white_space || comment)["{" >> *qi::lit('\n') > +command_gen >> *qi::lit('\n') > "}" > (command_end|&(qi::lit('&')|'|')) >> *qi::lit('\n')];
          // A subshell (between ( and ))
          subshell_rule %= qi::skip(+white_space || comment)["(" >> *qi::lit('\n') > +command_gen >> *qi::lit('\n') > ")" > (command_end|&(qi::lit('&')|'|')) >> *qi::lit('\n')];

          // A || command list
          or_command_list_rule = *white_space >> (and_command_list_rule|command_chain|subshell_rule|single_command|function|variable_affectation) [phoenix::push_back(phoenix::at_c<0>(_val), _1)]
                               >> +("||" > (and_command_list_rule|command_chain|subshell_rule|single_command|function|variable_affectation) [phoenix::push_back(phoenix::at_c<0>(_val), _1)])
                               >> *(white_space);
          // A && command list
          and_command_list_rule = *white_space >> (command_chain|subshell_rule|single_command|function|variable_affectation) [phoenix::push_back(phoenix::at_c<0>(_val), _1)]
                               >> +("&&" > (command_chain|subshell_rule|single_command|function|variable_affectation) [phoenix::push_back(phoenix::at_c<0>(_val), _1)])
                               >> *(white_space);

          // A function declaration:
          function %= "function" > +(white_space) > (variable_id|variable_expansion) >> *(white_space|'\n') > command_chain;

          // A more generic concept of command (matches anything executable)
          command_gen %= *white_space >> (or_command_list_rule|and_command_list_rule|command_chain|subshell_rule|single_command|function|variable_affectation) >> -command_end >> *(white_space|'\n');

          // The script (we skip comments to build a list of commands)
          script %= qi::skip(+(white_space|'\n') || comment)
                    [
                      +command_gen >> *qi::lit('\n') > qi::eoi
                    ]
                  ;

          script.name("script");
          command_chain.name("command list");
          subshell_rule.name("subshell");
          or_command_list_rule.name("|| command list");
          and_command_list_rule.name("&& command list");
          single_command.name("command");
          function.name("function declaration");
          command_gen.name("command");
          variable_affectation.name("variable affectation");
          variable_expansion.name("variable expansion");
          variable_id.name("variable identifier");
          argument.name("argument");
          escape_seq.name("escape sequence");
          nq_string.name("not-quoted string");
          sq_string.name("single-quoted string");
          dq_string.name("double-quoted string");
          dq_sub_string.name("double-quoted string");
          bq_string.name("back-quoted string");
          special_char.name("special character");
          keyword.name("keyword");
          white_space.name("white space");
          command_end.name("command end");
          comment.name("comment");

          qi::on_error<qi::fail>(script, internal::print_error(_1, _2, _3, _4));
        }

        qi::rule<Iterator, command_list()> script;
        qi::rule<Iterator, command_list()> command_chain;
        qi::rule<Iterator, subshell()> subshell_rule;
        qi::rule<Iterator, command()> single_command;
        qi::rule<Iterator, or_command_list()> or_command_list_rule;
        qi::rule<Iterator, and_command_list()> and_command_list_rule;
        qi::rule<Iterator, function_declaration()> function;
        qi::rule<Iterator, command_node()> command_gen;
        qi::rule<Iterator, var_affectation()> variable_affectation;
        qi::rule<Iterator, var_expansion()> variable_expansion;
        qi::rule<Iterator, std::string()> variable_id;
        qi::rule<Iterator, node_list()> argument;
        qi::rule<Iterator, std::string()> sq_string;
        qi::rule<Iterator, node_list()> dq_string;
        qi::rule<Iterator, std::string()> dq_sub_string;
        qi::rule<Iterator, std::string()> nq_string;
        qi::rule<Iterator, command_list(), qi::locals<bool>> bq_string;
        qi::rule<Iterator, char()> escape_seq;
        qi::rule<Iterator> keyword;
        qi::rule<Iterator> special_char;
        qi::rule<Iterator> white_space;
        qi::rule<Iterator> command_end;
        qi::rule<Iterator> comment;
      };
    }
  }
}

#endif /*__N_9958282391962871894_1413829201__PARSER_HPP__*/

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; 

