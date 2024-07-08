/// A simple bit vector implementation.
/// @file bitvector.hpp
/// @author Daniel Salwasser
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "bitsy/util/math.hpp"
#include "bitsy/util/static_vector.hpp"

namespace bitsy {

/**
 * A bit vector that provides is-set, set and unset operations.
 *
 * The individual bits are stored in 64-bit words, whereby the first logical bit
 * within a block is stored at the least signficant position to answer an is-set
 * query slightly faster. Furthermore, because the bits are stored in 64 words,
 * at most 63 bits are wasted due to internal fragmentation.
 */
class BitVector {
 public:
  //! The type of integer that is used to store the bits.
  using Word = std::uint64_t;

  //! The number of bits in a word.
  static constexpr std::size_t kWordWidth = sizeof(Word) * 8;

  /**
   * Constructs an uninitialized bit vector
   *
   * @param length The number of bits that this bit vector contains.
   */
  explicit BitVector(const std::size_t length)
      : _length(length),
        _num_words(math::div_ceil(length, kWordWidth)),
        _data(_num_words) {
  }

  /**
   * Constructs a bit vector whose bits are all set to zero or one.
   *
   * @param length The number of bits that this bit vector contains.
   * @param set Whether the bits are initially set to zero or one.
   */
  explicit BitVector(const std::size_t length, const bool set)
      : BitVector(length) {
    const Word default_word = math::setbits<Word>(set ? kWordWidth : 0);
    std::fill_n(_data.data(), _num_words, default_word);
  }

  // Create the default destructor.
  ~BitVector() = default;

  // Create the default move constructor/move assignment operator.
  BitVector(BitVector&&) noexcept = default;
  BitVector& operator=(BitVector&&) noexcept = default;

  // Delete the copy constructor/copy assignment operator as we do not intend
  // to copy the bit vector.
  BitVector(BitVector const&) = delete;
  BitVector& operator=(BitVector const&) = delete;

  /**
   * Sets a bit within this bit vector to zero.
   *
   * @param pos The position of the bit that is to be set to zero.
   */
  inline void unset(const std::size_t pos) {
    _data[pos / kWordWidth] &= ~(static_cast<Word>(1) << (pos % kWordWidth));
  }

  /**
   * Sets a bit within this bit vector to one.
   *
   * @param pos The position of the bit that is to be set to one.
   */
  inline void set(const std::size_t pos) {
    _data[pos / kWordWidth] |= (static_cast<Word>(1) << (pos % kWordWidth));
  }

  /**
   * Sets a bit within this bit vector depending on a boolean value.
   *
   * @param pos The position of the bit that is to be set to one.
   * @param value Whether to set the bit.
   */
  inline void set(const std::size_t pos, const bool value) {
    const std::size_t num_word = pos / kWordWidth;

    // The following implementation is due to the following source:
    // https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
    const Word mask = static_cast<Word>(1) << (pos % kWordWidth);
    _data[num_word] = (_data[num_word] & ~mask) | (-value & mask);
  }

  /**
   * Returns whether a bit within this bit vector is set.
   *
   * @param pos The position of the bit that is to be queried.
   * @param value Whether the bit is set.
   */
  [[nodiscard]] inline bool is_set(const std::size_t pos) const {
    const Word word = _data[pos / kWordWidth];
    return ((word >> (pos % kWordWidth)) & static_cast<Word>(1)) == 1;
  }

  /**
   * Returns the number of bits that this bit vector contains.
   *
   * @return The number of bits that this bit vector contains.
   */
  [[nodiscard]] inline std::size_t length() const {
    return _length;
  }

  /**
   * Returns a pointer to the underlying memory at which the bits are stored.
   *
   * @return A pointer to the underlying memory at which the bits are stored.
   */
  [[nodiscard]] inline const Word* data() const {
    return _data.data();
  }

  /**
   * Returns the used memory space of this bit vector in bits.
   *
   * Note that it only accounts for the memory that is stored on the heap, i.e.,
   * the memory that depends on the length of this bit vector.
   *
   * @return The used memory space of this bit vector in bits.
   */
  [[nodiscard]] inline std::size_t memory_space() const {
    return _data.size() * kWordWidth;
  }

 private:
  std::size_t _length;
  std::size_t _num_words;
  StaticVector<Word> _data;
};

};  // namespace bitsy
