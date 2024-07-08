#include <nanobench.h>

#include <cstddef>
#include <random>
#include <vector>

#include <bitsy/bitvector.hpp>
#include <bitsy/rank/two_layer_rank_combined_bitvector.hpp>

namespace {

std::vector<std::size_t> create_random_positions(
    const std::size_t num_positions,
    const std::size_t max_pos,
    const std::size_t seed = 1) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<std::size_t> dist(0, max_pos - 1);

  std::vector<std::size_t> positions;
  positions.resize(num_positions);
  for (std::size_t i = 0; i < num_positions; ++i) {
    positions[i] = dist(rng);
  }

  return positions;
}

void fetch_positions(const std::vector<std::size_t>& positions) {
  for (const std::size_t pos : positions) {
    ankerl::nanobench::doNotOptimizeAway(pos);
  }
}

void bench_bitsy(ankerl::nanobench::Bench& bench,
                 const std::size_t length,
                 const std::vector<std::size_t>& positions) {
  const bitsy::BitVector bitvector(length, true);

  bench.run("bitsy", [&] {
    for (const std::size_t pos : positions) {
      ankerl::nanobench::doNotOptimizeAway(bitvector.is_set(pos));
    }
  });
}

void bench_bitsy_two_layer_combined(ankerl::nanobench::Bench& bench,
                                    const std::size_t length,
                                    const std::vector<std::size_t>& positions) {
  const bitsy::TwoLayerRankCombinedBitVector bitvector(length, true);

  bench.run("bitsy-two-layer-rank-combined-512", [&] {
    for (const std::size_t pos : positions) {
      ankerl::nanobench::doNotOptimizeAway(bitvector.is_set(pos));
    }
  });
}

void bench_bitsy_two_layer_combined1024(
    ankerl::nanobench::Bench& bench,
    const std::size_t length,
    const std::vector<std::size_t>& positions) {
  const bitsy::TwoLayerRankCombinedBitVector<1024, 15> bitvector(length, true);

  bench.run("bitsy-two-layer-rank-combined-1024", [&] {
    for (const std::size_t pos : positions) {
      ankerl::nanobench::doNotOptimizeAway(bitvector.is_set(pos));
    }
  });
}

}  // namespace

int main() {
  ankerl::nanobench::Bench b;
  b.title("Bitvector Access Query")
      .unit("access")
      .relative(true)
      .minEpochIterations(100);

  constexpr std::size_t length = 1LL << 30;
  constexpr std::size_t num_samples = 10000;
  const auto positions = create_random_positions(num_samples, length);

  fetch_positions(positions);
  bench_bitsy(b, length, positions);

  fetch_positions(positions);
  bench_bitsy_two_layer_combined(b, length, positions);

  fetch_positions(positions);
  bench_bitsy_two_layer_combined1024(b, length, positions);
}
