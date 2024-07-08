# Fetch source files
file(GLOB_RECURSE BITSY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

# Create bitsy library
add_library(bitsy ${BITSY_SOURCES})
set_target_properties(bitsy PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(bitsy PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# Add compiler options that enable strictly compile-time checks
target_compile_options(bitsy PRIVATE -Wall -Wextra -Wformat -Wformat=2 -Wconversion
  -Wsign-conversion -Wtrampolines -Wimplicit-fallthrough -Wbidi-chars=any
  -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types
  -Werror=int-conversion-Wshadow -Wnon-virtual-dtor -pedantic)
# Being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(bitsy PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
