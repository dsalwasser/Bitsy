/// Query data structures.
/// @file query.hpp
/// @author Daniel Salwasser
#pragma once

#include <cstdint>

namespace bitsy {

/*!
 * The kind of a query.
 */
enum class QueryKind {
  //! Returns whether a bit is set.
  ACCESS,
  //! Returns the zero-rank of a bit.
  RANK0,
  //! Returns the one-rank of a bit.
  RANK1,
  //! Returns the position of the k-th zero.
  SELECT0,
  //! Returns the position of the k-th one.
  SELECT1,
};

/*!
 * A query on a bit vector with rank and select support.
 */
struct Query {
  //! The kind of the query.
  QueryKind kind;
  //! The position/rank of the query.
  std::uint64_t value;
};

}  // namespace bitsy
