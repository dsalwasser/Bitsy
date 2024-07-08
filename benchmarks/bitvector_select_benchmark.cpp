#include <nanobench.h>

#include <cstddef>
#include <random>
#include <vector>

#include <bitsy/bitvector.hpp>
#include <bitsy/rank/two_layer_rank_combined_bitvector.hpp>
#include <bitsy/select/naive_select.hpp>
#include <bitsy/select/two_layer_select.hpp>

namespace {

std::vector<std::size_t> create_random_queries(const std::size_t num_queries,
                                               const std::size_t num_ones,
                                               const std::size_t seed = 1) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<std::size_t> dist(1, num_ones);

  std::vector<std::size_t> queries;
  queries.resize(num_queries);
  for (std::size_t i = 0; i < num_queries; ++i) {
    queries[i] = dist(rng);
  }

  return queries;
}

void fetch_queries(const std::vector<std::size_t>& queries) {
  for (const std::size_t query : queries) {
    ankerl::nanobench::doNotOptimizeAway(query);
  }
}

void bench_bitsy_naive(ankerl::nanobench::Bench& bench,
                       const std::size_t length,
                       const std::vector<std::size_t>& queries) {
  const bitsy::BitVector bitvector(length, true);
  const bitsy::NaiveSelect select(bitvector, length);

  bench.run("bitsy-naive-select", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(select.select1(query));
    }
  });
}

void bench_bitsy_two_layer_linear_search(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector bitvector(length, true);
  const bitsy::TwoLayerSelect<bitsy::TwoLayerRankCombinedBitVector<>, false>
      select(bitvector, length);

  bench.run("bitsy-two-layer (linear search)", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(select.select1(query));
    }
  });
}

void bench_bitsy_two_layer_binary_search(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector bitvector(length, true);
  const bitsy::TwoLayerSelect<bitsy::TwoLayerRankCombinedBitVector<>, true>
      select(bitvector, length);

  bench.run("bitsy-two-layer (binary search)", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(select.select1(query));
    }
  });
}

void bench_bitsy_two_layer_binary_search_8192(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector bitvector(length, true);
  const bitsy::TwoLayerSelect<bitsy::TwoLayerRankCombinedBitVector<>, true,
                              8192>
      select(bitvector, length);

  bench.run("bitsy-two-layer-8192 (binary search)", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(select.select1(query));
    }
  });
}

void bench_bitsy_two_layer_binary_search_16384(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector bitvector(length, true);
  const bitsy::TwoLayerSelect<bitsy::TwoLayerRankCombinedBitVector<>, true,
                              16384>
      select(bitvector, length);

  bench.run("bitsy-two-layer-16384 (binary search)", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(select.select1(query));
    }
  });
}

void bench_bitsy_two_layer_binary_search_65536(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector bitvector(length, true);
  const bitsy::TwoLayerSelect<bitsy::TwoLayerRankCombinedBitVector<>, true,
                              65536>
      select(bitvector, length);

  bench.run("bitsy-two-layer-65536 (binary search)", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(select.select1(query));
    }
  });
}

void bench_bitsy_two_layer_binary_search_131072(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector bitvector(length, true);
  const bitsy::TwoLayerSelect<bitsy::TwoLayerRankCombinedBitVector<>, true,
                              131072>
      select(bitvector, length);

  bench.run("bitsy-two-layer-131072 (binary search)", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(select.select1(query));
    }
  });
}

}  // namespace

int main() {
  ankerl::nanobench::Bench b;
  b.title("Bitvector Select Query")
      .unit("select1")
      .relative(true)
      .minEpochIterations(100);

  constexpr std::size_t length = 1LL << 30;
  constexpr std::size_t num_queries = 10000;
  const auto queries = create_random_queries(num_queries, length);

  fetch_queries(queries);
  bench_bitsy_naive(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_linear_search(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_binary_search(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_binary_search_8192(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_binary_search_16384(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_binary_search_65536(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_binary_search_131072(b, length, queries);
}
