#pragma once

#include <cstddef>
#include <random>

#include <bitsy/type_traits.hpp>

namespace bitsy::testing {

template <type_traits::BitVector BitVector>
BitVector create_alternating_bitvec(const std::size_t length,
                                    const std::size_t period) {
  BitVector bitvector(length);

  for (std::size_t pos = 0; pos < length; ++pos) {
    const bool is_set = (pos % period) == 0;
    bitvector.set(pos, is_set);
  }

  return bitvector;
}

template <type_traits::BitVector BitVector>
BitVector create_random_bitvec(const std::size_t length,
                               const float fill_ratio,
                               const std::size_t seed) {
  BitVector bitvector(length);

  std::mt19937 gen(seed);
  std::bernoulli_distribution dist(fill_ratio);
  for (std::size_t pos = 0; pos < length; ++pos) {
    const bool is_set = dist(gen);
    bitvector.set(pos, is_set);
  }

  return bitvector;
}

template <type_traits::BitVector BitVector>
std::size_t count_ones(const BitVector& bitvector) {
  std::size_t num_ones = 0;

  const std::size_t length = bitvector.length();
  for (std::size_t pos = 0; pos < length; ++pos) {
    num_ones += bitvector.is_set(pos) ? 1 : 0;
  }

  return num_ones;
}

}  // namespace bitsy::testing
