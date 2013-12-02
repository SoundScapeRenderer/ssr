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
/// Some containers.

#ifndef APF_CONTAINER_H
#define APF_CONTAINER_H

#include <memory>  // for std::allocator
#include <vector>
#include <list>
#include <stdexcept>  // for std::logic_error
#include <algorithm>  // for std::find

#include "apf/iterator.h"  // for stride_iterator, ...

namespace apf
{

// TODO: move metaprogramming stuff into separate file?
namespace internal
{
  template<typename T1, typename...> struct first { using type = T1; };

  // This didn't work with GCC 4.8.2 (segmentation fault during compilation)
  //template<typename T1, typename...> using first = T1;

  template<typename T1, typename... Ts> struct last : last<Ts...> {};
  template<typename T1> struct last<T1> { using type = T1; };

  template<typename... Args> using if_first_not_integral
    = typename std::enable_if<
      !std::is_integral<typename first<Args...>::type>::value>::type;
  template<typename X> using if_integral
    = typename std::enable_if<std::is_integral<X>::value>::type;

  template<typename Arg, typename... Args> using if_last_not_convertible
    = typename std::enable_if<
    !std::is_convertible<typename last<Args...>::type, Arg>::value>::type;
}

/** Derived from @c std::vector, but without memory re-allocations.
 * Non-copyable types can be used as long as they are movable.
 * Normally, the size is specified in the constructor and doesn't change ever.
 * If you need to initialize the fixed_vector before you know its final size,
 * there is one exception: You can initialize the fixed_vector with the default
 * constructor, at a later time you can call reserve() and afterwards
 * emplace_back(). In this case the size grows, but the memory is still never
 * re-allocated.
 * @par Differences to @c std::vector:
 * - There are slightly different constructors, especially one with a
 *   size-argument and further arbitrary arguments which are forwarded to the
 *   constructor of each element.
 * - reserve() and emplace_back() have different semantics.
 * - all other functions which (potentially) change size are disabled.
 **/
template<typename T, typename Allocator = std::allocator<T>>
class fixed_vector : public std::vector<T, Allocator>
{
  private:
    using _base = typename std::vector<T, Allocator>;

  public:
    using value_type = typename _base::value_type;
    using size_type = typename _base::size_type;

    fixed_vector() = default;
    fixed_vector(fixed_vector&&) = default;
    fixed_vector(const fixed_vector&) = delete;
    fixed_vector& operator=(const fixed_vector&) = delete;
    fixed_vector& operator=(fixed_vector&&) = delete;

    /// Constructor that forwards everything except if first type is integral.
    template<typename... Args, typename =
                                       internal::if_first_not_integral<Args...>>
    explicit fixed_vector(Args&&... args)
      : _base(std::forward<Args>(args)...)
    {}

// TODO: constructor from size and allocator is missing in C++11 (but not C++14)
#if 0
    // TODO: re-activate with C++14:
    template<typename Size, typename = internal::if_integral<Size>>
    fixed_vector(Size n, const Allocator& a = Allocator())
      : _base(n, a)
    {}
#else
    explicit fixed_vector(size_type n) : _base(n) {}
#endif

    template<typename Size, typename Arg
      , typename = internal::if_integral<Size>>
    fixed_vector(Size n, Arg&& arg, const Allocator& a)
      : _base(n, std::forward<Arg>(arg), a)
    {}

    /// Constructor from size and initialization arguments.
    /// This can be used for initializing nested containers, for example.
    template<typename Size, typename... Args
      , typename = internal::if_integral<Size>
      , typename = internal::if_last_not_convertible<Allocator, Args...>>
    explicit fixed_vector(Size n, Args&&... args)
      : _base()
    {
      _base::reserve(static_cast<size_type>(n));
      for (Size i = 0; i < n; ++i)
      {
        // Note: std::forward is not used here, because it's called repeatedly
        _base::emplace_back(args...);
      }
    }

    // Perfect forwarding doesn't cover initializer lists:
    explicit fixed_vector(std::initializer_list<value_type> il
        , const Allocator& a = Allocator())
      : _base(il, a)
    {}

    /** Reserve space for new elements and default-construct them.
     * In contrast to @c std::vector::resize(), this can only be called @e once
     * and only on an empty fixed_vector (i.e. iff capacity == 0).
     * Thus, resize() will allocate memory, but never @e re-allocate.
     * @throw std::logic_error if capacity != 0
     **/
    void resize(size_type n)
    {
      if (this->capacity() == 0)
      {
        _base::resize(n);
      }
      else
      {
        throw std::logic_error(
            "Bug: fixed_vector::resize() is only allowed if capacity == 0!");
      }
    }

    /** Reserve space for new elements.
     * In contrast to @c std::vector::reserve(), this can only be called @e once
     * and only on an empty fixed_vector (i.e. iff capacity == 0).
     * Thus, reserve() will allocate memory, but never @e re-allocate.
     * @throw std::logic_error if capacity != 0
     **/
    void reserve(size_type n)
    {
      if (this->capacity() == 0)
      {
        _base::reserve(n);
      }
      else
      {
        throw std::logic_error(
            "Bug: fixed_vector::reserve() is only allowed if capacity == 0!");
      }
    }

    /** Construct element at the end.
     * In contrast to @c std::vector::emplace_back() this can @e only be called
     * after reserve() and at most as many times as specified in reserve() (and
     * is typically called @e exactly as many times).
     * Thus, memory will never be allocated.
     * @throw std::logic_error if capacity would be exceeded
     **/
    template<typename... Args>
    void emplace_back(Args&&... args)
    {
      if (this->size() < this->capacity())
      {
        _base::emplace_back(std::forward<Args>(args)...);
      }
      else
      {
        throw std::logic_error(
            "Bug: fixed_vector::emplace_back() "
            "is only allowed if size < capacity!");
      }
    }

  private:
    // Hide all base class functions which would change size:
    void resize();
    void assign();
    void push_back();
    void pop_back();
    void insert();
    void erase();
    void swap();
    void clear();
    void emplace();
};

/** Derived from std::list, but without re-sizing.
 * Items cannot be added/removed, but they can be re-ordered with move().
 **/
template<typename T, typename Allocator = std::allocator<T>>
class fixed_list : public std::list<T, Allocator>
{
  private:
    using _base = typename std::list<T, Allocator>;

  public:
    using value_type = typename _base::value_type;
    using iterator = typename _base::iterator;

    fixed_list() = default;
    fixed_list(fixed_list&&) = default;
    fixed_list(const fixed_list&) = delete;
    fixed_list& operator=(const fixed_list&) = delete;
    fixed_list& operator=(fixed_list&&) = delete;

    /// Constructor that forwards everything except if first type is integral.
    template<typename... Args, typename =
                                       internal::if_first_not_integral<Args...>>
    explicit fixed_list(Args&&... args)
      : _base(std::forward<Args>(args)...)
    {}

    /// Constructor from size and initialization arguments.
    template<typename Size, typename... Args, typename =
                                                    internal::if_integral<Size>>
    explicit fixed_list(Size n, Args&&... args)
      : _base()
    {
      for (Size i = 0; i < n; ++i)
      {
        // Note: std::forward is not used here, because it's called repeatedly
        _base::emplace_back(args...);
      }
    }

    explicit fixed_list(std::initializer_list<value_type> il
        , const Allocator& a = Allocator())
      : _base(il, a)
    {}

    /// Move list element @p from one place @p to another.
    /// @p from is placed in front of @p to.
    /// No memory is allocated/deallocated, no content is copied.
    void move(iterator from, iterator to)
    {
      _base::splice(to, *this, from);
    }

    /// Move range (from @p first to @p last) to @p target.
    /// The range is placed in front of @p target.
    /// No memory is allocated/deallocated, no content is copied.
    void move(iterator first, iterator last, iterator target)
    {
      _base::splice(target, *this, first, last);
    }

  private:
    // Hide all base class functions which would change size:
    void assign();
    void emplace_front();
    void emplace_back();
    void push_front();
    void pop_front();
    void push_back();
    void pop_back();
    void emplace();
    void insert();
    void erase();
    void swap();
    void resize();
    void clear();
    void splice();
    void remove();
    void remove_if();
    void unique();
    void merge();
};

/** Two-dimensional data storage for row- and column-wise access.
 * The two dimensions have following properties:
 *   -# Channel
 *     - stored in contiguous memory
 *     - fixed_matrix can be iterated from channels.begin() to channels.end()
 *       (using fixed_matrix::channels_iterator)
 *     - resulting channel can be iterated from .begin() to .end()
 *       (using fixed_matrix::channel_iterator)
 *   -# Slice
 *     - stored in memory locations with constant step size
 *     - fixed_matrix can be iterated from slices.begin() to slices.end()
 *       (using fixed_matrix::slices_iterator)
 *     - resulting slice can be iterated from .begin() to .end()
 *       (using fixed_matrix::slice_iterator)
 *
 * @tparam T Type of stored data
 **/
template<typename T, typename Allocator = std::allocator<T>>
class fixed_matrix : public fixed_vector<T, Allocator>
{
  private:
    using _base = fixed_vector<T, Allocator>;

  public:
    using pointer = typename _base::pointer;
    using size_type = typename _base::size_type;

    /// Proxy class for returning one channel of the fixed_matrix
    using Channel = has_begin_and_end<pointer>;
    /// Iterator within a Channel
    using channel_iterator = typename Channel::iterator;

    /// Proxy class for returning one slice of the fixed_matrix
    using Slice = has_begin_and_end<stride_iterator<channel_iterator>>;
    /// Iterator within a Slice
    using slice_iterator = typename Slice::iterator;

    class channels_iterator;
    class slices_iterator;

    /// Default constructor.
    /// Only initialize() makes sense after this.
    explicit fixed_matrix(const Allocator& a = Allocator())
      : _base(a)
    {
      this->initialize(0, 0);
    }

    fixed_matrix(fixed_matrix&&) = default;
    fixed_matrix(const fixed_matrix&) = delete;
    fixed_matrix& operator=(const fixed_matrix&) = delete;
    fixed_matrix& operator=(fixed_matrix&&) = delete;

    /** Constructor.
     * @param max_channels Number of Channels
     * @param max_slices Number of Slices
     * @param a Optional allocator
     **/
    fixed_matrix(size_type max_channels, size_type max_slices
        , const Allocator& a = Allocator())
      : fixed_matrix(a)
    {
      this->initialize(max_channels, max_slices);
    }

    /// Allocate memory for @p max_channels x @p max_slices elements and
    /// default-construct them.
    /// @pre empty() == true
    void initialize(size_type max_channels, size_type max_slices)
    {
      _base::resize(max_channels * max_slices);

      this->channels = make_begin_and_end(
          channels_iterator(_base::data(), max_slices), max_channels);
      this->slices = make_begin_and_end(
          slices_iterator(_base::data(), max_channels, max_slices), max_slices);

      _channel_ptrs.reserve(max_channels);
      for (const auto& channel: this->channels)
      {
        _channel_ptrs.emplace_back(&*channel.begin());
      }
      assert(_channel_ptrs.size() == max_channels);
    }

    template<typename Ch>
    void set_channels(const Ch& ch);

    /// Get array of pointers to the channels. This can be useful to interact
    /// with functions which use plain pointers instead of iterators.
    pointer const* get_channel_ptrs() const { return _channel_ptrs.data(); }

    /// Access to Channels; use channels.begin() and channels.end()
    has_begin_and_end<channels_iterator> channels;
    /// Access to Slices; use slices.begin() and slices.end()
    has_begin_and_end<slices_iterator> slices;

  private:
    // Hide functions from fixed_vector:
    void emplace_back();
    void reserve();
    void resize();

    fixed_vector<pointer> _channel_ptrs;
};

/// Iterator over fixed_matrix::Channel%s.
template<typename T, typename Allocator>
class fixed_matrix<T, Allocator>::channels_iterator
{
  private:
    using self = channels_iterator;
    using _base_type = stride_iterator<channel_iterator>;

    /// Helper class for operator->()
    struct ChannelArrowProxy : Channel
    {
      ChannelArrowProxy(const Channel& ch) : Channel(ch) {}
      Channel* operator->() { return this; }
    };

  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Channel;
    using reference = Channel;
    using difference_type = typename _base_type::difference_type;
    using pointer = ChannelArrowProxy;

    /// Default constructor.
    /// @note This constructor creates a singular iterator. Another
    /// channels_iterator can be assigned to it, but nothing else works.
    channels_iterator()
      : _size(0)
    {}

    /// Constructor.
    channels_iterator(channel_iterator base_iterator, size_type step)
      : _base_iterator(base_iterator, step)
      , _size(step)
    {}

    /// Dereference operator.
    /// @return a proxy object of type fixed_matrix::Channel
    reference operator*() const
    {
      auto temp = _base_iterator.base();
      assert(apf::no_nullptr(temp));
      return Channel(temp, temp + _size);
    }

    /// Arrow operator.
    /// @return a proxy object of type fixed_matrix::ChannelArrowProxy
    pointer operator->() const
    {
      return this->operator*();
    }

    APF_ITERATOR_RANDOMACCESS_EQUAL(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREINCREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREDECREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_DIFFERENCE(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_SUBSCRIPT
    APF_ITERATOR_RANDOMACCESS_LESS(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_UNEQUAL
    APF_ITERATOR_RANDOMACCESS_OTHER_COMPARISONS
    APF_ITERATOR_RANDOMACCESS_POSTINCREMENT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_THE_REST

    APF_ITERATOR_BASE(_base_type, _base_iterator)

  private:
    _base_type _base_iterator;
    size_type _size;
};

/// Iterator over fixed_matrix::Slice%s.
template<typename T, typename Allocator>
class fixed_matrix<T, Allocator>::slices_iterator
{
  private:
    using self = slices_iterator;

    /// Helper class for operator->()
    struct SliceArrowProxy : Slice
    {
      SliceArrowProxy(const Slice& sl) : Slice(sl) {}
      Slice* operator->() { return this; }
    };

  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Slice;
    using reference = Slice;
    using pointer = SliceArrowProxy;
    using difference_type
      = typename std::iterator_traits<channel_iterator>::difference_type;

    /// Default constructor.
    /// @note This constructor creates a singular iterator. Another
    /// slices_iterator can be assigned to it, but nothing else works.
    slices_iterator()
      : _max_channels(0)
      , _max_slices(0)
    {}

    /// Constructor.
    slices_iterator(channel_iterator base_iterator
        , size_type max_channels, size_type max_slices)
      : _base_iterator(base_iterator)
      , _max_channels(max_channels)
      , _max_slices(max_slices)
    {}

    /// Dereference operator.
    /// @return a proxy object of type fixed_matrix::Slice
    reference operator*() const
    {
      assert(apf::no_nullptr(_base_iterator));
      slice_iterator temp(_base_iterator, _max_slices);
      return Slice(temp, temp + _max_channels);
    }

    /// Arrow operator.
    /// @return a proxy object of type fixed_matrix::SliceArrowProxy
    pointer operator->() const
    {
      return this->operator*();
    }

    APF_ITERATOR_RANDOMACCESS_EQUAL(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREINCREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREDECREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_DIFFERENCE(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_SUBSCRIPT
    APF_ITERATOR_RANDOMACCESS_LESS(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_UNEQUAL
    APF_ITERATOR_RANDOMACCESS_OTHER_COMPARISONS
    APF_ITERATOR_RANDOMACCESS_POSTINCREMENT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_THE_REST

    APF_ITERATOR_BASE(channel_iterator, _base_iterator)

  private:
    channel_iterator _base_iterator;
    size_type _max_channels;
    size_type _max_slices;
};

/** Copy channels from another matrix.
 * @param ch channels (or slices) to copy from another fixed_matrix
 * @note A plain copy may be faster with @c std::copy() from
 *   fixed_matrix::begin() to fixed_matrix::end().
 * @note Anyway, a plain copy of a fixed_matrix is rarely needed, the main
 *   reason for this function is that if you use slices instead of channels,
 *   you'll get a transposed matrix.
 * @pre The dimensions must be correct beforehand!
 * @warning If the dimensions are not correct, bad things will happen!
 **/
template<typename T, typename Allocator>
template<typename Ch>
void
fixed_matrix<T, Allocator>::set_channels(const Ch& ch)
{
  assert(std::distance(ch.begin(), ch.end())
      == std::distance(this->channels.begin(), this->channels.end()));
  assert((ch.begin() == ch.end()) ? true :
      std::distance(ch.begin()->begin(), ch.begin()->end()) ==
      std::distance(this->channels.begin()->begin()
                  , this->channels.begin()->end()));

  auto target = this->channels.begin();

  for (const auto& i: ch)
  {
    std::copy(i.begin(), i.end(), target->begin());
    ++target;
  }
}

/// Append pointers to the elements of the first list to the second list.
/// @note @c L2::value_type must be a pointer to @c L1::value_type!
template<typename L1, typename L2>
void append_pointers(L1& source, L2& target)
{
  for (auto& i: source)
  {
    target.push_back(&i);
  }
}

/// Const-version of append_pointers()
/// @note @c L2::value_type must be a pointer to @b const @c L1::value_type!
template<typename L1, typename L2>
void append_pointers(const L1& source, L2& target)
{
  for (const auto& i: source)
  {
    target.push_back(&i);
  }
}

/// Splice list elements from @p source to member lists of @p target.
/// @param source Elements of this list are distributed to the corresponding
///   @p member lists of @p target. This must have the same type as @p member.
/// @param target Each element of this list receives one element of @p source.
/// @param member The distributed elements are appended at @c member.end().
/// @note Lists must have the same size. If not, an exception is thrown.
/// @note There is no const version, both lists are modified.
/// @post @p source will be empty.
/// @post The @p member of each @p target element will have one more element.
template<typename L1, typename L2, typename DataMember>
void distribute_list(L1& source, L2& target, DataMember member)
{
  if (source.size() != target.size())
  {
    throw std::logic_error("distribute_list: Different sizes!");
  }

  auto in = source.begin();

  for (auto& out: target)
  {
    (out.*member).splice((out.*member).end(), source, in++);
  }
}

/// The opposite of distribute_list() -- sorry for the strange name!
/// @param source Container of items which will be removed from @p member of the
///   corresponding @p target elements.
/// @param target Container of elements which have a @p member.
/// @param member Member container from which elements will be removed. Must
///   have a splice() member function (like @c std::list).
/// @param garbage Removed elements are appended to this list. Must have the
///   same type as @p member.
/// @throw std::logic_error If any element isn't found in the corresponding
///   @p member.
/// @attention If a list element is not found, an exception is thrown and the
///   original state is @b not restored!
// TODO: better name?
template<typename L1, typename L2, typename DataMember, typename L3>
void
undistribute_list(const L1& source, L2& target, DataMember member, L3& garbage)
{
  if (source.size() != target.size())
  {
    throw std::logic_error("undistribute_list(): Different sizes!");
  }

  auto in = source.begin();

  for (auto& out: target)
  {
    auto delinquent
      = std::find((out.*member).begin(), (out.*member).end(), *in++);
    if (delinquent == (out.*member).end())
    {
      throw std::logic_error("undistribute_list(): Element not found!");
    }
    garbage.splice(garbage.end(), out.*member, delinquent);
  }
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
