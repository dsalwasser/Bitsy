#include <gtest/gtest.h>

#include <ranges>

#include <bitsy/bitvector.hpp>
#include <bitsy/rank/naive_rank.hpp>
#include <bitsy/rank/two_layer_rank_combined_bitvector.hpp>
#include <bitsy/select/naive_select.hpp>
#include <bitsy/select/two_layer_select.hpp>
#include <bitsy/type_traits.hpp>

#include "bitvector_util.hpp"

namespace {
using namespace bitsy;
using namespace bitsy::testing;

constexpr std::size_t kLength = math::pow2(22) + 7;

constexpr auto kLengths = {0,   1,   63,    64,    65,    511,
                           512, 513, 16383, 16384, 16385, math::pow2(22) + 7};

template <type_traits::BitVector BitVector, type_traits::Select Select>
void test_select(const BitVector& bitvector, const Select& select) {
  const std::size_t length = bitvector.length();

  std::size_t cur_zero = 0;
  std::size_t cur_one = 0;
  for (std::size_t pos = 0; pos < length; ++pos) {
    if (bitvector.is_set(pos)) {
      EXPECT_EQ(pos, select.select1(++cur_one));
    } else {
      EXPECT_EQ(pos, select.select0(++cur_zero));
    }
  }
}

template <type_traits::BitVector BitVector, type_traits::Select Select>
void test_select_uniform() {
  for (const std::size_t length : kLengths) {
    const BitVector bitvector_u0(length, false);
    const BitVector bitvector_u1(length, true);

    const Select select_u0(bitvector_u0, 0);
    const Select select_u1(bitvector_u1, length);

    test_select(bitvector_u0, select_u0);
    test_select(bitvector_u1, select_u1);
  }
}

template <type_traits::BitVector BitVector,
          type_traits::Select Select,
          const bool kInit = false>
void test_select_alternating() {
  for (const std::size_t length : kLengths) {
    auto bitvector_p2 = create_alternating_bitvec<BitVector>(length, 2);
    auto bitvector_p5 = create_alternating_bitvec<BitVector>(length, 5);
    auto bitvector_p19 = create_alternating_bitvec<BitVector>(length, 19);

    if constexpr (kInit) {
      bitvector_p2.update();
      bitvector_p5.update();
      bitvector_p19.update();
    }

    const Select select_p2(bitvector_p2, count_ones(bitvector_p2));
    const Select select_p5(bitvector_p5, count_ones(bitvector_p5));
    const Select select_p19(bitvector_p19, count_ones(bitvector_p19));

    test_select(bitvector_p2, select_p2);
    test_select(bitvector_p5, select_p5);
    test_select(bitvector_p19, select_p19);
  }
}

template <type_traits::BitVector BitVector,
          type_traits::Select Select,
          const bool kInit = false>
void test_select_random() {
  for (const std::size_t length : kLengths) {
    for (const float fillratio : {0.1, 0.25, 0.75, 0.9}) {
      for (const float seed : std::views::iota(1, 10)) {
        auto bitvector =
            create_random_bitvec<BitVector>(length, fillratio, seed);

        if constexpr (kInit) {
          bitvector.update();
        }

        const Select select(bitvector, count_ones(bitvector));
        test_select(bitvector, select);
      }
    }
  }
}

TEST(NaiveSelectTest, Uniform) {
  test_select_uniform<BitVector, NaiveSelect<BitVector>>();
}

TEST(NaiveSelectTest, Alternating) {
  test_select_alternating<BitVector, NaiveSelect<BitVector>>();
}

TEST(NaiveSelectTest, Random) {
  test_select_random<BitVector, NaiveSelect<BitVector>>();
}

TEST(TwoLayerSelectTestLinearSearch, Uniform) {
  using BitVector = TwoLayerRankCombinedBitVector<>;
  using BitVector1024 = TwoLayerRankCombinedBitVector<1024, 15>;

  test_select_uniform<BitVector, TwoLayerSelect<BitVector, false>>();
  test_select_uniform<BitVector1024, TwoLayerSelect<BitVector1024, false>>();
}
TEST(TwoLayerSelectTestLinearSearch, Alternating) {
  using BitVector = TwoLayerRankCombinedBitVector<>;
  using BitVector1024 = TwoLayerRankCombinedBitVector<1024, 15>;

  test_select_alternating<BitVector, TwoLayerSelect<BitVector, false>, true>();
  test_select_alternating<BitVector1024, TwoLayerSelect<BitVector1024, false>,
                          true>();
}

TEST(TwoLayerSelectTestLinearSearch, Random) {
  using BitVector = TwoLayerRankCombinedBitVector<>;
  using BitVector1024 = TwoLayerRankCombinedBitVector<1024, 15>;

  test_select_random<BitVector, TwoLayerSelect<BitVector, false>, true>();
  test_select_random<BitVector1024, TwoLayerSelect<BitVector1024, false>,
                     true>();
}

TEST(TwoLayerSelectTestBinarySearch, Uniform) {
  using BitVector = TwoLayerRankCombinedBitVector<>;
  using BitVector1024 = TwoLayerRankCombinedBitVector<1024, 15>;

  test_select_uniform<BitVector, TwoLayerSelect<BitVector, true>>();
  test_select_uniform<BitVector1024, TwoLayerSelect<BitVector1024, true>>();
}
TEST(TwoLayerSelectTestBinarySearch, Alternating) {
  using BitVector = TwoLayerRankCombinedBitVector<>;
  using BitVector1024 = TwoLayerRankCombinedBitVector<1024, 15>;

  test_select_alternating<BitVector, TwoLayerSelect<BitVector, true>, true>();
  test_select_alternating<BitVector1024, TwoLayerSelect<BitVector1024, true>,
                          true>();
}

TEST(TwoLayerSelectTestBinarySearch, Random) {
  using BitVector = TwoLayerRankCombinedBitVector<>;
  using BitVector1024 = TwoLayerRankCombinedBitVector<1024, 15>;

  test_select_random<BitVector, TwoLayerSelect<BitVector, true>, true>();
  test_select_random<BitVector1024, TwoLayerSelect<BitVector1024, true>,
                     true>();
}

}  // namespace
