/// A naive rank implementation.
/// @file naive_rank.hpp
/// @author Daniel Salwasser
#pragma once

#include <cstddef>
#include <cstdint>

#include "bitsy/type_traits.hpp"
#include "bitsy/util/static_vector.hpp"

namespace bitsy {

/**
 * A naive rank data structure that stores for each bit in a bit vector its
 * rank.
 *
 * Because we support bit vectors with length up to 2^64, we store a 64-bit
 * integer for each bit. This results in a space overhead of 640% on top of
 * the bit vector.
 *
 * @tparam BitVector The type of bit vector to support.
 */
template <type_traits::BitVector BitVector>
class NaiveRank {
  using Word = std::uint64_t;
  static constexpr std::size_t kWordWidth = sizeof(Word) * 8;

 public:
  /**
   * Constructs and initializes a new rank data structure, which supports rank
   * queries for a given bit vector.
   *
   * Note that updates to the bit vector are only visible after a call to
   * update().
   *
   * @param bitvector The bit vector to support.
   */
  explicit NaiveRank(const BitVector& bitvector)
      : _bitvector(bitvector), _data(bitvector.length()) {
    update();
  }

  // Create the default destructor.
  ~NaiveRank() = default;

  // Create the default move constructor/move assignment operator.
  NaiveRank(NaiveRank&&) noexcept = default;
  NaiveRank& operator=(NaiveRank&&) noexcept = default;

  // Delete the copy constructor/copy assignment operator as we do not intend
  // to copy the data structure.
  NaiveRank(NaiveRank const&) = delete;
  NaiveRank& operator=(NaiveRank const&) = delete;

  /**
   * Updates this rank data structure such that updates to the associated bit
   * vector since the initialization or the last update are reflected here.
   */
  void update() {
    Word cur_rank = 0;

    for (std::size_t pos = 0; pos < _bitvector.length(); ++pos) {
      _data[pos] = cur_rank;
      cur_rank += static_cast<Word>(_bitvector.is_set(pos) ? 1 : 0);
    }
  }

  /**
   * Returns the number of bits equal to zero up to a position.
   *
   * @param pos The position up to which bits are to be taken into account.
   * @return The number of bits equal to zero up to the position.
   */
  [[nodiscard]] inline Word rank0(const std::size_t pos) const {
    return static_cast<Word>(pos) - rank1(pos);
  }

  /**
   * Returns the number of bits equal to one up to a position.
   *
   * @param pos The position up to which bits are to be taken into account.
   * @return The number of bits equal to zero up to the position.
   */
  [[nodiscard]] inline Word rank1(const std::size_t pos) const {
    return _data[pos];
  }

  /**
   * Returns the used memory space of this data structure in bits.
   *
   * Note that it only accounts for the memory that is stored on the heap, i.e.,
   * the memory that depends on the length of the associated bit vector.
   *
   * @return The used memory space of this data structure in bits.
   */
  [[nodiscard]] inline std::size_t memory_space() const {
    return _data.size() * kWordWidth;
  }

 private:
  const BitVector& _bitvector;
  StaticVector<Word> _data;
};

}  // namespace bitsy
