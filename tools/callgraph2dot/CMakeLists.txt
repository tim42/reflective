##
## CMAKE file for neam/nsched test sample
##

cmake_minimum_required(VERSION 2.8)

set(TOOL_NAME "callgraph2dot")
if (Boost_PROGRAM_OPTIONS_FOUND)
  # set the name of the sample

  # avoid listing all the files
  set(srcs  ./dot_gen.cpp
            ./main.cpp
  )

  add_definitions(${PROJ_FLAGS})
  include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROJ_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${PROJ_FLAGS}")

  add_executable(${TOOL_NAME} ${srcs})
  target_link_libraries(${TOOL_NAME} ${PROJ_APP} ${libntools} ${Boost_PROGRAM_OPTIONS_LIBRARY})

  # install that tool
  install(TARGETS ${TOOL_NAME} DESTINATION bin/neam)
else()
  message(STATUS "Skipping ${TOOL_NAME}: missing boost/program_options")
endif()

