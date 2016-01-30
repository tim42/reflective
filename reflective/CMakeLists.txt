

set(PROJ_SOURCES
  ./storage.cpp
  ./stack_entry.cpp
  ./function_call.cpp
)

add_definitions(${PROJ_FLAGS} "-fno-whole-program")

add_library(${PROJ_APP} STATIC ${PROJ_SOURCES})
target_link_libraries(${PROJ_APP} ${libpersistence})
