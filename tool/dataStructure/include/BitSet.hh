// -*-Mode: C++;-*-
/**
@file
@brief Defines and implements class BitSet.
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Tool

@author
Sephen Tether <tether@slac.stanford.edu>

@par Date created:
2010/03/24

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(TOOL_DATASTRUCTURE_BITSET_HH)
#define      TOOL_DATASTRUCTURE_BITSET_HH

#include <algorithm>

namespace tool {
  namespace dataStructure {

    /// @brief A classic powerset stored in an unsigned integer.
    ///
    /// std::bitset<> is a bit too general and, as its implementation
    /// is hidden, is hard to make a nice bit-queue with.
    class BitSet {
      unsigned _bits;

    public:
      /** @brief Specify the set members directly using an unsigned int. */
      explicit BitSet(unsigned value=0) : _bits(value) {}

      /// @brief Specify the set members via a range of bit positions.
      /// @param[in, out] begin An iterator which refers to an int in the range 0-31.
      /// @param[in, out] end   An iterator which refers to an int in the range 0-31.
      ///
      /// If dereferencing all the iterators in the range begin to end
      /// yields 2, 4, 6 then BitSet(begin, end) yields the same
      /// result as BitSet((1U<<2) | (1U<<4) | (1U<<6)).
      template<typename T>
      BitSet(T begin, T end) {// T is an iterator type.
	_bits = 0;
	for (T it(begin); it != end; ++it) {
	  int i(*it);
	  this |= i;
	}
      }

      /** @brief Yield the bitset represented as an unsigned int. */
      unsigned bits() const {return _bits;}

      /** @brief  Set a single bit. */
      void set(int i)                         {_bits |= (1 << i);}
      /** @brief Set all bits. */
      void set()                              {_bits = ~0U;}
      /** @brief  Clear one bit. */
      void reset(int i)                       {_bits &= ~(1 << i);}
      /** @brief  Clear all bits. */
      void reset()                            {_bits = 0;}
      /** @brief Is bit i set?. */
      bool has(int i) const                   {return _bits & (1U << i);}
      /** @brief Set union. */
      void operator|=(const BitSet &other)    {_bits |= other._bits;}
      /** @brief Set intersection. */
      void operator&=(const BitSet &other)    {_bits &= other._bits;}
      /** @brief Set difference. */
      void operator-=(const BitSet &other)    {_bits &= ~other._bits;}
      /** @brief  Symmetric set difference. */
      void operator^=(const BitSet &other)    {_bits ^= other._bits;}
      /** @brief Flip all the bits. */
      void complement()                       {_bits = ~_bits;}
      /** @brief Are all bits clear? */
      bool empty() const                      {return _bits == 0;}
      /// @brief Count the number of set bits.
      /// @note size() is relatively expensive., O(# of set bits) rather than O(1).
      int size()   const {
	unsigned x(_bits);
	int n(0);
	while (x != 0) {
	  // Get a value with a single 0 at the position of the rightmost
	  // bit set in x and use that to mask off the bit in x.
	  ++n;
	  x &= ~(x & -x);
	}
	return n;
      }
      /** @brief Exchange the members of this set with those of another. */
      void swap(BitSet &other) {std::swap(this->_bits, other._bits);}
      /** @brief Are all members of this set also members of another? */
      bool isSubsetOf(const BitSet &other) const {return (_bits & other._bits) == other._bits;}
      /** @brief Are all members of another set also members of this one? */
      bool isSupersetOf(const BitSet &other) const {return (_bits & other._bits) == _bits;}
      /** @brief Does this set have exactly the same members as another? */
      bool operator==(const BitSet &other) const {return _bits == other._bits;}
      /** @brief */
      bool operator!=(const BitSet &other) const {return _bits != other._bits;}
    };

    /** @brief Produce a new Bitset with the same members as b.complement(). */
    inline BitSet operator~(const BitSet &b) {BitSet nb(b); nb.complement(); return nb;}

    /** @brief Produce a new Bitset that's the union of a and b. */
    inline BitSet operator|(const BitSet &a, const BitSet &b) {
      BitSet r(a);
      r |= b;
      return r;
    }

    /** @brief Produce a new Bitset that's the intersection of a and b. */
    inline BitSet operator&(const BitSet &a, const BitSet &b) {
      BitSet r(a);
      r &= b;
      return r;
    }

    /** @brief Produce a new Bitset that's the symmetric difference of a and b. */
    inline BitSet operator^(const BitSet &a, const BitSet &b) {
      BitSet r(a);
      r ^= b;
      return r;
    }

    /** @brief Produce a new Bitset that's the difference of a and b. */
    inline BitSet operator-(const BitSet &a, const BitSet &b) {
      BitSet r(a);
      r -= b;
      return r;
    }

    /** @internal
        We want the following definitions to be duplicated when
        necessary so that the methods of class BitPq can be completely
        inlined.

        The following magic number is a de Bruijn sequence; if you
        arranged all its 32 bits in a circle then every possible
        sequence of five bits would appear once as a contiguous
        subsequence (sequences overlap). Since the number of symbols
        is 2 (0 and 1) and we want each possible sequence of five
        symbols this number is classified as B(2, 5). Not any member
        of B(2, 5) will do; the upper five bits must be zero. Using it
        as a multiplier gives us a perfect hash for the set of 32
        values with a single bit set.  See
        http://supertech.csail.mit.edu/papers/debruijn.pdf.
    */
    static const unsigned deBruijn = 0x077CB531U;

    /** @internal
        Multiplying 2^N by the de Bruijn sequence and shifting right by
        27 yields a unique integer in [0, 31] for each N which we must
        translate back to N.
    */
    static const int deBruijnUnscramble[32] =
      {
	0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
	31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
      };

    /// @brief Finds each "1" bit in a BitSet.
    ///
    /// Yields a set of bit position numbers in order from lowest to
    /// highest. A bit position occurs at most once in the sequence.
    ///
    /// Typical use: for (BitPq pq(BitSet); !pq.empty(); pq.pop())
    /// {... pq.top() ...}
    class BitPq {
      unsigned _bits;
      int _top;

    public:
      /** @brief Set the initial queue contents from a BitSet. */
      explicit BitPq(const BitSet &set): _bits(set.bits()), _top(-1) {newtop();}
      /** @brief Is the queue empty? */
      bool empty()         const {return _top == -1;}
      /** @brief Return the top-most (lowest) bit position in the queue. */
      int  top()           const {return _top;}
      /** @brief Put a bit position in is proper place in the queue (if not already there). */
      void push(int pos)         {_bits |= (1U << pos); newtop();}
      /** @brief Remove the top bit position. */
      void pop() {
	if (!empty()) {
	  _bits ^= (1U << _top);
	  newtop();
	}
      }
      /** @brief How many bit positions are in the queue? */
      int size() const {return BitSet(_bits).size();}

    private:
      /** @brief Find the lowest numbered bit position now present in the queue. */
      void newtop() {
	if (_bits != 0) {
	  // First we use the old (_bits & -_bits) trick to get a value
	  // with just one bit set at the lowest bit position set in _bits.
	  // Then we use our hash function to turn that into a bit position.
	  _top = ( deBruijnUnscramble[((_bits & -_bits) * deBruijn) >> 27] );
	}
	else {
	  _top = -1;
	}
      }
    };
  }; // dataStructure
}; // tool
#endif // TOOL_DATASTRUCTURE_BITSET_HH
