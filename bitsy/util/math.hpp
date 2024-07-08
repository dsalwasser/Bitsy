/// Utility functions for math operations.
/// @file math.hpp
/// @author Daniel Salwasser
#pragma once

#include <concepts>
#include <cstddef>
#include <limits>

namespace bitsy::math {

/**
 * Computes the ceiling of an integer division.
 *
 * @tparam Int1 The integer type of the dividend.
 * @tparam Int2 The integer type of the divisor.
 * @param x The dividend.
 * @param y The divisor.
 * @return The ceiling of the integer division.
 */
template <std::integral Int1, std::integral Int2>
[[nodiscard]] constexpr Int1 div_ceil(const Int1 x, const Int2 y) {
  return (x / y) + ((x % y) != 0);
}

/**
 * Computes a power of two.
 *
 * @tparam Int The integer type to operate on.
 * @param n The exponent.
 * @return Returns two raised to the given power.
 */
template <std::integral Int>
[[nodiscard]] constexpr Int pow2(const Int n) {
  return static_cast<Int>(1) << n;
}

/**
 * Rounds a given integer to a multiple of another integer.
 *
 * @tparam Int The integer type to operate on.
 * @param x The integer to round to a multiple of another integer.
 * @param y The integer to round to.
 * @return The integer x rounded to a multiple of the integer y.
 */
template <std::integral Int>
[[nodiscard]] constexpr Int round_to(const Int x, const Int y) {
  return math::div_ceil(x, y) * y;
}

/**
 * Computes a bit mask with consecutive ones of a given length that starts at a
 * given position and zeroes apart from that.
 *
 * @tparam Int The integer type of the bit mask.
 * @param num_set_bits The number of consecutive ones in the mask.
 * @param start The starting bit of the consecutive ones (default is 0).
 * @return The bit mask.
 */
template <std::integral Int>
[[nodiscard]] constexpr Int setbits(const std::size_t num_set_bits,
                                    const std::size_t start = 0) {
  if (num_set_bits == 0) {
    return 0;
  }

  constexpr Int kOnes = std::numeric_limits<Int>::max();
  constexpr std::size_t kWidth = std::numeric_limits<Int>::digits;
  return (kOnes >> static_cast<Int>(kWidth - num_set_bits)) << start;
}

}  // namespace bitsy::math
