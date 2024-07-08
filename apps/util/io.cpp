#include "apps/util/io.hpp"

#include <fstream>
#include <iostream>

namespace bitsy {

std::pair<std::string, std::vector<Query>> read_input(
    const std::string& filename) {
  std::ifstream in(filename, std::ios::binary);

  std::uint64_t n;
  in >> n;

  std::string raw_bitvector;
  in >> raw_bitvector;

  std::vector<Query> queries;
  queries.reserve(n);

  while (n > 0) {
    std::string cmd;
    in >> cmd;

    if (cmd == "access") {
      std::uint64_t position;
      in >> position;

      queries.emplace_back(QueryKind::ACCESS, position);
    } else if (cmd == "rank") {
      bool bit;
      in >> bit;

      std::uint64_t position;
      in >> position;

      QueryKind kind = bit ? QueryKind::RANK1 : QueryKind::RANK0;
      queries.emplace_back(kind, position);
    } else if (cmd == "select") {
      bool bit;
      in >> bit;

      std::uint64_t position;
      in >> position;

      QueryKind kind = bit ? QueryKind::SELECT1 : QueryKind::SELECT0;
      queries.emplace_back(kind, position);
    }

    n -= 1;
  }

  return std::make_pair(std::move(raw_bitvector), std::move(queries));
}

void write_answers(const std::string& output_file,
                   const std::vector<std::uint64_t>& answers) {
  std::ofstream out(output_file, std::ios::binary);

  for (const std::uint64_t answer : answers) {
    out << answer << '\n';
  }
}

}  // namespace bitsy
