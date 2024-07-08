/// A simple timer to measure the execution time of functions.
/// @file timer.hpp
/// @author Daniel Salwasser
#pragma once

#include <chrono>
#include <concepts>

namespace bitsy {

/**
 * Times a function.
 *
 * @tparam Lambda The type of (lambda) function whose execution time to measure.
 * @param l The function whose execution time to measure.
 * @return The time in milliseconds it took the function to execute.
 */
template <std::invocable Lambda>
std::size_t time_function(Lambda&& l) {
  using namespace std::chrono;

  const auto start = system_clock::now();
  l();
  const auto end = system_clock::now();

  return duration_cast<milliseconds>(end - start).count();
}

}  // namespace bitsy
