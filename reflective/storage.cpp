
#include <fstream>
#include <ctime>
#include <set>
#include <algorithm>

#include "tools/logger/logger.hpp"
#include "storage.hpp"
#include "function_call.hpp"

#include "persistence_metadata.hpp"
#include "config.hpp"

using root_data = std::deque<neam::r::internal::data>;

static root_data *root_ptr = nullptr;
static neam::r::internal::data *global_ptr = nullptr;

static std::set<neam::r::internal::thread_local_data *> tl_data_ptrs;
static thread_local neam::r::internal::thread_local_data tl_data;

static neam::r::internal::mutex_type internal_lock;

neam::r::internal::thread_local_data::thread_local_data()
{
  std::lock_guard<neam::r::internal::mutex_type> _u0(internal_lock);
  tl_data_ptrs.emplace(this);
}

neam::r::internal::thread_local_data::~thread_local_data()
{
  std::lock_guard<neam::r::internal::mutex_type> _u0(internal_lock);
  tl_data_ptrs.erase(this);
}

neam::r::internal::thread_local_data *neam::r::internal::get_thread_data()
{
  return &tl_data;
}

std::set<neam::r::internal::thread_local_data *> &neam::r::internal::get_all_thread_data()
{
  return tl_data_ptrs;
}

neam::r::internal::data *neam::r::internal::get_global_data()
{
  if (!global_ptr) // don't lock each time
  {
    std::lock_guard<neam::r::internal::mutex_type> _u0(internal_lock);
    if (!global_ptr)
    {
      load_data_from_disk(conf::out_file);
      if (!root_ptr)
        root_ptr = new root_data;
      if (!global_ptr)
      {
        if (root_ptr->empty())
          root_ptr->emplace_back(neam::r::internal::data());
        global_ptr = &root_ptr->back();
      }
    }
  }
  return global_ptr;
}

neam::r::internal::call_info_struct &neam::r::internal::_get_call_info_struct(const func_descriptor &d, long &index)
{
  data *global = get_global_data(); // call info structs are located in the global thread

  std::lock_guard<mutex_type> _u0(global->lock); // lock 'cause we do a search and create if not present.

  // search
  index = 0;
  for (call_info_struct &it : global->func_info)
  {
    // doing everything to call as least as possible strcmp...
    // NOTE: the first bit tells if the hash comes from a string (in that case we have to strcmp) or from a pointer
    if (d == it.descr)
    {
      // set properties if not already present
      if (it.descr.pretty_name.empty() && !d.pretty_name.empty())
        it.descr.pretty_name = d.pretty_name;
      if (it.descr.name.empty() && !d.name.empty())
        it.descr.name = d.name;
      if (it.descr.file.empty() && !d.file.empty())
      {
        it.descr.file = d.file;
        it.descr.line = d.line;
      }
      if (!it.descr.key_hash && d.key_hash)
        it.descr.key_hash = d.key_hash;
      if (it.descr.key_name.empty() && !d.key_name.empty())
        it.descr.key_hash = d.key_hash;
      // done !
      return it;
    }
    ++index;
  }

  // before creating it, check that the descriptor is a valid one
  if (d.key_name.empty() && (!d.key_hash || ((d.key_hash & 0x01) != 0)))
    throw std::runtime_error("reflective: invalid func_descriptor structure when registering a new call_info_struct: no key_name or no unique key_hash");

  // nothing found: create it
  index = global->func_info.size();
  global->func_info.emplace_back(call_info_struct{d});
  neam::r::internal::call_info_struct &ret = global->func_info.back();

  return ret;
}

neam::r::internal::call_info_struct *neam::r::internal::_get_call_info_struct_search_only(const func_descriptor &d, long int &index)
{
  data *global = get_global_data(); // call info structs are located in the global thread

  std::lock_guard<mutex_type> _u0(global->lock); // lock 'cause we do a search and create if not present.

  // search
  index = 0;
  for (call_info_struct &it : global->func_info)
  {
    // doing everything to call as least as possible strcmp...
    // NOTE: the first bit tells if the hash comes from a string (in that case we have to strcmp) or from a pointer
    if (d == it.descr)
    {
      // found !
      return &it;
    }
    ++index;
  }
  return nullptr;
}

neam::r::internal::call_info_struct &neam::r::internal::get_call_info_struct_at_index(long int index)
{
  data *global = get_global_data(); // call info structs are located in the global thread

  // No need to lock anything here

  if (index >= (long)global->func_info.size() || index < 0)
    throw std::out_of_range("r::internal::get_call_info_struct_at_index has been given an out-of-range index");

  return global->func_info[index];
}

void neam::r::internal::cleanup_reflective_data()
{
  // begin with the current thread (we may crash on some other thread)
  while (get_thread_data()->top)
  {
    get_thread_data()->top->~function_call();
  }

  for (thread_local_data *it : tl_data_ptrs)
  {
    while (it->top)
    {
      it->top->~function_call();
    }
  }
}

void neam::r::sync_data_to_disk(const std::string &file)
{
  std::lock_guard<neam::r::internal::mutex_type> _u0(internal_lock);
  neam::cr::raw_data serialized_data;

  if (root_ptr == nullptr)
  {
    neam::cr::out.warning() << LOGGER_INFO << "Empty data, will not overwrite/create '" << file << "'" << std::endl;
    return;
  }

  serialized_data = neam::cr::persistence::serialize<neam::cr::persistence_backend::neam>(root_ptr);

  if (!serialized_data.size)
  {
    neam::cr::out.warning() << LOGGER_INFO << "Empty data, will not overwrite/create '" << file << "'" << std::endl;
    return;
  }

  std::ofstream of(file, std::ios_base::binary);
  of.write((const char *)serialized_data.data, serialized_data.size);
  of.flush();
  of.close();

  neam::cr::out.debug() << LOGGER_INFO << "Wrote '" << file << "'" << std::endl;
}

std::string neam::r::get_data_as_json()
{
  std::lock_guard<neam::r::internal::mutex_type> _u0(internal_lock);
  neam::cr::raw_data serialized_data;

  if (root_ptr == nullptr)
    return std::string();

  serialized_data = neam::cr::persistence::serialize<neam::cr::persistence_backend::json>(root_ptr);

  if (serialized_data.size <= 1)
    return std::string();

  return (const char *)(serialized_data.data);
}

bool neam::r::load_data_from_disk(const std::string &file)
{
  if (root_ptr)
  {
    delete root_ptr;
    root_ptr = nullptr;
  }
  global_ptr = nullptr;

  std::string contents;
  std::ifstream inf(file, std::ios_base::binary);

  if (!inf)
  {
    neam::cr::out.warning() << LOGGER_INFO << "Failed to load '" << file << "': file does not exists" << std::endl;
    return false;
  }

  inf.seekg(0, std::ios_base::end);
  long size = inf.tellg();
  inf.seekg(0, std::ios_base::beg);

  if (!size || size < 0)
  {
    neam::cr::out.warning() << LOGGER_INFO << "Failed to load '" << file << "': empty file" << std::endl;
    return false;
  }

  char *memory = new char[size + 1];

  inf.read(memory, size);
  memory[size] = 0;

  cr::raw_data serialized_data;
  serialized_data.ownership = false;
  serialized_data.data = (int8_t *)memory;
  serialized_data.size = size;

  root_ptr = neam::cr::persistence::deserialize<neam::cr::persistence_backend::neam, root_data>(serialized_data);

  delete [] memory;

  if (root_ptr)
  {
    if (root_ptr->size())
    {
      global_ptr = &root_ptr->back();
      ++global_ptr->launch_count;
    }
    for (internal::data &data_it : *root_ptr)
    {
#ifdef _MSC_VER
      data_it.post_deserialization();
#endif
      std::lock_guard<internal::mutex_type> _u0(data_it.lock); // lock 'cause we do a lot of nasty things.

      // walk the whole callgraph to set correct ids
      size_t stack_index = 0;
      for (auto & graph_it : data_it.callgraph)
      {
        size_t index = 0;
        for (internal::stack_entry & it : graph_it)
        {
          const_cast<size_t &>(it.self_index) = index;
          const_cast<size_t &>(it.stack_index) = stack_index;
          ++index;
        }
        ++stack_index;
      }
    }
    neam::cr::out.debug() << LOGGER_INFO << "Loaded '" << file << "'" << std::endl;
    return true;
  }
  else
  {
    neam::cr::out.warning() << LOGGER_INFO << "Failed to load '" << file << "', data is probably corrupted" << std::endl;
    return false;
  }
}

// // // STASH // // //

void neam::r::stash_current_data(const std::string &name)
{
  internal::get_global_data(); // init, if not already done

  // stash it !
  global_ptr->timestamp = time(nullptr);
  root_ptr->push_back(internal::data());
  global_ptr = &root_ptr->back();
  global_ptr->name = name;

  if (std::max(2l, conf::max_stash_count) < long(root_ptr->size()) && conf::max_stash_count >= 0)
    root_ptr->pop_front();
}

bool neam::r::load_data_from_stash(const std::string &data_name)
{
  internal::get_global_data(); // init, if not already done

  for (internal::data &data_it : *root_ptr)
  {
    if (data_it.name == data_name)
    {
      global_ptr = &data_it;
      return true;
    }
  }
  return false;
}

bool neam::r::auto_stash_current_data(const std::string &name)
{
  internal::get_global_data(); // init, if not already done

  if (global_ptr->name.empty())
  {
    global_ptr->name = name;
    return false;
  }
  if (global_ptr->name == name)
    return false;

  stash_current_data(global_ptr->name);
  global_ptr->name = name;
  return true;
}

std::vector<std::string> neam::r::get_stashes_name()
{
  internal::get_global_data(); // init, if not already done

  std::vector<std::string> namevec;
  for (internal::data &data_it : *root_ptr)
  {
    if (data_it.name.empty())
      namevec.push_back("[unnamed]");
    else
      namevec.push_back(data_it.name);
  }
  return namevec;
}

std::vector<long> neam::r::get_stashes_timestamp()
{
  internal::get_global_data(); // init, if not already done

  std::vector<long> tsvec;
  for (internal::data &data_it : *root_ptr)
    tsvec.push_back(data_it.timestamp);
  return tsvec;
}

size_t neam::r::get_active_stash_index()
{
  internal::get_global_data(); // init, if not already done

  size_t index = 0;
  for (internal::data &data_it : *root_ptr)
  {
    if (global_ptr == &data_it)
      return index;
    ++index;
  }
  return -1;
}

