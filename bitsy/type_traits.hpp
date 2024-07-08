/// Type traits for the bit vectors as well as the rank and select structures.
/// @file type_traits.hpp
/// @author Daniel Salwasser
#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace bitsy::type_traits {

//! Type trait that a bit vector fulfills.
template <typename T>
concept BitVector = requires(T a,
                             const std::size_t length,
                             const std::size_t pos,
                             const bool value) {
  //! Creates an uninitialized bit vector.
  T(length);
  //! Creates an bit vector with bits set to zero or one.
  T(length, value);
  //! Sets a bit to zero.
  a.unset(pos);
  //! Sets a bit to one.
  a.set(pos);
  //! Sets a bit either to zero or one.
  a.set(pos, value);
  //! Returns whether a bit is set.
  { a.is_set(pos) } -> std::same_as<bool>;
  //! Returns the length of the bit vector in bits.
  { a.length() } -> std::convertible_to<std::size_t>;
  //! Returns a pointer to the underlying memory.
  { a.data() } -> std::same_as<const std::uint64_t*>;
  //! Returns the used memory space of the bit vector in bits.
  { a.memory_space() } -> std::convertible_to<std::size_t>;
};

//! Type trait that a rank data structure fulfills.
template <typename T>
concept Rank = requires(T a, const std::size_t pos) {
  //! Updates the rank structure.
  a.update();
  //! For the associated bit vector, it returns the number of bits equal to zero
  //! up to the given position.
  { a.rank0(pos) } -> std::convertible_to<std::uint64_t>;
  //! For the associated bit vector, it returns the number of bits equal to one
  //! up to the given position.
  { a.rank1(pos) } -> std::convertible_to<std::uint64_t>;
  //! Returns the used memory space of the rank structure in bits.
  { a.memory_space() } -> std::convertible_to<std::size_t>;
};

//! Type trait that a rank-combined bit vector fulfills.
template <typename T>
concept RankCombinedBitVector = BitVector<T> && Rank<T>;

//! Type trait that a select data structure fulfills.
template <typename T>
concept Select = requires(T a, const std::size_t pos) {
  //! Updates the select structure.
  a.update();
  //! For the associated bit vector, it returns the position of the pos-th
  //! occurence of zero.
  { a.select0(pos) } -> std::convertible_to<std::uint64_t>;
  //! For the associated bit vector, it returns the position of the pos-th
  //! occurence of one.
  { a.select1(pos) } -> std::convertible_to<std::uint64_t>;
  //! Returns the used memory space of the select structure in bits.
  { a.memory_space() } -> std::convertible_to<std::size_t>;
};

}  // namespace bitsy::type_traits
