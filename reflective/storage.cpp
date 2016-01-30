
#include <fstream>

#include <tools/logger/logger.hpp>
#include "storage.hpp"

#include "persistence_metadata.hpp"

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
    load_data_from_disk("file.xr");
    if (!global_ptr)
      global_ptr = new data;
  }
  return global_ptr;
}

neam::r::internal::call_info_struct &neam::r::internal::_get_call_info_struct(uint32_t hash, const char *const name, const char *const pretty_name, long &index)
{
  data *global = get_global_data(); // call info structs are located in the global thread

  std::lock_guard<mutex_type> _u0(global->lock); // lock 'cause we do a search and create if not present.

  // search
  index = 0;
  for (auto &it : global->func_info)
  {
    // doing everything to call as least as possible strcmp...
    // NOTE: the first bit tells if the hash comes from a string (in that case we have to strcmp) or from a pointer
    if ((hash && it.name_hash == hash) && (((hash & 0x1) == 0) || name == it.name || !strcmp(name, it.name)))
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
  neam::cr::raw_data serialized_data = neam::cr::persistence::serialize<neam::cr::persistence_backend::json>(global_ptr);

  std::ofstream of(file);
  of.write((const char *)serialized_data.data, serialized_data.size);

  std::cout << "serialized data:\n" << serialized_data.data << "\n" << std::endl;
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

  global_ptr = neam::cr::persistence::deserialize<neam::cr::persistence_backend::json, neam::r::internal::data>(serialized_data);

  delete [] memory;

  if (!global_ptr)
    abort();
  else
    ++global_ptr->launch_count;
}
