# Fetch the Google Testing framework
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
  GIT_SHALLOW TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

function(add_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE GTest::gtest_main bitsy)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    gtest_discover_tests(${target})

    message(STATUS "Enabled test: ${target}")
endfunction()

add_test(test_bitvector_access bitvector_access_test.cpp)
add_test(test_bitvector_rank bitvector_rank_test.cpp)
add_test(test_bitvector_select bitvector_select_test.cpp)
