# Fetch Ankerl's nanobench microbenchmarking library
FetchContent_Declare(
  nanobench
  GIT_REPOSITORY https://github.com/martinus/nanobench.git
  GIT_TAG v4.3.11
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nanobench)

function(add_benchmark target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} PRIVATE nanobench bitsy)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  message(STATUS "Enabled benchmark: ${target}")
endfunction()

add_benchmark(benchmark_bitvector_access bitvector_access_benchmark.cpp)
add_benchmark(benchmark_bitvector_rank bitvector_rank_benchmark.cpp)
add_benchmark(benchmark_bitvector_select bitvector_select_benchmark.cpp)
add_benchmark(benchmark_word_select word_select_benchmark.cpp)
