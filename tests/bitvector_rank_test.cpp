#include <gtest/gtest.h>

#include <ranges>

#include <bitsy/bitvector.hpp>
#include <bitsy/rank/naive_rank.hpp>
#include <bitsy/rank/two_layer_rank_combined_bitvector.hpp>

#include "bitvector_util.hpp"

namespace {
using namespace bitsy;
using namespace bitsy::testing;

constexpr auto kLengths = {0,   1,   63,    64,    65,    511,
                           512, 513, 16383, 16384, 16385, math::pow2(22) + 7};

template <type_traits::BitVector BitVector, type_traits::Rank Rank>
void test_rank(const BitVector& bitvector, const Rank& rank) {
  const std::size_t length = bitvector.length();

  std::size_t cur_rank = 0;
  for (std::size_t pos = 0; pos < length; ++pos) {
    EXPECT_EQ(pos - cur_rank, rank.rank0(pos));
    EXPECT_EQ(cur_rank, rank.rank1(pos));

    cur_rank += static_cast<std::size_t>(bitvector.is_set(pos) ? 1 : 0);
  }
}

template <type_traits::RankCombinedBitVector RankBitVector>
void test_combined_rank(const RankBitVector& bitvector) {
  const std::size_t length = bitvector.length();

  std::size_t cur_rank = 0;
  for (std::size_t pos = 0; pos < length; ++pos) {
    EXPECT_EQ(pos - cur_rank, bitvector.rank0(pos));
    EXPECT_EQ(cur_rank, bitvector.rank1(pos));

    cur_rank += static_cast<std::size_t>(bitvector.is_set(pos) ? 1 : 0);
  }
}

template <type_traits::BitVector BitVector, type_traits::Rank Rank>
void test_rank_uniform() {
  for (const std::size_t length : kLengths) {
    const BitVector bitvector_u0(length, false);
    const BitVector bitvector_u1(length, true);

    const Rank rank_u0(bitvector_u0);
    const Rank rank_u1(bitvector_u1);

    test_rank(bitvector_u0, rank_u0);
    test_rank(bitvector_u1, rank_u1);
  }
}

template <type_traits::RankCombinedBitVector RankBitVector>
void test_rank_combined_uniform() {
  for (const std::size_t length : kLengths) {
    const RankBitVector bitvector_u0(length, false);
    const RankBitVector bitvector_u1(length, true);

    test_combined_rank(bitvector_u0);
    test_combined_rank(bitvector_u1);
  }
}

template <type_traits::BitVector BitVector, type_traits::Rank Rank>
void test_rank_alternating() {
  for (const std::size_t length : kLengths) {
    const auto bitvector_p2 = create_alternating_bitvec<BitVector>(length, 2);
    const auto bitvector_p5 = create_alternating_bitvec<BitVector>(length, 5);
    const auto bitvector_p16 = create_alternating_bitvec<BitVector>(length, 16);

    const Rank rank_p2(bitvector_p2);
    const Rank rank_p5(bitvector_p5);
    const Rank rank_p16(bitvector_p16);

    test_rank(bitvector_p2, rank_p2);
    test_rank(bitvector_p5, rank_p5);
    test_rank(bitvector_p16, rank_p16);
  }
}

template <type_traits::RankCombinedBitVector RankBitVector>
void test_rank_combined_alternating() {
  for (const std::size_t length : kLengths) {
    auto bitvector_p2 = create_alternating_bitvec<RankBitVector>(length, 2);
    auto bitvector_p5 = create_alternating_bitvec<RankBitVector>(length, 5);
    auto bitvector_p16 = create_alternating_bitvec<RankBitVector>(length, 16);

    bitvector_p2.update();
    bitvector_p5.update();
    bitvector_p16.update();

    test_combined_rank(bitvector_p2);
    test_combined_rank(bitvector_p5);
    test_combined_rank(bitvector_p16);
  }
}

template <type_traits::BitVector BitVector, type_traits::Rank Rank>
void test_rank_random() {
  for (const std::size_t length : kLengths) {
    for (const float fillratio : {0.1, 0.25, 0.75, 0.9}) {
      for (const float seed : std::views::iota(1, 10)) {
        const auto bitvector =
            create_random_bitvec<BitVector>(length, fillratio, seed);
        const Rank rank(bitvector);
        test_rank(bitvector, rank);
      }
    }
  }
}

template <type_traits::RankCombinedBitVector RankBitVector>
void test_rank_combined_random() {
  for (const std::size_t length : kLengths) {
    for (const float fillratio : {0.1, 0.25, 0.75, 0.9}) {
      for (const float seed : std::views::iota(1, 10)) {
        auto bitvector =
            create_random_bitvec<RankBitVector>(length, fillratio, seed);
        bitvector.update();
        test_combined_rank(bitvector);
      }
    }
  }
}

TEST(NaiveRankTest, Uniform) {
  test_rank_uniform<BitVector, NaiveRank<BitVector>>();
}

TEST(NaiveRankTest, Alternating) {
  test_rank_alternating<BitVector, NaiveRank<BitVector>>();
}

TEST(TwoLayerRankCombinedBitVectorTest, Uniform) {
  test_rank_combined_uniform<TwoLayerRankCombinedBitVector<>>();
  test_rank_combined_uniform<TwoLayerRankCombinedBitVector<1024, 15>>();
}

TEST(TwoLayerRankCombinedBitVectorTest, Alternating) {
  test_rank_combined_alternating<TwoLayerRankCombinedBitVector<>>();
  test_rank_combined_alternating<TwoLayerRankCombinedBitVector<1024, 15>>();
}

TEST(TwoLayerRankCombinedBitVectorTest, Random) {
  test_rank_combined_random<TwoLayerRankCombinedBitVector<>>();
  test_rank_combined_random<TwoLayerRankCombinedBitVector<1024, 15>>();
}

}  // namespace
