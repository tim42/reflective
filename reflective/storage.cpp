
#include <fstream>

#include <tools/logger/logger.hpp>
#include "storage.hpp"

#include "persistence_metadata.hpp"
#include "config.hpp"

static neam::r::internal::data *global_ptr = nullptr;
static thread_local neam::r::internal::thread_local_data *tl_ptr = nullptr;

neam::r::internal::thread_local_data *neam::r::internal::get_thread_data()
{
  if (!tl_ptr)
    tl_ptr = new thread_local_data;
  return tl_ptr;
}

neam::r::internal::data *neam::r::internal::get_global_data()
{
  if (!global_ptr)
  {
    load_data_from_disk(conf::out_file);
    if (!global_ptr)
      global_ptr = new data;
  }
  return global_ptr;
}

static inline bool call_info_cmp(uint32_t hash, const char *const name, neam::r::internal::call_info_struct &info)
{
  return ((!hash || info.name_hash == hash) && (((hash & 0x1) == 0 && hash) || name == info.name || !strcmp(name, info.name)));
}

neam::r::internal::call_info_struct &neam::r::internal::_get_call_info_struct(uint32_t hash, const char *const name, const char *const pretty_name, long &index)
{
  data *global = get_global_data(); // call info structs are located in the global thread

  std::lock_guard<mutex_type> _u0(global->lock); // lock 'cause we do a search and create if not present.

  // search
  index = 0;
  for (call_info_struct &it : global->func_info)
  {
    // doing everything to call as least as possible strcmp...
    // NOTE: the first bit tells if the hash comes from a string (in that case we have to strcmp) or from a pointer
    if (call_info_cmp(hash, name, it))
    {
      // set the pretty name if not already present
      if (!it.pretty_name && pretty_name)
        it.pretty_name = pretty_name;
      // done !
      return it;
    }
    ++index;
  }

  // nothing found: create it
  index = global->func_info.size();
  global->func_info.emplace_back(call_info_struct{name, hash, pretty_name});

  return global->func_info.back();
}

neam::r::internal::call_info_struct *neam::r::internal::_get_call_info_struct_search_only(uint32_t hash, const char *const name, const char *const pretty_name, long int &index)
{
  data *global = get_global_data(); // call info structs are located in the global thread

  std::lock_guard<mutex_type> _u0(global->lock); // lock 'cause we do a search and create if not present.

  // search
  index = 0;
  for (call_info_struct &it : global->func_info)
  {
    // doing everything to call as least as possible strcmp...
    // NOTE: the first bit tells if the hash comes from a string (in that case we have to strcmp) or from a pointer
    if (call_info_cmp(hash, name, it))
    {
      // set the pretty name if not already present
      if (!it.pretty_name && pretty_name)
        it.pretty_name = pretty_name;
      // done !
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

void neam::r::sync_data_to_disk(const std::string &file)
{
  neam::cr::raw_data serialized_data = neam::cr::persistence::serialize<neam::cr::persistence_backend::neam>(global_ptr);

  std::ofstream of(file);
  of.write((const char *)serialized_data.data, serialized_data.size);

    neam::cr::out.debug() << LOGGER_INFO << "Wrote '" << file << "'" << std::endl;
}

void neam::r::load_data_from_disk(const std::string &file)
{
  if (global_ptr)
  {
    delete global_ptr;
    global_ptr = nullptr;
  }

  std::string contents;
  std::ifstream inf(file);

  inf.seekg(0, std::ios_base::end);
  long size = inf.tellg();
  inf.seekg(0, std::ios_base::beg);

  if (!size || size < 0)
    return;

  char *memory = new char[size + 1];

  inf.read(memory, size);
  memory[size] = 0;

  cr::raw_data serialized_data;
  serialized_data.ownership = false;
  serialized_data.data = (int8_t *)memory;
  serialized_data.size = size;

  global_ptr = neam::cr::persistence::deserialize<neam::cr::persistence_backend::neam, neam::r::internal::data>(serialized_data);

  delete [] memory;

  if (global_ptr)
  {
    ++global_ptr->launch_count;
    std::lock_guard<internal::mutex_type> _u0(global_ptr->lock); // lock 'cause we do a lot of nasty things.

    // walk the whole callgraph to set correct ids
    size_t stack_index = 0;
    for (auto & graph_it : global_ptr->callgraph)
    {
      size_t index = 0;
      for (internal::stack_entry &it : graph_it)
      {
        const_cast<size_t &>(it.self_index) = index;
        const_cast<size_t &>(it.stack_index) = stack_index;
        ++index;
      }
      ++stack_index;
    }
    neam::cr::out.debug() << LOGGER_INFO << "Loaded '" << file << "'" << std::endl;
  }
  else
  {
    neam::cr::out.warning() << LOGGER_INFO << "Failed to load '" << file << "', data is probably corrupted" << std::endl;
  }
}
