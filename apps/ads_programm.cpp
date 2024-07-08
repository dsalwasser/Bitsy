#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <bitsy/rank/two_layer_rank_combined_bitvector.hpp>
#include <bitsy/select/two_layer_select.hpp>

#include "apps/util/io.hpp"
#include "apps/util/query.hpp"
#include "apps/util/timer.hpp"

int main(int argc, char* argv[]) {
  using namespace bitsy;

  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input_file> <output_file>"
              << std::endl;
    std::exit(EXIT_FAILURE);
  }

  const char* input_file = argv[1];
  const char* output_file = argv[2];

  const auto [raw_bitvector, queries] = read_input(input_file);
  const std::size_t length = raw_bitvector.size();

  std::size_t num_ones = 0;
  TwoLayerRankCombinedBitVector bitvector(length);
  for (std::size_t pos = 0; pos < length; ++pos) {
    const bool is_set = raw_bitvector[pos] == '1';
    num_ones += is_set ? 1 : 0;
    bitvector.set(pos, is_set);
  }

  const std::size_t num_queries = queries.size();
  std::vector<std::uint64_t> answers(num_queries);

  std::size_t memory_space = bitvector.memory_space();
  const std::size_t milliseconds = time_function([&] {
    // Initialize the rank data structure, which is integrated into the bit
    // vector.
    bitvector.update();

    // Initialize the select data structure.
    TwoLayerSelect select(bitvector, num_ones);
    memory_space += select.memory_space();

    // Answer the queries using the initialized data structures.
    for (std::size_t i = 0; i < num_queries; ++i) {
      const auto [kind, value] = queries[i];

      switch (kind) {
        case QueryKind::ACCESS:
          answers[i] = bitvector.is_set(value);
          break;
        case QueryKind::RANK0:
          answers[i] = bitvector.rank0(value);
          break;
        case QueryKind::RANK1:
          answers[i] = bitvector.rank1(value);
          break;
        case QueryKind::SELECT0:
          answers[i] = select.select0(value);
          break;
        case QueryKind::SELECT1:
          answers[i] = select.select1(value);
          break;
      }
    }
  });

  std::cout << "RESULT name=daniel_salwasser time=" << milliseconds
            << " space=" << memory_space << std::endl;
  write_answers(output_file, answers);

  return EXIT_SUCCESS;
}
