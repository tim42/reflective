
cmake_minimum_required(VERSION 2.8)

set(TOOL_NAME "reflective2json")
# set the name of the sample

set(srcs  ./main.cpp
)

add_definitions(${PROJ_FLAGS})

add_executable(${TOOL_NAME} ${srcs})
target_link_libraries(${TOOL_NAME} ${PROJ_APP} ${libntools})

# install that tool
install(TARGETS ${TOOL_NAME} DESTINATION bin/neam)

