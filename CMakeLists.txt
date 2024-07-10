cmake_minimum_required(VERSION 3.16...3.29)

project(Bitsy
  VERSION 1.0.0
  DESCRIPTION "An itsy-bitsy bit vector with rank and select support"
  LANGUAGES CXX
)

set(PROJECT_VENDOR "Daniel Salwasser")
set(PROJECT_CONTACT "danielsalwater@gmail.com")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
if (CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Add configuration options for Bitsy.
option(BITSY_HUGE_PAGES "Use huge pages." ON)
option(BITSY_USE_PDEP "Use PDEP instructions." ON)

# Add build options for Bitsy.
option(BUILD_WITH_MTUNE_NATIVE "Build with -mtune=native." ON)
option(BUILD_WITH_DEBUG_SYMBOLS "Build with debug symbols." ON)
option(BUILD_WITH_CCACHE "Use ccache to build." ON)

# Add options for test and benchmark targets.
option(BUILD_TESTS "Build unit tests." OFF)
option(BUILD_BENCHMARKS "Build benchmark binaries." OFF)

# Log configuration and setup flags etc.
message(STATUS "Following configuration is used for Bitsy:")
if (BITSY_HUGE_PAGES)
  add_definitions(-DBITSY_HUGE_PAGES)
  message(STATUS "> Huge Pages: enabled")
else ()
  message(STATUS "> Huge Pages: disabled")
endif ()

if (BITSY_USE_PDEP)
  add_definitions(-DBITSY_USE_PDEP)
  add_compile_options(-mbmi2)
  message(STATUS "> Use PDEP: enabled")
else ()
  message(STATUS "> Use PDEP: disabled")
endif ()

if (BUILD_WITH_MTUNE_NATIVE)
  add_compile_options(-mtune=native -march=native)
  message(STATUS "> Use -mtune=native: enabled")
else ()
  message(STATUS "> Use -mtune=native: disabled")
endif ()

if (BUILD_WITH_DEBUG_SYMBOLS)
  add_compile_options(-g -g3)
  message(STATUS "> Debug Symbols: enabled")
else ()
  message(STATUS "> Debug Symbols: disabled")
endif ()

if (BUILD_WITH_CCACHE)
  find_program(CCACHE_PROGRAM ccache)

  if (CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  endif ()
endif ()

# We use FetchContent for testing and benchmarking dependencies
include(FetchContent)

# Start include paths on project root
include_directories(${PROJECT_SOURCE_DIR})

# Add components
add_subdirectory(bitsy)
add_subdirectory(apps)

if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif ()

if (BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif ()
