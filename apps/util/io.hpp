/// Functions to handle the apps's IO.
/// @file io.hpp
/// @author Daniel Salwasser
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "apps/util/query.hpp"

namespace bitsy {

/**
 * Parses a bit vector and queries that operate on that bit vector from an input
 * text file.
 *
 * The file should have with the following format:
 * 0)     <number of queries N>
 * 1)     <raw bit vector, e.g. "0100010...">
 * 2)     <query_1>
 * 3)     <query_2>
 * ...
 * N - 2) <query_N>
 *
 * Furthermore, each query should be of the form:
 * a) access <pos>
 * b) rank <0/1> <pos>
 * c) select <0/1> <rank>
 *
 * @param filename The name of the file to be parsed.
 * @return A pair consisting of the raw bit vector and a vector of the queries.
 */
[[nodiscard]] std::pair<std::string, std::vector<Query>> read_input(
    const std::string& filename);

/**
 * Writes answers to a text file, where each answer is written to a single line.
 *
 * @param filename The name of the file to be written to.
 * @param answers The answers to be written to the file.
 */
void write_answers(const std::string& filename,
                   const std::vector<std::uint64_t>& answers);

}  // namespace bitsy
