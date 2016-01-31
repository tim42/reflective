

set(PROJ_SOURCES
  ./config.cpp
  ./storage.cpp
  ./stack_entry.cpp
  ./function_call.cpp
  ./introspect.cpp
  ./if_wont_fail.cpp
  ./signal.cpp
)

add_definitions(${PROJ_FLAGS} "-fno-whole-program")

add_library(${PROJ_APP} STATIC ${PROJ_SOURCES})
target_link_libraries(${PROJ_APP} ${libpersistence})
