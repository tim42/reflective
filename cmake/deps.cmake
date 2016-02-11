

# include files:
# include(cmake/find_mozjs.cmake)

# deps libs (for exec)
set(PROJ_DEPS_LIBS )

# include dirs
set(PROJ_INCLUDE_DIRS )

# boost (tools may require it for argument parsing)
find_package(Boost "1.50" COMPONENTS program_options)
