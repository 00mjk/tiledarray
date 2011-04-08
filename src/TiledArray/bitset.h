#ifndef TILEDARRAY_BITSET_H__INCLUDED
#define TILEDARRAY_BITSET_H__INCLUDED

#include <TiledArray/error.h>
#include <boost/scoped_array.hpp>

namespace TiledArray {

  namespace detail {

    /// Fixed size bitset

    /// Bitset is similar to \c std::bitset except the size is set at runtime.
    /// This bit set has very limited functionality, because it is only intended
    /// to fit the needs of \c SparseShape.
    /// \tparam Block The type used to store the data [ default = \c unsigned \c long ]
    template <typename Block = unsigned long>
    class Bitset {
    public:
      typedef Block block_type;     ///< The type used to store the data
      typedef bool const_reference; ///< Constant reference to a bit

      /// Construct a bitset that contains \c s bits.

      /// \param s The number of bits
      /// \throw std::bad_alloc If bitset allocation fails.
      Bitset(std::size_t s) :
          size_(s),
          blocks_(s / sizeof(block_type) + (s % sizeof(block_type) ? 1 : 0)),
          set_(new block_type[blocks_])
      {
        std::fill_n(set_.get(), num_blocks(), block_type(0));
      }

      /// Copy constructor for bitset

      /// \param other The bitset to copy
      /// \throw std::bad_alloc If bitset allocation fails.
      Bitset(const Bitset<Block>& other) :
          size_(other.size_), blocks_(other.blocks_), set_(new block_type[other.blocks_])
      {
        std::copy(other.set_.get(), other.set_.get() + blocks_, set_.get());
      }

      /// Assignment operator

      /// This will only copy the data from \c other. It will not change the size
      /// of the bitset.
      /// \param other The bitset to copy
      /// \throw std::runtime_error If the bitset sizes are not equal.
      Bitset<Block>& operator=(const Bitset<Block>& other) {
        TA_ASSERT(size_ == other.size_, std::range_error, "The sizes must match.");
        if(this != &other)
          std::copy(other.set_.get(), other.set_.get() + blocks_, set_.get());

        return *this;
      }

      /// Or-assignment operator

      /// Or-assign all bits from the two ranges
      /// \param other The bitset to be or-assigned to this bitset
      /// \throw std::range_error If the bitset sizes are not equal.
      Bitset<Block>& operator|=(const Bitset<Block>& other) {
        TA_ASSERT(size_ == other.size_, std::range_error, "The sizes must match.");
        for(std::size_t i = 0; i < blocks_; ++i)
          set_[i] |= other.set_[i];

        return *this;
      }

      /// And-assignment operator

      /// And-assign all bits from the two ranges
      /// \param other The bitset to be and-assigned to this bitset
      /// \throw std::range_error If the bitset sizes are not equal.
      Bitset<Block>& operator&=(const Bitset<Block>& other) {
        TA_ASSERT(size_ == other.size_, std::range_error, "The sizes must match.");
        for(std::size_t i = 0; i < blocks_; ++i)
          set_[i] &= other.set_[i];

        return *this;
      }

      /// Bit accessor operator

      /// \param i The bit to access
      /// \return The value of i-th bit of the bit set
      /// \throw std::out_of_range If \c i is greater than or equal to the size
      const_reference operator[](std::size_t i) const {
        TA_ASSERT(i < size_, std::out_of_range, "Bit i is out of range.")
        return mask(i) & set_[block_index(i)];
      }

      /// Set a bit value

      /// \param i The bit to be set
      /// \param value The new value of the bit
      /// \throw nothing
      void set(std::size_t i, bool value = true) {
        if(value)
          set_[block_index(i)] |= mask(i);
        else
          set_[block_index(i)] &= ~mask(i);
      }

      /// Data pointer accessor

      /// The pointer to the data points to a contiguous block of memory of type
      /// \c block_type that contains \c num_blocks() elements.
      /// \return A pointer to the first element of the bitset data
      /// \throw nothing
      const block_type* get() const { return set_.get(); }


      /// Data pointer accessor

      /// The pointer to the data points to a contiguous block of memory of type
      /// \c block_type that contains \c num_blocks() elements.
      /// \return A pointer to the first element of the bitset data
      /// \throw nothing
      block_type* get() { return set_.get(); }

      /// Bitset size

      /// \return Number of bits in the bitset
      /// \throw nothing
      std::size_t size() const { return size_; }

      /// Bitset block size

      /// \return Number of block_type elements used to store the bitset array.
      /// \throw nothing
      std::size_t num_blocks() const { return blocks_; }

    private:

      static std::size_t block_index(std::size_t i) { return i / sizeof(block_type); }
      static std::size_t bit_index(std::size_t i) { return i % sizeof(block_type); }
      static block_type mask(std::size_t i) { return block_type(1) << bit_index(i); }

      std::size_t size_;
      std::size_t blocks_;
      boost::scoped_array<block_type> set_;
    }; // class Bitset

  } // namespace detail

} // namespace TiledArray

#endif // TILEDARRAY_BITSET_H__INCLUDED
