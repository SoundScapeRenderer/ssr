/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the Audio Processing Framework (APF).                 *
 *                                                                            *
 * The APF is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The APF is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 *                                 http://AudioProcessingFramework.github.com *
 ******************************************************************************/

/// @file
/// Block-based delay line.

#ifndef APF_BLOCKDELAYLINE_H
#define APF_BLOCKDELAYLINE_H

#include <algorithm>  // for std::max()
#include <vector>  // default container

#include "apf/iterator.h"  // for circular_iterator, stride_iterator

namespace apf
{

/** Block-based delay line.
 * This is a "write once, read many times" delay line.
 * The write operation is simple and fast.
 * The desired delay is specified at the more flexible read operation.
 **/
template<typename T, typename Container = std::vector<T>>
class BlockDelayLine
{
  public:
    using size_type = typename Container::size_type;
    using pointer = typename Container::pointer;
    using circulator = apf::circular_iterator<typename Container::iterator>;

    BlockDelayLine(size_type block_size, size_type max_delay);

    /// Return @b true if @p delay is valid
    bool delay_is_valid(size_type delay) const
    {
      return delay <= _max_delay;
    }

    /// Advance the internal iterators/pointers to the next block.
    void advance()
    {
      ++_block_circulator;
      _data_circulator += _block_size;
    }

    template<typename Iterator>
    void write_block(Iterator source);

    template<typename Iterator>
    bool read_block(Iterator destination, size_type delay) const;

    template<typename Iterator>
    bool read_block(Iterator destination, size_type delay, T weight) const;

    pointer get_write_pointer() const;

    circulator get_read_circulator(size_type delay = 0) const;

  protected:
    /// Get a circular iterator to the sample with time 0
    circulator _get_data_circulator() const { return _data_circulator; }

    const size_type _block_size;  ///< Size of read/write blocks

  private:
    const size_type _max_delay;  ///< Maximum delay

    const size_type _number_of_blocks; ///< No\. of blocks needed for storage

    Container _data;  ///< Internal storage for sample data

    /// Circular iterator which iterates over each sample
    circulator _data_circulator;

    /// Circular iterator iterating over the block-beginnings.
    apf::stride_iterator<circulator> _block_circulator;
};

/** Constructor.
 * @param block_size Block size
 * @param max_delay Maximum delay in samples
 **/
template<typename T, typename Container>
BlockDelayLine<T, Container>::BlockDelayLine(size_type block_size
    , size_type max_delay)
  : _block_size(block_size)
  , _max_delay(max_delay)
  // Minimum number of blocks is 2, even if _max_delay is 0.
  // With only one block the circular iterators r and (r + _block_size) would be
  // equal and the read...() functions wouldn't work.
  // But anyway, who wants a delay line with no delay? Kind of useless ...
  , _number_of_blocks(
      std::max(size_type(2), (_max_delay + 2 * _block_size - 1) / _block_size))
  , _data(_number_of_blocks * _block_size)  // initialized with default ctor T()
  , _data_circulator(_data.begin(), _data.end())
  , _block_circulator(_data_circulator, _block_size)
{
  assert(_block_size >= 1);
}

/** Write a block of data to the delay line.
 * Before writing, the read and write pointers are advanced to the next block.
 * If you don't want to use this function, you can also call advance(), get the
 * write pointer with get_write_pointer() and write directly to it.
 * @param source Pointer/iterator where the block of data shall be
 * read from.
 * @attention In @p source there must be enough data to read from!
 * @note @p source must be a random access iterator. If you want to use another
 * iterator, you'll have to do it on your own (as written above).
 **/
template<typename T, typename Container>
template<typename Iterator>
void
BlockDelayLine<T, Container>::write_block(Iterator source)
{
  this->advance();
  // Ignore return value, next time get_write_pointer() has to be used again!
  std::copy(source, source + _block_size, this->get_write_pointer());
}

/** Read a block of data from the delay line.
 * @param destination Iterator to destination
 * @param delay Delay in samples
 * @return @b true on success
 **/
template<typename T, typename Container>
template<typename Iterator>
bool
BlockDelayLine<T, Container>::read_block(Iterator destination, size_type delay)
  const
{
  // TODO: try to get a more meaningful error message if source is not a random
  // access iterator (e.g. when using a std::list)
  if (!this->delay_is_valid(delay)) return false;
  circulator source = this->get_read_circulator(delay);
  std::copy(source, source + _block_size, destination);
  return true;
}

/// Read from the delay line and multiply each element by a given factor.
template<typename T, typename Container>
template<typename Iterator>
bool
BlockDelayLine<T, Container>::read_block(Iterator destination
    , size_type delay, T weight) const
{
  if (!this->delay_is_valid(delay)) return false;
  circulator source = this->get_read_circulator(delay);
  std::transform(source, source + _block_size, destination
      , [weight] (T in) { return in * weight; });
  return true;
}

/** Get the write pointer.
 * @attention Before the write operation, advance() must be called to
 * update read and write pointers.
 * @attention You must not write more than one block with this pointer! For
 * the next block, you have to get a new pointer.
 **/
template<typename T, typename Container>
typename BlockDelayLine<T, Container>::pointer
BlockDelayLine<T, Container>::get_write_pointer() const
{
  return &*_block_circulator.base().base();
}

/** Get the read circulator.
 * @param delay Delay in samples
 * @attention There is no check if the delay is in the valid range between
 * 0 and @c max_delay. You are responsible for checking that!
 **/
template<typename T, typename Container>
typename BlockDelayLine<T, Container>::circulator
BlockDelayLine<T, Container>::get_read_circulator(size_type delay) const
{
  return _get_data_circulator() - delay;
}

/** A block-based delay line where negative delay is possible.
 * This is done by delaying everything by a given initial delay. The (absolute
 * value of the) negative delay can be at most as large as the initial delay.
 * @see BlockDelayLine
 **/
template<typename T, typename Container = std::vector<T>>
class NonCausalBlockDelayLine : private BlockDelayLine<T, Container>
{
  private:
    using _base = BlockDelayLine<T, Container>;

  public:
    using size_type = typename _base::size_type;
    using circulator = typename _base::circulator;
    using difference_type = typename circulator::difference_type;

    /// Constructor. @param initial_delay initial delay
    /// @param block_size Block size
    /// @param max_delay Maximum delay in samples
    /// @param initial_delay Additional delay to achieve negative delay
    /// @see BlockDelayLine::BlockDelayLine()
    NonCausalBlockDelayLine(size_type block_size, size_type max_delay
        , size_type initial_delay)
      : _base(block_size, max_delay + initial_delay)
      , _initial_delay(initial_delay)
    {}

#ifdef APF_DOXYGEN_HACK
    // This is just for Doxygen documentation:
    /// @see BlockDelayLine::advance()
    void advance();
    /// @see BlockDelayLine::write_block()
    template<typename Iterator> void write_block(Iterator source);
    /// @see BlockDelayLine::get_write_pointer()
    pointer get_write_pointer() const;
#else
    // This is the real thing:
    using _base::advance;
    using _base::write_block;
    using _base::get_write_pointer;
#endif

    /// @see BlockDelayLine::delay_is_valid()
    bool delay_is_valid(difference_type delay) const
    {
      if (delay < -_initial_delay) return false;
      if (delay < 0) return true;
      return _base::delay_is_valid(delay + _initial_delay);
    }

    /// @see BlockDelayLine::read_block()
    template<typename Iterator>
    bool read_block(Iterator destination, difference_type delay) const
    {
      if (delay < -_initial_delay) return false;
      return _base::read_block(destination, delay + _initial_delay);
    }

    /// @see BlockDelayLine::read_block()
    template<typename Iterator>
    bool read_block(Iterator destination, difference_type delay, T weight) const
    {
      if (delay < -_initial_delay) return false;
      return _base::read_block(destination, delay + _initial_delay, weight);
    }

    /// @see BlockDelayLine::get_read_circulator()
    circulator get_read_circulator(difference_type delay = 0) const
    {
      return _base::get_read_circulator(delay + _initial_delay);
    }

  private:
    const difference_type _initial_delay;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
