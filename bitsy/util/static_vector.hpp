/// A dynamically allocated vector of fixed size.
/// @file static_vector.hpp
/// @author Daniel Salwasser
#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>

#ifdef __linux__
#include <sys/mman.h>
#endif

#include "bitsy/util/math.hpp"

namespace bitsy {

/**
 * A vector of fixed size whose elements are stored on the heap.
 *
 * When Bitsy is compiled with the huge pages option enabled, we try to allocate
 * memory using huge pages and switch to normal pages if this fails.
 *
 * @tparam T The type of element to store.
 */
template <typename T>
class StaticVector {
#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#if defined(BITSY_HUGE_PAGES) && defined(__linux__)
  static constexpr bool kUseHugePages = true;
#else
  static constexpr bool kUseHugePages = false;
#endif

  // We use 2 MiB-sized huge pages.
  static constexpr std::size_t kHugePageSize = 1 << 21;

 public:
  //! The type of the value that is stored.
  using value_type = T;
  //! The type of the integer used to index.
  using size_type = std::size_t;
  //! The type of a pointer that points to a value.
  using pointer = T*;
  //! The type of a constant pointer that points to a value.
  using const_pointer = T*;
  //! The type of a reference that refers to a value.
  using reference = T&;
  //! The type of a constant reference that refers to a value.
  using const_reference = const T&;

  /**
   * Constructs an unitialized static vector.
   *
   * @param size The number of elements that this vector contains.
   */
  explicit StaticVector(const size_type size) : _size(size) {
    const std::size_t num_bytes = size * sizeof(T);

    if constexpr (kUseHugePages) {
      const std::size_t length = math::round_to(num_bytes, kHugePageSize);
      _ptr = static_cast<pointer>(mmap(
          nullptr, length, PROT_READ | PROT_WRITE,
          MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_2MB, -1, 0));
      if (_ptr != MAP_FAILED) {
        _huge_pages = true;
        return;
      }
    }

    _ptr = static_cast<pointer>(std::malloc(num_bytes));
    _huge_pages = false;
    if (_ptr == nullptr) {
      throw std::bad_alloc();
    }
  }

  /**
   * Destructs this vector and thereby releases the underlying memory that is
   * allocated on the heap.
   */
  ~StaticVector() {
    if constexpr (kUseHugePages) {
      if (_huge_pages) {
        const std::size_t num_bytes = _size * sizeof(T);
        const std::size_t length = math::round_to(num_bytes, kHugePageSize);
        munmap(_ptr, length);
        return;
      }
    }

    std::free(_ptr);
  }

  /**
   * Constructs this static vector by moving the data owned by another static
   * vector and thereby invalidating it.
   *
   * @param other The other static vectors whose data to take.
   */
  StaticVector(StaticVector&& other) noexcept
      : _huge_pages(other._huge_pages), _size(other._size), _ptr(other._ptr) {
    other._huge_pages = false;
    other._size = 0;
    other._ptr = nullptr;
  }

  /**
   * Transferring the data owned by another static vector to this static vector.
   *
   * @param other The other static vectors whose data to take.
   */
  StaticVector& operator=(StaticVector&& other) noexcept {
    if (this != &other) {
      _huge_pages = other._huge_pages;
      _size = other._size;
      _ptr = other._ptr;

      other._huge_pages = false;
      other._size = 0;
      other._ptr = nullptr;
    }

    return *this;
  }

  // Delete the copy constructor/copy assignment operator as we do not intend
  // to copy the static vector.
  StaticVector(StaticVector const&) = delete;
  StaticVector& operator=(StaticVector const&) = delete;

  /**
   * Returns a reference to an element stored in this vector.
   *
   * @param pos The position of the element whose reference is to be returned.
   * @return A reference to the element.
   */
  [[nodiscard]] reference operator[](const size_type pos) {
    return _ptr[pos];
  }

  /**
   * Returns a const reference to an element stored in this vector.
   *
   * @param pos The position of the element whose reference is to be returned.
   * @return A const reference to the element.
   */
  [[nodiscard]] const_reference operator[](const size_type pos) const {
    return _ptr[pos];
  }

  /**
   * Returns the number of elements that this vector stores.
   *
   * @return The number of elements.
   */
  [[nodiscard]] size_type size() const {
    return _size;
  }

  /**
   * Returns a pointer to the underlying memory at which the elements are
   * stored.
   *
   * @return A pointer to the underlying memory.
   */
  [[nodiscard]] pointer data() {
    return _ptr;
  }

  /**
   * Returns a const pointer to the underlying memory at which the elements are
   * stored.
   *
   * @return A const pointer to the underlying memory.
   */
  [[nodiscard]] const_pointer data() const {
    return _ptr;
  }

 private:
  bool _huge_pages;
  size_type _size;
  pointer _ptr;
};

}  // namespace bitsy
