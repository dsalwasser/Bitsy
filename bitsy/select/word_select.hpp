/// Utility for finding the position of the i-th set bit in an integer.
/// @file word_select.hpp
/// @author Daniel Salwasser
#pragma once

#include <bit>
#include <concepts>
#include <cstddef>
#include <limits>

#if defined(BITSY_USE_PDEP) && defined(__BMI2__)
#define USE_PDEP
#endif

#ifdef USE_PDEP
#include <immintrin.h>
#endif

namespace bitsy {

/**
 * Returns the position of the rank-th set bit in an integer.
 *
 * @tparam kUseBinarySearch Whether to use a binary search to find the position.
 * @tparam kOverwriteFastPDEP Whether to not use the PDEP implementation.
 * @tparam Int The type of integer to operate on.
 * @param word The word in which to find the position.
 * @param rank The position of the first one with given bit-rank.
 */
template <bool kUseBinarySearch = false,
          bool kOverwriteFastPDEP = false,
          std::integral Int>
[[nodiscard]] inline constexpr Int word_select1(Int word, std::size_t rank) {
  if constexpr (!kOverwriteFastPDEP) {
#ifdef USE_PDEP
    // The following implementation is due to the following source:
    // https://stackoverflow.com/a/27453505
    const std::size_t rank_th_one = static_cast<std::size_t>(1) << (rank - 1);
    return std::countr_zero(_pdep_u64(rank_th_one, word));
#endif
  }

  if constexpr (kUseBinarySearch) {
    constexpr Int kIntWidth = std::numeric_limits<Int>::digits;

    Int pos = 0;
    Int length = kIntWidth;
    while (length > 1) {
      const Int half = length / 2;
      pos += (std::popcount(word << (kIntWidth - (pos + half))) < rank) * half;
      length -= half;
    }

    return pos;
  } else {
    Int pos = 0;

    while (rank > 0) {
      rank -= word & static_cast<Int>(1);
      word >>= 1;
      pos++;
    }

    return pos - 1;
  }
}

}  // namespace bitsy
