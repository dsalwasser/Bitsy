/// A naive select implementation.
/// @file naive_select.hpp
/// @author Daniel Salwasser
#pragma once

#include <cstddef>
#include <cstdint>

#include "bitsy/type_traits.hpp"
#include "bitsy/util/static_vector.hpp"

namespace bitsy {

/**
 * A naive select data structure that stores the position of each occurrence of
 * a zero and one within a bit vector.
 *
 * Because we support bit vectors with length up to 2^64, we store a 64-bit
 * integer for each bit. This results in a space overhead of 640% on top of
 * the bit vector.
 *
 * @tparam BitVector The type of bit vector to support.
 */
template <type_traits::BitVector BitVector>
class NaiveSelect {
  using Word = std::uint64_t;
  static constexpr std::size_t kWordWidth = sizeof(Word) * 8;

 public:
  /**
   * Constructs and initializes a new select data structure, which supports
   * select queries for a specified bit vector.
   *
   * Note that updates to the bit vector are only visible after a call to
   * update().
   *
   * @param bitvector The bit vector to support.
   * @param num_ones The number of bits set to one in the bit vector.
   */
  explicit NaiveSelect(const BitVector& bitvector, const std::size_t num_ones)
      : _bitvector(bitvector),
        _zero_positions(bitvector.length() - num_ones),
        _one_positions(num_ones) {
    update();
  }

  // Create the default destructor.
  ~NaiveSelect() = default;

  // Create the default move constructor/move assignment operator.
  NaiveSelect(NaiveSelect&&) noexcept = default;
  NaiveSelect& operator=(NaiveSelect&&) noexcept = default;

  // Delete the copy constructor/copy assignment operator as we do not intend
  // to copy the data structure.
  NaiveSelect(NaiveSelect const&) = delete;
  NaiveSelect& operator=(NaiveSelect const&) = delete;

  /**
   * Updates this select data structure such that updates to the associated bit
   * vector since the initialization or the last update are reflected here.
   */
  void update() {
    const std::size_t length = _bitvector.length();

    std::size_t cur_one = 0;
    std::size_t cur_zero = 0;
    for (std::size_t pos = 0; pos < length; ++pos) {
      if (_bitvector.is_set(pos)) {
        _one_positions[cur_one++] = pos;
      } else {
        _zero_positions[cur_zero++] = pos;
      }
    }
  }

  /**
   * Returns the position of the rank-th occurence of zero.
   *
   * @param rank The rank of the first zero whose position is to be returned.
   * @return The position of the first zero with given rank.
   */
  [[nodiscard]] inline Word select0(const std::size_t rank) const {
    return _zero_positions[rank - 1];
  }

  /**
   * Returns the position of the rank-th occurence of one.
   *
   * @param rank The rank of the first one whose position is to be returned.
   * @return The position of the first one with given rank.
   */
  [[nodiscard]] inline Word select1(const std::size_t rank) const {
    return _one_positions[rank - 1];
  }

  /**
   * Returns the used memory space of this data structure in bits.
   *
   * Note that it only accounts for the memory that is stored on the heap, i.e.,
   * the memory that depends on the length of this bit vector.
   *
   * @return The used memory space of this data structure in bits.
   */
  [[nodiscard]] inline std::size_t memory_space() const {
    return _zero_positions.size() * kWordWidth +
           _one_positions.size() * kWordWidth;
  }

 private:
  const BitVector& _bitvector;
  StaticVector<Word> _zero_positions;
  StaticVector<Word> _one_positions;
};

}  // namespace bitsy
