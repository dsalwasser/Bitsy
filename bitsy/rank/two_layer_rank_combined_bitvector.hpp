/// A bit vector with rank support which groups the bits into superblocks and
/// blocks and stores the rank-data for blocks interleaved with the bit-data.
/// @file two_layer_rank_combined_bitvector.hpp
/// @author Daniel Salwasser
#pragma once

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>

#include "bitsy/util/math.hpp"
#include "bitsy/util/static_vector.hpp"

namespace bitsy {

// clang-format off
/**
 * A bit vector with rank support which groups the bits into superblocks and
 * blocks and stores the rank-data for blocks interleaved with the bit-data.
 *
 * The bits of the bit vector are grouped into blocks of size \a BlockWidth -
 * \a BlockHeaderWidth. Furthermore, the blocks are grouped into superblocks of
 * size \a 2^BlockHeaderWidth. For each superblock, we store the number of ones
 * up to the start of the superblock separately from the bits and block-data.
 * Because we support bit vectors with length up to 2^64, we store a 64-bit
 * integer for each superblock. For each block, we store the number of ones up
 * to the start of the block interleaved with the bits of the bit vector:
 *
 * ----------------....---------------------....------...----------------....-----
 * | Header |      Bits     | Header |      Bits     |...| Header |      Bits    |
 * ----------------....---------------------....------...----------------....-----
 *  ^^^^^^^^ \a BlockHeaderWidth wide
 *           ^^^^^^^^^^^^^^^ \a BlockWidth - \a BlockHeaderWidth wide
 *
 * We store the block data interleaved with the bits to reduce the number of
 * cache misses, as this has the biggest impact on performance. With a block
 * width of 512 we get two cache misses, one for accessing the superblock data
 * and one for accessing the block, since on modern cpus the cache lines are 64
 * bytes in size.. We achive a space overhead of
 * BlockHeaderWidth / (BlockWidth - BlockHeaderWidth) + 64 / 2^BlockHeaderWidth
 * on top of the bit vector. For a block width of 512 and a header width of 14,
 * we get a space overhead of ~3.20% on top of the bit vector.
 *
 * @tparam BlockWidth The size of each block in bits.
 * @tparam BlockHeaderWidth The size of the block header in bits.
 */
// clang-format on
template <std::size_t BlockWidth = 512, std::size_t BlockHeaderWidth = 14>
class TwoLayerRankCombinedBitVector {
  static_assert(BlockWidth % 2 == 0, "Block width has to be a power of two.");
  static_assert(BlockWidth > 64, "Block width has to greater than 64 bits.");
  static_assert(BlockHeaderWidth <= 64,
                "Block header has to be a at most 64 bits wide.");
  static_assert(math::pow2(BlockHeaderWidth) > BlockWidth,
                "Superblock width has to be greater than the block width.");

  using BitVector = TwoLayerRankCombinedBitVector;

  using Word = std::uint64_t;
  static constexpr std::size_t kWordWidth = sizeof(Word) * 8;

 public:
  //! The width in bits of a block.
  static constexpr std::size_t kBlockWidth = BlockWidth;
  //! The width in bits of the header that is stored in the first word of a
  //! block.
  static constexpr std::size_t kBlockHeaderWidth = BlockHeaderWidth;
  //! The width in bits of the data that is stored in a block.
  static constexpr std::size_t kBlockDataWidth =
      kBlockWidth - kBlockHeaderWidth;
  //! The width in bits of the data this is stored in the first word of a block.
  static constexpr std::size_t kHeaderDataWidth =
      kWordWidth - kBlockHeaderWidth;
  //! The number of words per block.
  static constexpr std::size_t kNumWordsPerBlock = kBlockWidth / kWordWidth;

  //! The width in bits of a superblock.
  static constexpr std::size_t kSuperblockWidth = math::pow2(BlockHeaderWidth);
  //! The number of blocks per superblock.
  static constexpr std::size_t kNumBlocksPerSuperblock =
      kSuperblockWidth / kBlockWidth;
  //! The number of words per superblock.
  static constexpr std::size_t kNumWordsPerSuperblock =
      kSuperblockWidth / kWordWidth;
  //! The width in bits of the data that is stored in a superblock.
  static constexpr std::size_t kSuperblockDataWidth =
      kSuperblockWidth - kNumBlocksPerSuperblock * kBlockHeaderWidth;

  /**
   * Constructs an uninitialized bit vector.
   *
   * @param length The number of bits that this bit vector contains.
   */
  explicit TwoLayerRankCombinedBitVector(const std::size_t length)
      : _length(length),
        _num_blocks(math::div_ceil(length, kBlockDataWidth)),
        // We have to pad the data with (virtual) blocks, which allow us to do
        // an binary search (for select) without segfaulting or having to
        // consider an edge case.
        _data(_num_blocks * kNumWordsPerBlock +
              kNumBlocksPerSuperblock * kNumWordsPerBlock),
        _num_superblocks(math::div_ceil(length, kSuperblockDataWidth)),
        _superblock_data(_num_superblocks) {
    if (_num_blocks > 0) {
      // Fill the last bits with zeros such that the behaivour is predictable,
      // since this bits are nether set explicitly when the length is not a
      // multiple of the block-data width.
      Word* last_block = _data.data() + (_num_blocks - 1) * kNumWordsPerBlock;
      std::fill_n(last_block, kNumWordsPerBlock, 0);
    }
  }

  /**
   * Constructs a bit vector whose bits are all set to zero or one and
   * initializes the integrated rank structure.
   *
   * @param length The number of bits that this bit vector contains.
   * @param set Whether the bits are initially set to zero or one.
   */
  explicit TwoLayerRankCombinedBitVector(const std::size_t length,
                                         const bool set)
      : TwoLayerRankCombinedBitVector(length) {
    for (std::size_t pos = 0; pos < length; ++pos) {
      TwoLayerRankCombinedBitVector::set(pos, set);
    }

    update();
  }

  // Create the default destructor.
  ~TwoLayerRankCombinedBitVector() = default;

  // Create the default move constructor/move assignment operator.
  TwoLayerRankCombinedBitVector(BitVector&&) noexcept = default;
  TwoLayerRankCombinedBitVector& operator=(BitVector&&) noexcept = default;

  // Delete the copy constructor/copy assignment operator as we do not intend
  // to copy the bit vector.
  TwoLayerRankCombinedBitVector(BitVector const&) = delete;
  TwoLayerRankCombinedBitVector& operator=(BitVector const&) = delete;

  /**
   * Sets a bit within this bit vector to zero.
   *
   * @param pos The position of the bit that is to be set to zero.
   */
  inline void unset(const std::size_t pos) {
    const std::size_t num_block = pos / kBlockDataWidth;
    const std::size_t block_pos = pos % kBlockDataWidth + kBlockHeaderWidth;

    const std::size_t num_local_word = block_pos / kWordWidth;
    const std::size_t num_word = num_block * kNumWordsPerBlock + num_local_word;

    _data[num_word] &= ~(static_cast<Word>(1) << (block_pos % kWordWidth));
  }

  /**
   * Sets a bit within this bit vector to one.
   *
   * @param pos The position of the bit that is to be set to one.
   */
  inline void set(const std::size_t pos) {
    const std::size_t num_block = pos / kBlockDataWidth;
    const std::size_t block_pos = pos % kBlockDataWidth + kBlockHeaderWidth;

    const std::size_t num_local_word = block_pos / kWordWidth;
    const std::size_t num_word = num_block * kNumWordsPerBlock + num_local_word;

    _data[num_word] |= static_cast<Word>(1) << (block_pos % kWordWidth);
  }

  /**
   * Sets a bit within this bit vector depending on a boolean value.
   *
   * @param pos The position of the bit that is to be set to one.
   * @param value Whether to set the bit.
   */
  inline void set(const std::size_t pos, const bool value) {
    const std::size_t num_block = pos / kBlockDataWidth;
    const std::size_t block_pos = pos % kBlockDataWidth + kBlockHeaderWidth;

    const std::size_t num_local_word = block_pos / kWordWidth;
    const std::size_t num_word = num_block * kNumWordsPerBlock + num_local_word;

    // The following implementation is due to the following source:
    // https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
    const Word mask = static_cast<Word>(1) << (block_pos % kWordWidth);
    _data[num_word] = (_data[num_word] & ~mask) | (-value & mask);
  }

  /**
   * Returns whether a bit within this bit vector is set.
   *
   * @param pos The position of the bit that is to be queried.
   * @param value Whether the bit is set.
   */
  [[nodiscard]] inline bool is_set(const std::size_t pos) const {
    const std::size_t num_block = pos / kBlockDataWidth;
    const std::size_t block_pos = pos % kBlockDataWidth + kBlockHeaderWidth;

    const std::size_t num_local_word = block_pos / kWordWidth;
    const std::size_t num_word = num_block * kNumWordsPerBlock + num_local_word;

    const Word word = _data[num_word];
    const std::size_t word_pos = block_pos % kWordWidth;

    const bool is_set = ((word >> word_pos) & static_cast<Word>(1)) == 1;
    return is_set;
  }

  /**
   * Updates this rank data structure such that updates to the bit vector since
   * the initialization or the last update are reflected.
   */
  void update() {
    const Word* const data = _data.data();
    const std::size_t num_words = _num_blocks * kNumWordsPerBlock;

    // To update the rank information, we iterate over all blocks and count the
    // number of ones within a block. This generates more efficient code, since
    // in doing so we (somewhat) manually unroll the loop.
    Word cur_rank = 0;
    Word cur_block_rank = 0;
    std::size_t cur_num_super_block = 0;
    for (std::size_t i = 0; i < num_words; i += kNumWordsPerBlock) {
      const bool is_superblock_word = (i % kNumWordsPerSuperblock) == 0;

      if (is_superblock_word) [[unlikely]] {
        cur_rank += cur_block_rank;
        _superblock_data[cur_num_super_block] = cur_rank;

        cur_num_super_block += 1;
        cur_block_rank = 0;
      }

      _data[i] = (_data[i] &
                  math::setbits<Word>(kHeaderDataWidth, kBlockHeaderWidth)) |
                 cur_block_rank;
      cur_block_rank += block_popcount(data + i);
    }

    // Also fill the virtual blocks (which is just padding) so that a binary
    // search for a select query works correctly.
    for (std::size_t i = num_words; i < _data.size(); i += kNumWordsPerBlock) {
      const bool is_superblock_word = (i % kNumWordsPerSuperblock) == 0;

      if (is_superblock_word) {
        cur_block_rank = 0;
      }

      _data[i] = cur_block_rank;
    }
  }

  /**
   * Returns the number of bits equal to zero up to a position.
   *
   * @param pos The position up to which bits are to be taken into account.
   * @return The number of bits equal to zero up to the position.
   */
  [[nodiscard]] inline Word rank0(const std::size_t pos) const {
    // Query the one-rank and uses that to compute the zero-rank. This avoids
    // the additional memory that would be required to store the zero-rank
    // information and costs (basically) no running time.
    return static_cast<Word>(pos) - rank1(pos);
  }

  /**
   * Returns the number of bits equal to one up to a position.
   *
   * @param pos The position up to which bits are to be taken into account.
   * @return The number of bits equal to zero up to the position.
   */
  [[nodiscard]] inline Word rank1(const std::size_t pos) const {
    // Step 1: Compute the block and the word within the block in which the bit
    // is located as well as the position of the bit within the word. This
    // information is needed to access the data.
    const std::size_t num_block = pos / kBlockDataWidth;
    const std::size_t block_pos = pos % kBlockDataWidth + kBlockHeaderWidth;

    std::size_t num_word = block_pos / kWordWidth;
    const std::size_t word_pos = block_pos % kWordWidth;

    // Step 2: Compute the superblock in which the bit is located and fetch the
    // number of ones up to the start of the superblock, which we store
    // explicitly.
    const std::size_t num_superblock = pos / kSuperblockDataWidth;
    Word rank = _superblock_data[num_superblock];

    // Step 3: Fetch the number of ones up to the start of the block, which we
    // store in the first kBlockHeaderWidth bits of the block.
    const Word* const data = _data.data() + num_block * kNumWordsPerBlock;
    const Word first_word = *data;
    rank += first_word & math::setbits<Word>(kBlockHeaderWidth);

    if (num_word == 0) [[unlikely]] {
      // Step a4: If we are in the first word, count the number of ones up to
      // the bit. Note that we have to clear the data about the block-rank, as
      // it is also stored in the first word. Furthemore, we avoid a conditional
      // jump by using a conditional move.
      const std::size_t shift = (kWordWidth + kBlockHeaderWidth) - word_pos;
      rank += std::popcount((first_word >> kBlockHeaderWidth) << shift) *
              (word_pos != kBlockHeaderWidth);
    } else {
      // Step b4: Count all the number of ones within the first word since we
      // are in a higher word. Note that we have to clear the data about the
      // block-rank, as it is also stored in the first word.
      rank += std::popcount(first_word >> kBlockHeaderWidth);

      // Step b5: Count the number of ones within the words up to the second
      // last word.
      std::size_t i = 1;
      while (i < num_word) {
        rank += std::popcount(data[i++]);
      }

      // Step b6: Count the number of ones up to the bit. Here, we avoid a
      // conditional jump by using a conditional move.
      const std::size_t shift = kWordWidth - word_pos;
      rank += std::popcount(data[i] << shift) * (word_pos != 0);
    }

    return rank;
  }

  /**
   * Returns the number of ones within the data of a block.
   *
   * @param num_block The block for which the popcount is to be returned.
   * @return The popcount of the block.
   */
  [[nodiscard]] inline Word block_popcount(const std::size_t num_block) const {
    const Word* const data = _data.data() + num_block * kNumWordsPerBlock;
    return block_popcount(data);
  }

  /**
   * Returns the number of ones within the data of a block.
   *
   * @param data A pointer to the start of block for which the popcount is to be
   * returned.
   * @return The popcount of the block.
   */
  [[nodiscard]] inline static Word block_popcount(const Word* const data) {
    Word popcount = std::popcount(*data >> kBlockHeaderWidth);

    for (std::size_t i = 1; i < kNumWordsPerBlock; ++i) {
      popcount += std::popcount(data[i]);
    }

    return popcount;
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
   * Returns the number of superblocks.
   *
   * @return The number of superblocks.
   */
  [[nodiscard]] inline std::size_t num_superblocks() const {
    return _num_superblocks;
  }

  /**
   * Returns the number of blocks.
   *
   * @return The number of blocks.
   */
  [[nodiscard]] inline std::size_t num_blocks() const {
    return _num_blocks;
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
   * Returns a pointer to the underlying memory at which the ranks for the
   * superblocks are stored.
   *
   * @return A pointer to the underlying memory at which the ranks for the
   * superblocks are stored.
   */
  [[nodiscard]] inline const Word* superblock_data() const {
    return _superblock_data.data();
  }

  /**
   * Returns the used memory space of this data structure in bits.
   *
   * Note that it only accounts for the memory that is stored on the heap,
   * i.e., the memory that depends on the length of the bit vector.
   *
   * @return The used memory space of this data structure in bits.
   */
  [[nodiscard]] inline std::size_t memory_space() const {
    return _data.size() * kWordWidth + _superblock_data.size() * kWordWidth;
  }

 private:
  std::size_t _length;
  std::size_t _num_blocks;
  StaticVector<Word> _data;

  std::size_t _num_superblocks;
  StaticVector<Word> _superblock_data;
};

}  // namespace bitsy
