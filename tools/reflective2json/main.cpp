
#include <fstream>

#include <reflective/reflective.hpp> // The reflective header
#include <tools/logger/logger.hpp>   // Just to set the logger in debug mode

int main(int argc, char **argv)
{
  // Set the reflective configuration
  neam::r::conf::disable_auto_save = true;
  neam::r::conf::out_file = "";

  if (argc != 2)
  {
    neam::cr::out.log() << LOGGER_INFO << "Usage: " << argv[0] << " [reflective-file]" << neam::cr::newline
                        << "  it will then create (if the file is valid) a [reflective-file].json that will contain the JSON data" << std::endl;
    return 1;
  }

  // load the file
  if (!neam::r::load_data_from_disk(argv[1]))
  {
    neam::cr::out.error() << LOGGER_INFO << "Error: Unable to load the file '" << argv[1] << "'. No output produced." << std::endl;
    return 2;
  }

  // get the JSON
  std::string result = neam::r::get_data_as_json();

  if (result.empty())
  {
    neam::cr::out.error() << LOGGER_INFO << "Error: Empty JSON data. No output produced." << std::endl;
    return 3;
  }

  // write the JSON file
  std::string out_file = std::string(argv[1]) + ".json";
  std::ofstream out(out_file, std::ios_base::trunc);

  if (!out)
  {
    neam::cr::out.error() << LOGGER_INFO << "Error: Unable to create/truncate '" << out_file << "'. No output produced." << std::endl;
    return 4;
  }

  out << result;
  out.flush();
  out.close();

  return 0;
}
