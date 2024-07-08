/// A select data structure which samples the number of superblock every k-th
/// one and zero is located in.
/// @file two_layer_select.hpp
/// @author Daniel Salwasser
#pragma once

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>

#include "bitsy/select/word_select.hpp"
#include "bitsy/type_traits.hpp"
#include "bitsy/util/static_vector.hpp"

namespace bitsy {

/**
 * A select data structure which samples the number of superblock every k-th one
 * and zero is located in and requires the two-layer rank-combined bit vector to
 * work.
 *
 * The number of superblock of every k-th one and zero is stored explicitly and
 * separately from the bit vector. Because we support bit vectors with length up
 * to 2^64, we store a 64-bit integer for each sample. For a stride of 32768
 * (i.e., we sample every 32768-th one and zero), we get a space overhead of
 * ~0.20% on top of the bit vector.
 *
 * @tparam TwoLayerRankCombinedBitVector The rank-combined bit vector to
 * support.
 * @tparam UseBinarySearch Whether to use a binary search to find the
 * superblocks and blocks.
 * @tparam Stride The stride with which is sampled.
 */
template <type_traits::RankCombinedBitVector TwoLayerRankCombinedBitVector,
          bool UseBinarySearch = true,
          std::size_t Stride = 32768>
class TwoLayerSelect {
  static_assert(Stride % 2 == 0, "Stride has to be a power of two.");

  using BitVector = TwoLayerRankCombinedBitVector;

  using Word = std::uint64_t;
  static constexpr std::size_t kWordWidth = sizeof(Word) * 8;

  static constexpr std::size_t kBlockWidth = BitVector::BlockWidth;
  static constexpr std::size_t kBlockHeaderWidth = BitVector::kBlockHeaderWidth;
  static constexpr std::size_t kBlockDataWidth = BitVector::kBlockDataWidth;

  static constexpr std::size_t kNumWordsPerBlock = BitVector::kNumWordsPerBlock;
  static constexpr std::size_t kHeaderDataWidth = BitVector::kHeaderDataWidth;

  static constexpr std::size_t kSuperblockWidth = BitVector::kSuperblockWidth;
  static constexpr std::size_t kNumBlocksPerSuperblock =
      BitVector::kNumBlocksPerSuperblock;
  static constexpr std::size_t kNumWordsPerSuperblock =
      BitVector::kNumWordsPerSuperblock;
  static constexpr std::size_t kSuperblockDataWidth =
      BitVector::kSuperblockDataWidth;

  static constexpr std::size_t kStride = Stride;
  static constexpr bool kUseBinarySearch = UseBinarySearch;

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
  explicit TwoLayerSelect(const BitVector& bitvector,
                          const std::size_t num_ones)
      : _bitvector(bitvector),
        _zero_samples((bitvector.length() - num_ones) / kStride + 2),
        _one_samples(num_ones / kStride + 2) {
    update();
  }

  // Create the default destructor.
  ~TwoLayerSelect() = default;

  // Create the default move constructor/move assignment operator.
  TwoLayerSelect(TwoLayerSelect&&) noexcept = default;
  TwoLayerSelect& operator=(TwoLayerSelect&&) noexcept = default;

  // Delete the copy constructor/copy assignment operator as we do not intend
  // to copy the data structure.
  TwoLayerSelect(TwoLayerSelect const&) = delete;
  TwoLayerSelect& operator=(TwoLayerSelect const&) = delete;

  /**
   * Updates this select data structure such that updates to the associated bit
   * vector since the initialization or the last update are reflected here.
   */
  void update() {
    if (_bitvector.length() == 0) {
      return;
    }

    std::size_t cur_one = 0;
    std::size_t cur_zero = 0;

    std::size_t total_ones = 0;
    std::size_t total_zeros = 0;

    std::size_t threshold_one = 0;
    std::size_t threshold_zero = 0;
    const auto handle_block = [&](const std::size_t num_block,
                                  const std::size_t num_ones,
                                  const std::size_t num_zeros) {
      total_ones += num_ones;
      total_zeros += num_zeros;

      if (total_ones >= threshold_one) [[unlikely]] {
        const std::size_t num_superblock =
            (num_block * kBlockDataWidth) / kSuperblockDataWidth;
        _one_samples[cur_one] = num_superblock;

        cur_one += 1;
        threshold_one += kStride;
      }

      if (total_zeros >= threshold_zero) [[unlikely]] {
        const std::size_t num_superblock =
            (num_block * kBlockDataWidth) / kSuperblockDataWidth;
        _zero_samples[cur_zero] = num_superblock;

        cur_zero += 1;
        threshold_zero += kStride;
      }
    };

    for (std::size_t num_block = 0; num_block < _bitvector.num_blocks() - 1;
         ++num_block) {
      const std::size_t num_ones = _bitvector.block_popcount(num_block);
      const std::size_t num_zeros = kBlockDataWidth - num_ones;
      handle_block(num_block, num_ones, num_zeros);
    }

    // Treat the last block specially, as the number of zeros cannot be computed
    // in the same way as for the other blocks due to padding.
    const std::size_t num_last_block = _bitvector.num_blocks() - 1;
    const std::size_t wrong_zeros =
        _bitvector.num_blocks() * kBlockDataWidth - _bitvector.length();
    const std::size_t num_ones = _bitvector.block_popcount(num_last_block);
    const std::size_t num_zeros = kBlockDataWidth - num_ones - wrong_zeros;
    handle_block(num_last_block, num_ones, num_zeros);

    // Store one more sample so that the "next superblock" can be retrieved for
    // a bit in the last superblock without considering a special case.
    _one_samples[cur_one] = _bitvector.num_superblocks() - 1;
    _zero_samples[cur_zero] = _bitvector.num_superblocks() - 1;
  }

  /**
   * Returns the position of the rank-th occurence of zero.
   *
   * @param rank The rank of the first zero whose position is to be returned.
   * @return The position of the first zero with given rank.
   */
  [[nodiscard]] inline Word select0(std::size_t rank) const {
    // Step 1: Fetch the range of superblocks containing the position we are
    // looking for using the explicitly stored samples.
    const std::size_t nearest_prev_sample = (rank - 1) / kStride;

    Word num_superblock = _zero_samples[nearest_prev_sample];
    const Word num_last_superblock = _zero_samples[nearest_prev_sample + 1];

    // Step 2: Find the superblock containing the position we are looking for
    // using either a binary search or linear search.
    const Word* superblock_data = _bitvector.superblock_data();
    const auto superblock_rank = [&superblock_data](const Word num_superblock) {
      return num_superblock * kSuperblockDataWidth -
             superblock_data[num_superblock];
    };

    if constexpr (kUseBinarySearch) {
      Word length = num_last_superblock - num_superblock + 1;
      while (length > 1) {
        const Word half = length / 2;
        length -= half;

        // Prefetch both the smaller and larger rank of the next possible
        // superblock, one of which will be used for comparison in the next
        // iteration. As we use a conditional move below, the branch predictor
        // can't speculative execute and therefore can't speculative fetch.
        // Therefore, we explicitly prefetch.
        __builtin_prefetch(&superblock_data[num_superblock + length / 2]);
        __builtin_prefetch(
            &superblock_data[num_superblock + length / 2 + half]);

        // Remove the conditional branch by using a conditional move.
        num_superblock +=
            (superblock_rank(num_superblock + half) < rank) * half;
      }
    } else {
      while (num_superblock < num_last_superblock &&
             superblock_rank(num_superblock + 1) < rank) {
        num_superblock += 1;
      }
    }

    rank -= superblock_rank(num_superblock);

    // Step 3: Find the block within the superblock containing the position we
    // are looking for using either a binary search or linear search.
    Word num_block = num_superblock * kNumBlocksPerSuperblock;
    const Word num_last_block =
        std::min(_bitvector.num_blocks(),
                 (num_superblock + 1) * kNumBlocksPerSuperblock) -
        1;

    const Word* data = _bitvector.data();
    const auto block_rank = [&data](const std::size_t num_block) {
      const Word header_word = data[num_block * kNumWordsPerBlock];
      const Word block_rank =
          header_word & math::setbits<Word>(kBlockHeaderWidth);
      return (num_block % kNumBlocksPerSuperblock) * kBlockDataWidth -
             block_rank;
    };

    if constexpr (kUseBinarySearch) {
      Word length = kNumBlocksPerSuperblock;
      while (length > 1) {
        const Word half = length / 2;
        length -= half;

        // Use the same binary search implementation as above.
        __builtin_prefetch(&data[num_block + length / 2]);
        __builtin_prefetch(&data[num_block + length / 2 + half]);
        num_block += (block_rank(num_block + half) < rank) * half;
      }

      rank -= block_rank(num_block);
      data += num_block * kNumWordsPerBlock;
    } else {
      while (num_block < num_last_block && block_rank(num_block + 1) < rank) {
        num_block += 1;
      }

      rank -= block_rank(num_block);
      data += num_block * kNumWordsPerBlock;
    }

    // Step 4: Find the word within the block containing the position we
    // are looking using a linear search.
    Word num_word = 0;
    const auto current_word_rank = [&data, &num_word]() {
      const Word word = *data;

      // Don't use a conditional move since this conditional jump is perfectly
      // predictable.
      if (num_word == 0) [[unlikely]] {
        return std::popcount(~(word | math::setbits<Word>(kBlockHeaderWidth)));
      } else {
        return std::popcount(~word);
      }
    };

    Word word_rank;
    while ((word_rank = current_word_rank()) < rank) {
      num_word += 1;
      data += 1;
      rank -= word_rank;
    }

    // We might have to clear the data about the block-rank, as it is stored in
    // the first word. Here, we avoid a conditional jump by using a conditional
    // move.
    Word word = (num_word == 0)
                    ? (*data | math::setbits<Word>(kBlockHeaderWidth))
                    : *data;

    // Step 5: Return the total position based on the superblock, block and
    // word we found above.
    return num_block * kBlockDataWidth + num_word * kWordWidth +
           word_select1(~word, rank) - kBlockHeaderWidth;
  }

  /**
   * Returns the position of the rank-th occurence of one.
   *
   * @param rank The rank of the first one whose position is to be returned.
   * @return The position of the first one with given rank.
   */
  [[nodiscard]] inline Word select1(std::size_t rank) const {
    // Step 1: Fetch the range of superblocks containing the position we are
    // looking for using the explicitly stored samples.
    const std::size_t nearest_prev_sample = (rank - 1) / kStride;

    Word num_superblock = _one_samples[nearest_prev_sample];
    const Word num_last_superblock = _one_samples[nearest_prev_sample + 1];

    // Step 2: Find the superblock containing the position we are looking for
    // using either a binary search or linear search.
    const Word* superblock_data = _bitvector.superblock_data();
    if constexpr (kUseBinarySearch) {
      Word length = num_last_superblock - num_superblock + 1;
      while (length > 1) {
        const Word half = length / 2;
        length -= half;

        // Prefetch both the smaller and larger rank of the next possible
        // superblock, one of which will be used for comparison in the next
        // iteration. As we use a conditional move below, the branch predictor
        // can't speculative execute and therefore can't speculative fetch.
        // Therefore, we explicitly prefetch.
        __builtin_prefetch(&superblock_data[num_superblock + length / 2]);
        __builtin_prefetch(
            &superblock_data[num_superblock + length / 2 + half]);

        // Remove the conditional branch by using a conditional move.
        num_superblock +=
            (superblock_data[num_superblock + half] < rank) * half;
      }
    } else {
      while (num_superblock < num_last_superblock &&
             superblock_data[num_superblock + 1] < rank) {
        num_superblock += 1;
      }
    }

    rank -= superblock_data[num_superblock];

    // Step 3: Find the block within the superblock containing the position we
    // are looking for using either a binary search or linear search.
    Word num_block = num_superblock * kNumBlocksPerSuperblock;
    const Word num_last_block =
        std::min(_bitvector.num_blocks(),
                 (num_superblock + 1) * kNumBlocksPerSuperblock) -
        1;

    const Word* data = _bitvector.data();
    const auto block_rank = [&](const std::size_t offset) {
      const Word word = data[offset * kNumWordsPerBlock];
      const Word word_rank = word & math::setbits<Word>(kBlockHeaderWidth);
      return word_rank;
    };

    if constexpr (kUseBinarySearch) {
      Word length = kNumBlocksPerSuperblock;
      while (length > 1) {
        const Word half = length / 2;
        length -= half;

        // Use the same binary search implementation as above.
        __builtin_prefetch(&data[num_block + length / 2]);
        __builtin_prefetch(&data[num_block + length / 2 + half]);
        num_block += (block_rank(num_block + half) < rank) * half;
      }

      rank -= block_rank(num_block);
      data += num_block * kNumWordsPerBlock;
    } else {
      while (num_block < num_last_block && block_rank(num_block + 1) < rank) {
        num_block += 1;
      }

      rank -= block_rank(num_block);
      data += num_block * kNumWordsPerBlock;
    }

    // Step 4: Find the word within the block containing the position we
    // are looking using a linear search.
    Word num_word = 0;
    const auto current_word_rank = [&]() -> Word {
      const Word word = *data;

      // Don't use a conditional move since this conditional jump is perfectly
      // predictable.
      if (num_word == 0) [[unlikely]] {
        return std::popcount(word >> kBlockHeaderWidth);
      } else {
        return std::popcount(word);
      }
    };

    Word word_rank;
    while ((word_rank = current_word_rank()) < rank) {
      num_word += 1;
      data += 1;
      rank -= word_rank;
    }

    // We might have to clear the data about the block-rank, as it is stored in
    // the first word. Here, we avoid a conditional jump by using a conditional
    // move.
    Word word = (num_word == 0)
                    ? (*data & ~math::setbits<Word>(kBlockHeaderWidth))
                    : *data;

    // Step 5: Return the total position based on the superblock, block and word
    // we found above.
    return num_block * kBlockDataWidth + num_word * kWordWidth +
           word_select1(word, rank) - kBlockHeaderWidth;
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
    return _zero_samples.size() * kWordWidth + _one_samples.size() * kWordWidth;
  }

 private:
  const BitVector& _bitvector;
  StaticVector<Word> _zero_samples;
  StaticVector<Word> _one_samples;
};

}  // namespace bitsy
