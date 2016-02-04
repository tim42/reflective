##
## CMAKE file for neam/reflective callgraph2dot tool
##

cmake_minimum_required(VERSION 2.8)

set(TOOL_NAME "callgraph2dot")
# set the name of the tool

set(srcs    ./main.cpp
)

add_definitions(${PROJ_FLAGS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROJ_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${PROJ_FLAGS}")

add_executable(${TOOL_NAME} ${srcs})
target_link_libraries(${TOOL_NAME} ${Boost_PROGRAM_OPTIONS_LIBRARY} tools-common ${PROJ_APP} ${libntools})

# install that tool
install(TARGETS ${TOOL_NAME} DESTINATION bin/neam)

