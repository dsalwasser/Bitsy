#include <gtest/gtest.h>

#include <ranges>

#include <bitsy/bitvector.hpp>
#include <bitsy/rank/two_layer_rank_combined_bitvector.hpp>
#include <bitsy/type_traits.hpp>

#include "bitvector_util.hpp"

namespace {
using namespace bitsy;
using namespace bitsy::testing;

constexpr auto kLengths = {0,   1,   63,    64,    65,    511,
                           512, 513, 16383, 16384, 16385, math::pow2(22) + 7};

template <type_traits::BitVector BitVector>
void test_access_uniform() {
  for (const std::size_t length : kLengths) {
    const BitVector bitvector_u0(length, false);
    const BitVector bitvector_u1(length, true);

    for (std::size_t i = 0; i < length; ++i) {
      EXPECT_FALSE(bitvector_u0.is_set(i));
      EXPECT_TRUE(bitvector_u1.is_set(i));
    }
  }
}

template <type_traits::BitVector BitVector>
void test_access_alternating() {
  for (const std::size_t length : kLengths) {
    const auto bitvector_p2 = create_alternating_bitvec<BitVector>(length, 2);
    const auto bitvector_p5 = create_alternating_bitvec<BitVector>(length, 5);
    const auto bitvector_p16 = create_alternating_bitvec<BitVector>(length, 16);

    for (std::size_t i = 0; i < length; ++i) {
      EXPECT_EQ(bitvector_p2.is_set(i), (i % 2) == 0);
      EXPECT_EQ(bitvector_p5.is_set(i), (i % 5) == 0);
      EXPECT_EQ(bitvector_p16.is_set(i), (i % 16) == 0);
    }
  }
}

template <type_traits::BitVector ReferenceBitVector,
          type_traits::BitVector BitVector>
void test_access_random() {
  for (const std::size_t length : kLengths) {
    for (const float fillratio : {0.1, 0.25, 0.75, 0.9}) {
      for (const float seed : std::views::iota(1, 10)) {
        const auto reference =
            create_random_bitvec<ReferenceBitVector>(length, fillratio, seed);
        const auto bitvector =
            create_random_bitvec<BitVector>(length, fillratio, seed);

        for (std::size_t i = 0; i < length; ++i) {
          EXPECT_EQ(reference.is_set(i), bitvector.is_set(i));
        }
      }
    }
  }
}

TEST(BitVectorAccessTest, Uniform) {
  test_access_uniform<BitVector>();
}

TEST(BitVectorAccessTest, Alternating) {
  test_access_alternating<BitVector>();
}

TEST(TwoLayerRankCombinedBitVectorAccessTest, Uniform) {
  test_access_uniform<TwoLayerRankCombinedBitVector<>>();
  test_access_uniform<TwoLayerRankCombinedBitVector<1024, 15>>();
}

TEST(TwoLayerRankCombinedBitVectorAccessTest, Alternating) {
  test_access_alternating<TwoLayerRankCombinedBitVector<>>();
  test_access_alternating<TwoLayerRankCombinedBitVector<1024, 15>>();
}

TEST(TwoLayerRankCombinedBitVectorAccessTest, Random) {
  test_access_random<BitVector, TwoLayerRankCombinedBitVector<>>();
  test_access_random<BitVector, TwoLayerRankCombinedBitVector<1024, 15>>();
}

}  // namespace
