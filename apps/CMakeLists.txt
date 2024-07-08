function(add_app target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} PRIVATE bitsy)

  message(STATUS "Enabled app: ${target}")
endfunction()

add_app(ads_programm ads_programm.cpp util/query.hpp util/io.hpp util/io.cpp util/timer.hpp)
add_app(input_generator input_generator.cpp util/query.hpp)
