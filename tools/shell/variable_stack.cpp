
#include <tools/logger/logger.hpp>
#include "variable_stack.hpp"

neam::r::shell::variable_stack::variable_stack()
{
  push_context(true, false);
}

void neam::r::shell::variable_stack::push_context(bool argument_list, bool copy_on_write_context)
{
  context_stack.push_back({argument_list, copy_on_write_context, {}, {}, {}});
}

void neam::r::shell::variable_stack::pop_context()
{
  if (context_stack.size() == 1)
    neam::cr::out.error() << LOGGER_INFO << "trying to pop the last shell context: refusing" << std::endl;
  else
    context_stack.pop_back();
}

bool neam::r::shell::variable_stack::has_variable(const std::string &name) const
{
  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    const context &ctx = context_stack[context_stack.size() - 1 - i];

    if (ctx.variable_list.count(name))
      return true;
  }

  if (special_variable_list.count(name))
    return true;

  return false;
}

std::string neam::r::shell::variable_stack::get_variable(const std::string &name) const
{
  // lookup argument
  if (name.size() && std::isdigit(name[0]))
  {
    size_t arg_idx = atoi(name.c_str());
    const std::deque<std::string> &a = get_argument_array();
    if (arg_idx < a.size())
      return a[arg_idx];
    return std::string();
  }
  if (name == "#")
    return std::to_string(get_argument_count() - 1);

  // lookup normal variables
  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    const context &ctx = context_stack[context_stack.size() - 1 - i];

    auto it = ctx.variable_list.find(name);
    if (it != ctx.variable_list.end())
      return it->second;
  }

  auto it = special_variable_list.find(name);
  if (it != special_variable_list.end())
    return it->second(name);  // call the special handler

  return "";
}

void neam::r::shell::variable_stack::set_variable(const std::string &name, const std::string &value, bool in_root_context)
{
  // create the variable in the right context
  if (in_root_context)
    context_stack.front().variable_list[name] = value;
  else
    context_stack.back().variable_list[name] = value;
}

void neam::r::shell::variable_stack::unset_variable(const std::string &name)
{
  // Handle COW context
  if (context_stack.back().is_copy_on_write)
  {
    context_stack.back().variable_list.erase(name);
    return;
  }

  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    context &ctx = context_stack[context_stack.size() - 1 - i];

    auto it = ctx.variable_list.find(name);
    if (it != ctx.variable_list.end())
    {
      ctx.variable_list.erase(it);
      return;
    }

    if (ctx.is_copy_on_write) // well, cow contexts acts like a barrier.
      return;
  }
}

bool neam::r::shell::variable_stack::has_function(const std::string &name) const
{
  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    const context &ctx = context_stack[context_stack.size() - 1 - i];

    if (ctx.function_list.count(name))
      return true;
  }

  return false;
}

const neam::r::shell::command_list *neam::r::shell::variable_stack::get_function(const std::string &name) const
{
  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    const context &ctx = context_stack[context_stack.size() - 1 - i];

    auto it = ctx.function_list.find(name);
    if (it != ctx.function_list.end())
      return &it->second;
  }

  return nullptr;
}

void neam::r::shell::variable_stack::set_function(const std::string &name, const neam::r::shell::command_list &func, bool in_root_context)
{
  // create the variable in the right context
  if (in_root_context)
    context_stack.front().function_list[name] = func;
  else
    context_stack.back().function_list[name] = func;
}

void neam::r::shell::variable_stack::unset_function(const std::string &name)
{
  // Handle COW context
  if (context_stack.back().is_copy_on_write)
  {
    context_stack.back().function_list.erase(name);
    return;
  }

  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    context &ctx = context_stack[context_stack.size() - 1 - i];

    auto it = ctx.function_list.find(name);
    if (it != ctx.function_list.end())
    {
      ctx.function_list.erase(it);
      return;
    }

    if (ctx.is_copy_on_write) // well, cow contexts acts like a barrier.
      return;
  }
}

void neam::r::shell::variable_stack::cow_copy_argument_list(neam::r::shell::variable_stack::context *ctx, size_t index)
{
  if (ctx->is_copy_on_write &&!ctx->has_argument_list)
  {
    ctx->has_argument_list = true;

    // copy the previous argument handling context
    context *prev_cptr = nullptr;

    for (; index < context_stack.size(); ++index)
    {
      if (context_stack[context_stack.size() - 1 - index].has_argument_list)
        prev_cptr = &context_stack[context_stack.size() - 1 - index];
    }

    // found something: copy it
    if (prev_cptr)
      ctx->argument_list = prev_cptr->argument_list;
  }
}

void neam::r::shell::variable_stack::push_argument(const std::string &value)
{
  context *cptr = &context_stack.back();

  size_t i = 0;
  // get the first argument-handling context
  if (!cptr->has_argument_list && !cptr->is_copy_on_write)
  {
    for (; i < context_stack.size(); ++i)
    {
      if (context_stack[context_stack.size() - 1 - i].has_argument_list
          || context_stack[context_stack.size() - 1 - i].is_copy_on_write)
        cptr = &context_stack[context_stack.size() - 1 - i];
    }
  }

  cow_copy_argument_list(cptr, i);

  // push
  cptr->argument_list.push_back(value);
}

size_t neam::r::shell::variable_stack::get_argument_count() const
{
  const context *cptr = nullptr;

  // get the first argument-handling context
  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    if (context_stack[context_stack.size() - 1 - i].has_argument_list)
      cptr = &context_stack[context_stack.size() - 1 - i];
  }

  if (!cptr) // should never happen
    return 0;

  return cptr->argument_list.size();
}

bool neam::r::shell::variable_stack::shift_arguments(size_t shift_count)
{
  context *cptr = &context_stack.back();

  size_t i = 0;
  // get the first argument-handling context
  if (!cptr->has_argument_list && !cptr->is_copy_on_write)
  {
    for (; i < context_stack.size(); ++i)
    {
      if (context_stack[context_stack.size() - 1 - i].has_argument_list
          || context_stack[context_stack.size() - 1 - i].is_copy_on_write)
        cptr = &context_stack[context_stack.size() - 1 - i];
    }
  }

  cow_copy_argument_list(cptr, i);


  bool ret = true;
  if (cptr->argument_list.size() - 1 < shift_count)
  {
    shift_count = cptr->argument_list.size() - 1;
    ret = false;
  }

  // shift
  if (cptr->argument_list.size() > 1)
    cptr->argument_list.erase(cptr->argument_list.begin() + 1, cptr->argument_list.begin() + 1 + shift_count);

  return ret;
}

char **neam::r::shell::variable_stack::get_argument_as_c_array() const
{
  size_t argc = get_argument_count();
  char **argv = new char*[argc + 1];
  argv[argc] = nullptr;

  const context *cptr = nullptr;

  // get the first argument-handling context
  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    if (context_stack[context_stack.size() - 1 - i].has_argument_list)
      cptr = &context_stack[context_stack.size() - 1 - i];
  }

  if (!cptr) // should never happen
  {
    argv[0] = nullptr;
    return argv;
  }

  size_t i = 0;
  for (const std::string &s : cptr->argument_list)
    argv[i] = strdup(s.data());

  return argv;
}

void neam::r::shell::variable_stack::free_argument_c_array(char **argv)
{
  for (size_t i = 0; argv[i]; ++i)
    free(argv[i]);
  delete [] argv;
}

const std::deque<std::string> &neam::r::shell::variable_stack::get_argument_array() const
{
  const context *cptr = /*nullptr*/&context_stack.back();

  // get the first argument-handling context
  for (size_t i = 0; i < context_stack.size(); ++i)
  {
    if (context_stack[context_stack.size() - 1 - i].has_argument_list)
    {
      cptr = &context_stack[context_stack.size() - 1 - i];
      break;
    }
  }

  if (!cptr) // should never happen. Too bad we're in reflective, this is typically what would needs an error reporting system.
    cptr = &context_stack.front();

  return cptr->argument_list;
}

void neam::r::shell::variable_stack::set_variable_special_handler(const std::string &name, const std::function< special_variable_handler_sig > &hdlr)
{
  special_variable_list[name] = hdlr;
}

void neam::r::shell::variable_stack::unset_variable_special_handler(const std::string &name)
{
  special_variable_list.erase(name);
}

