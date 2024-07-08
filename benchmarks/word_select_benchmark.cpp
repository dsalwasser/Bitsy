#include <nanobench.h>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <ranges>
#include <vector>

#include <bitsy/select/word_select.hpp>

namespace {

std::vector<std::pair<std::uint64_t, std::size_t>> create_benchmark_set(
    const std::size_t size,
    const std::size_t seed = 1) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<std::uint64_t> word_dist(
      1, std::numeric_limits<std::uint64_t>::max());

  std::vector<std::uniform_int_distribution<std::size_t>> rank_dists;
  rank_dists.reserve(64);
  for (const std::size_t i : std::views::iota(0, 63)) {
    rank_dists.emplace_back(0, i);
  }

  std::vector<std::pair<std::uint64_t, std::size_t>> benchmark_set;
  benchmark_set.resize(size);
  for (std::size_t i = 0; i < size; ++i) {
    const std::uint64_t word = word_dist(rng);
    const std::size_t rank = rank_dists[std::popcount(word) - 1](rng);
    benchmark_set[i] = std::make_pair(word, rank);
  }

  return benchmark_set;
}

void prefetch(const auto& benchmark_set) {
  for (const auto [word, rank] : benchmark_set) {
    ankerl::nanobench::doNotOptimizeAway(word);
    ankerl::nanobench::doNotOptimizeAway(rank);
  }
}

template <bool kUseBinarySearch, bool kOverwriteFastPDEP>
void bench_config(ankerl::nanobench::Bench& bench,
                  const char* benchmark_name,
                  const auto& benchmark_set) {
  using namespace bitsy;

  prefetch(benchmark_set);
  bench.run(benchmark_name, [&] {
    for (const auto [word, rank] : benchmark_set) {
      ankerl::nanobench::doNotOptimizeAway(
          word_select1<kUseBinarySearch, kOverwriteFastPDEP>(word, rank));
    }
  });
}

}  // namespace

int main() {
  ankerl::nanobench::Bench b;
  b.title("Word Select")
      .unit("word_select")
      .relative(true)
      .minEpochIterations(100);

  constexpr std::size_t kBenchmarkSetSize = 100000;
  const auto benchmark_set = create_benchmark_set(kBenchmarkSetSize);

  bench_config<false, false>(b, "pdep", benchmark_set);
  bench_config<false, true>(b, "linear-search", benchmark_set);
  bench_config<true, true>(b, "binary-search", benchmark_set);
}
