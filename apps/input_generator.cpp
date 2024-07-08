#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>

#include "apps/util/query.hpp"

namespace {
using namespace bitsy;

std::uint64_t generate_bitvector(std::ofstream& out,
                                 const std::uint64_t seed,
                                 const std::uint64_t length,
                                 const double fill_ratio) {
  std::mt19937 gen(seed);
  std::bernoulli_distribution dist(fill_ratio);

  std::uint64_t num_ones = 0;
  for (std::size_t i = 0; i < length; ++i) {
    const bool is_set = dist(gen);
    num_ones += is_set ? 1 : 0;
    out << (is_set ? '1' : '0');
  }

  return num_ones;
}

void generate_queries(std::ofstream& out,
                      const std::uint64_t seed,
                      const std::uint64_t num_queries,
                      const std::uint64_t length,
                      const std::uint64_t num_ones) {
  const std::uint64_t num_zeros = length - num_ones;

  std::mt19937 gen(seed);
  std::uniform_int_distribution<std::uint64_t> query_kind_dist(0, 4);
  std::uniform_int_distribution<std::uint64_t> position_dist(0, length - 1);
  std::uniform_int_distribution<std::uint64_t> select0_dist(1, num_zeros);
  std::uniform_int_distribution<std::uint64_t> select1_dist(1, num_ones);

  for (std::size_t i = 0; i < num_queries; ++i) {
    QueryKind query_kind = static_cast<QueryKind>(query_kind_dist(gen));

    switch (query_kind) {
      case QueryKind::ACCESS:
        out << "\naccess " << position_dist(gen);
        break;
      case QueryKind::RANK0:
        out << "\nrank 0 " << position_dist(gen);
        break;
      case QueryKind::RANK1:
        out << "\nrank 1 " << position_dist(gen);
        break;
      case QueryKind::SELECT0:
        out << "\nselect 0 " << select0_dist(gen);
        break;
      case QueryKind::SELECT1:
        out << "\nselect 1 " << select1_dist(gen);
        break;
    }
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 6) {
    std::cout << "Usage: " << argv[0]
              << " <seed> <length> <fill_ratio> <num_queries> <output_file>"
              << std::endl;
    std::exit(EXIT_FAILURE);
  }

  const std::uint64_t seed = std::strtol(argv[1], nullptr, 10);
  const std::uint64_t length = std::strtol(argv[2], nullptr, 10);
  const double fill_ratio = std::strtod(argv[3], nullptr);
  const std::uint64_t num_queries = std::strtol(argv[4], nullptr, 10);

  const char* output_file = argv[5];
  std::ofstream out(output_file, std::ios::binary);

  out << num_queries << '\n';
  const std::uint64_t num_ones =
      generate_bitvector(out, seed, length, fill_ratio);
  generate_queries(out, seed, num_queries, length, num_ones);

  return EXIT_SUCCESS;
}
