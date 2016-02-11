
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

if (Boost_FOUND)
  if (Boost_PROGRAM_OPTIONS_FOUND)
    add_subdirectory(shell)
  else()
    message(STATUS  "Skipping reflective-shell tool: missing boost/program_options")
  endif()
else()
  message(STATUS  "Skipping reflective-shell tool: missing boost headers")
endif()
