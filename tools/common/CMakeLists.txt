
cmake_minimum_required(VERSION 2.8)

set(TOOL_NAME "tools-common")
# set the name of the sample

# avoid listing all the files
set(srcs  ./dot_gen.cpp
)

add_definitions(${PROJ_FLAGS})

add_library(${TOOL_NAME} STATIC ${srcs})
#target_link_libraries(${TOOL_NAME} ${PROJ_APP} ${libntools})
