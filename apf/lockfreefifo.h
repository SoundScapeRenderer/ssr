/******************************************************************************
 * Copyright © 2012-2013 Institut für Nachrichtentechnik, Universität Rostock *
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
/// Lock-free first-in-first-out queue.

#ifndef APF_LOCKFREEFIFO_H
#define APF_LOCKFREEFIFO_H

#include "apf/math.h"  // for next_power_of_2()
#include "apf/misc.h"  // for NonCopyable
#include "apf/container.h"  // for fixed_vector

namespace apf
{

template<typename T> class LockFreeFifo;  // undefined, use LockFreeFifo<T*>!

/** Lock-free first-in-first-out (FIFO) queue. 
 * It is thread-safe for single reader/single writer access.
 * One thread may use push() to en-queue items and another thread may use pop()
 * to de-queue items.
 * @note This FIFO queue is implemented as a ring buffer.
 * @note This class is somehow related to the JACK ringbuffer implementation:
 *   http://jackaudio.org/files/docs/html/ringbuffer_8h.html
 **/
template<typename T>
class LockFreeFifo<T*> : NonCopyable
{
  public:
    explicit LockFreeFifo(size_t size);

    bool push(T* item);
    T* pop();
    bool empty() const;

  private:
    volatile size_t _write_index; ///< Write pointer
    volatile size_t _read_index;  ///< Read pointer
    const size_t _size;           ///< Size of the ringbuffer
    const size_t _size_mask;      ///< Bit mask used in modulo operation
    fixed_vector<T*> _data;       ///< Actual ringbuffer data
};

/** ctor.
 * @param size desired ring buffer size, gets rounded up to the next power of 2.
 **/
template<typename T>
LockFreeFifo<T*>::LockFreeFifo(size_t size)
  : _write_index(0)
  ,  _read_index(0)
  , _size(apf::math::next_power_of_2(size))
  , _size_mask(_size - 1)
  , _data(_size)
{}

/** Add an item to the queue.
 * @param item pointer to an item to be added. 
 * @return @b true on success, @b false if queue is full.
 * @attention You have to check the return value to be sure the item has
 *   actually been added.
 **/
template<typename T>
bool
LockFreeFifo<T*>::push(T* item)
{
  if (item == nullptr) return false;

  // Concurrent reading and writing is safe for one reader and writer. Once
  // the _read_index is read the _write_index won't change before reading 
  // it, because it is modified only in this function. This won't work for
  // multiple readers/writers.  
  auto r = _read_index;
  auto w = _write_index;

  // Move write pointer by FIFO-size in order to compute the distance to 
  // the read pointer in next step. 
  if (w < r) w += _size;

  // Check if FIFO is full and return false instead of waiting until space is
  // freed. (Prevent read pointer to overtake write pointer.) 
  if (w-r > _size-2) return false;

  _data[w & _size_mask] = item;

  // Set _write_index to next memory location (applying modulo operation)
  _write_index = ++w & _size_mask;
  return true;
}

/** Get an item and remove it from the queue.
 * @return Pointer to the item, @b 0 if queue is empty. 
 **/
template<typename T>
T*
LockFreeFifo<T*>::pop()
{
  T* retval = nullptr;

  if (this->empty()) return retval;

  auto r = _read_index;

  retval = _data[r];

  // Set _read_index to next memory location (applying modulo operation)
  _read_index = ++r & _size_mask;
  return retval;
}

/** Check if queue is empty.
 * @return @b true if empty.
 **/
template<typename T>
bool
LockFreeFifo<T*>::empty() const
{
  return _read_index == _write_index;
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
