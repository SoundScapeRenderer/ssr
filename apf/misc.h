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
/// Miscellaneous helper classes.

#ifndef APF_MISC_H
#define APF_MISC_H

#include <utility>  // for std::forward
#include <type_traits>  // for std::is_base_of

namespace apf
{

/// Curiously Recurring Template Pattern (CRTP) base class.
/// The idea for derived() is stolen from the Eigen library.
template<typename Derived>
class CRTP
{
  public:
    Derived& derived()
    {
      static_assert(std::is_base_of<CRTP, Derived>::value
          , "Derived must be derived from CRTP (as the name suggests)!");

      return *static_cast<Derived*>(this);
    }

  protected:
     CRTP() = default;
    ~CRTP() = default;  ///< Protected destructor to avoid base class pointers
};

/// Classes derived from this class cannot be copied (but still moved).
class NonCopyable
{
  protected:
     NonCopyable() = default;  ///< Protected default constructor
    ~NonCopyable() = default;  ///< Protected non-virtual destructor

  public:
    /// @name Deactivated copy ctor and assignment operator
    //@{
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    //@}

    /// @name Default move ctor and move assignment operator
    //@{
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
    //@}
};

/// Hold current and old value of any type.
/// A new value can be assigned with operator=().
/// The current and old value can be obtained with get() and old(),
/// respectively.
/// To find out if the old and current value are different, use changed().
/// BlockParameter is supposed to avoid pairs of variables where one
/// represents an old value and the other a new one. The @e old value of
/// BlockParameter is updated in operator=() (and only there).
/// @attention It's @e not possible to use operator=() without updating the
///   @e old value. To avoid the unintended use of operator=(), use
///                                                                        @code
///   #include <cassert>
///   BlockParameter<int> bp(42);
///   bp = 23;
///   assert(bp.exactly_one_assignment());
///                                                                     @endcode
///   This is of course only checked if NDEBUG is undefined.
/// @tparam T The contained type.
template<typename T>
class BlockParameter
{
  public:
    /// Constructor. Any arguments are forwarded to both old and current value.
    template<typename... Args>
    explicit BlockParameter(Args&&... args)
      : _current{args...}
      , _old{std::forward<Args>(args)...}
    {}

    /// Assignment operator.
    /// As a side effect, the old value is updated to the former current value.
    template<typename Arg>
    T& operator=(Arg&& arg)
    {
#ifndef NDEBUG
      ++_assignments;
#endif

      _old = std::move(_current);
      return _current = std::forward<Arg>(arg);
    }

    const T& get() const { return _current; }  ///< Get current value.
    const T& old() const { return _old; }  ///< Get old value.

    /// Conversion operator. For avoiding to call get() all the time.
    operator const T&() const { return this->get(); }

    /// Check if value has changed between before the last assignment and now.
    bool changed() const { return this->get() != this->old(); }

    /// @name Operators which do not change the old value:
    /// @{
    BlockParameter& operator+=(const T& rhs) { _current += rhs; return *this; }
    BlockParameter& operator-=(const T& rhs) { _current -= rhs; return *this; }
    BlockParameter& operator*=(const T& rhs) { _current *= rhs; return *this; }
    BlockParameter& operator/=(const T& rhs) { _current /= rhs; return *this; }
    BlockParameter& operator%=(const T& rhs) { _current %= rhs; return *this; }
    BlockParameter& operator&=(const T& rhs) { _current &= rhs; return *this; }
    BlockParameter& operator|=(const T& rhs) { _current |= rhs; return *this; }
    BlockParameter& operator<<=(const T& rhs){ _current <<= rhs; return *this; }
    BlockParameter& operator>>=(const T& rhs){ _current >>= rhs; return *this; }

    BlockParameter& operator++() { ++_current; return *this; }
    BlockParameter& operator--() { --_current; return *this; }
    T operator++(int) { return _current++; }
    T operator--(int) { return _current--; }
    /// @}

  private:
    class both_proxy
    {
      public:
        explicit both_proxy(const BlockParameter& param) : _p(param) {}

#define APF_BLOCKPARAMETER_BOTH_PROXY_OPERATORS(opstring) \
        friend bool operator opstring(const both_proxy& lhs, const T& rhs) { \
          return lhs._p.get() opstring rhs && lhs._p.old() opstring rhs; } \
        friend bool operator opstring(const T& lhs, const both_proxy& rhs) { \
          return lhs opstring rhs._p.get() && lhs opstring rhs._p.old(); }

        APF_BLOCKPARAMETER_BOTH_PROXY_OPERATORS(==)
        APF_BLOCKPARAMETER_BOTH_PROXY_OPERATORS(!=)
        APF_BLOCKPARAMETER_BOTH_PROXY_OPERATORS(<)
        APF_BLOCKPARAMETER_BOTH_PROXY_OPERATORS(>)
        APF_BLOCKPARAMETER_BOTH_PROXY_OPERATORS(<=)
        APF_BLOCKPARAMETER_BOTH_PROXY_OPERATORS(>=)
#undef APF_BLOCKPARAMETER_BOTH_PROXY_OPERATORS

      private:
        const BlockParameter& _p;
    };

  public:

    both_proxy both() const { return both_proxy(*this); }

#ifndef NDEBUG
    bool no_multiple_assignments() const
    {
      auto result = (_assignments <= 1);
      _assignments = 0;
      return result;
    }

    bool exactly_one_assignment() const
    {
      auto result = (_assignments == 1);
      _assignments = 0;
      return result;
    }
#endif

  private:
    T _current, _old;

#ifndef NDEBUG
    mutable int _assignments = 0;
#endif
};

} // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
