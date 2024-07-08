#include <nanobench.h>

#include <cstddef>
#include <random>
#include <vector>

#include <bitsy/bitvector.hpp>
#include <bitsy/rank/naive_rank.hpp>
#include <bitsy/rank/two_layer_rank_combined_bitvector.hpp>

namespace {

std::vector<std::size_t> create_random_queries(const std::size_t num_queries,
                                               const std::size_t max_val,
                                               const std::size_t seed = 1) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<std::size_t> dist(0, max_val - 1);

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
  const bitsy::NaiveRank rank(bitvector);

  bench.run("bitsy-naive-rank", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(rank.rank1(query));
    }
  });
}

void bench_bitsy_two_layer_combined(ankerl::nanobench::Bench& bench,
                                    const std::size_t length,
                                    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector bitvector(length, true);

  bench.run("bitsy-two-layer-rank-combined-512", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(bitvector.rank1(query));
    }
  });
}

void bench_bitsy_two_layer_combined1024(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector<1024, 15> bitvector(length, true);

  bench.run("bitsy-two-layer-rank-combined-1024", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(bitvector.rank1(query));
    }
  });
}

void bench_bitsy_two_layer_combined1536(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector<1536, 16> bitvector(length, true);

  bench.run("bitsy-two-layer-rank-combined-1536", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(bitvector.rank1(query));
    }
  });
}

void bench_bitsy_two_layer_combined2048(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& queries) {
  const bitsy::TwoLayerRankCombinedBitVector<2048, 16> bitvector(length, true);

  bench.run("bitsy-two-layer-rank-combined-2048", [&] {
    for (const std::size_t query : queries) {
      ankerl::nanobench::doNotOptimizeAway(bitvector.rank1(query));
    }
  });
}

}  // namespace

int main() {
  ankerl::nanobench::Bench b;
  b.title("Bitvector Rank Query")
      .unit("rank1")
      .relative(true)
      .minEpochIterations(100);

  constexpr std::size_t length = 1LL << 30;
  constexpr std::size_t num_queries = 10000;
  const auto queries = create_random_queries(num_queries, length);

  fetch_queries(queries);
  bench_bitsy_naive(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_combined(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_combined1024(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_combined1536(b, length, queries);

  fetch_queries(queries);
  bench_bitsy_two_layer_combined2048(b, length, queries);
}
