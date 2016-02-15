
# build the common library
add_subdirectory(common)

# add the common directory to the include search directories
include_directories(common/)

# build the tools
if (Boost_PROGRAM_OPTIONS_FOUND)
  add_subdirectory(callgraph2dot)
else()
  message(STATUS  "Skipping callgraph2dot tool: missing boost/program_options")
endif()

if (NOT Boost_FOUND OR NOT Boost_PROGRAM_OPTIONS_FOUND OR NOT Boost_FILESYSTEM_FOUND OR NOT Boost_SYSTEM_FOUND)
    message(STATUS  "Skipping reflective-shell tool: missing boost/program_options, boost/filesystem, boost/system or boost headers")
else()
  add_subdirectory(shell)
endif()
