

set(PROJ_SOURCES
  ./config.cpp
  ./storage.cpp
  ./stack_entry.cpp
  ./function_call.cpp
  ./introspect.cpp
  ./if_wont_fail.cpp
  ./signal.cpp
  ./measure_point.cpp
)

add_definitions(${PROJ_FLAGS})

add_library(${PROJ_APP} STATIC ${PROJ_SOURCES})
target_link_libraries(${PROJ_APP} ${libpersistence})

# install
install(TARGETS ${PROJ_APP} DESTINATION lib/neam)
install(DIRECTORY ./ DESTINATION include/neam/reflective
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "tools" EXCLUDE        # submodule tools has it own include rules
        PATTERN "persistence" EXCLUDE  # submodule persistence has it own include rules
        PATTERN ".git" EXCLUDE
        PATTERN "cmake" EXCLUDE
        PATTERN "samples" EXCLUDE)

