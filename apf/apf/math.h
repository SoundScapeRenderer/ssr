/******************************************************************************
 Copyright (c) 2012-2016 Institut für Nachrichtentechnik, Universität Rostock
 Copyright (c) 2006-2012 Quality & Usability Lab
                         Deutsche Telekom Laboratories, TU Berlin

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*******************************************************************************/

// https://AudioProcessingFramework.github.io/

/// @file
/// Mathematical constants and helper functions.

#ifndef APF_MATH_H
#define APF_MATH_H

#include <cmath>  // for std::pow(), ...
#include <iterator> // for std::iterator_traits
#include <numeric>  // for std::accumulate()
#include <algorithm>  // for std::max()

namespace apf
{
/// Mathematical constants and helper functions
namespace math
{

/// \f$\pi\f$.
/// Undefined general case.
template<typename T> inline T pi();

/// \f$\pi\f$.
/// Specialization for float.
template<>
inline float pi<float>()
{
  // 9 digits are needed for float, 17 digits for double, 21 for long double
  // TODO: But this may be different on other hardware platforms ...?
  return 3.1415926535897932384626433832795f;
}

/// \f$\pi\f$.
/// Specialization for double.
template<>
inline double pi<double>()
{
  return 3.1415926535897932384626433832795;
}

/// \f$\pi\f$.
/// Specialization for long double.
template<>
inline long double pi<long double>()
{
  return 3.1415926535897932384626433832795l;
}

/// \f$\pi/180\f$
template<typename T>
inline T pi_div_180()
{
  // local variable to avoid warning message about conversion
  T q = 180;
  return pi<T>() / q;
}

/** Product of a number with itself
 * @param x number (any numeric type)
 * @return @a x squared.
 **/
template<typename T> inline T square(T x) { return x*x; }

/** Convert a level in decibel to a linear gain factor.
 * @param L level
 * @param power if @b true, a factor of 10 is used, if @b false (default), the
 * factor is 20.
 * @return the linear counterpart to @a L.
 **/
template<typename T>
inline T dB2linear(T L, bool power = false)
{
  T f = 20; if (power) f = 10;
  T base = 10;
  return std::pow(base, L / f);
}

/** Convert a linear gain factor to a level in dB.
 * @param x gain factor
 * @param power if @b true, a factor of 10 is used, if @b false (default), the
 * factor is 20.
 * @return the logarithmic counterpart to @a x.
 * @attention returns -Inf for x=0 and NaN for x<0.
 **/
template<typename T>
inline T linear2dB(T x, bool power = false)
{
  T f = 20; if (power) f = 10;
  return f * std::log10(x);
}

/** Convert an angle in degrees to an angle in radians.
 * @param angle dito
 * @return angle in radians.
 **/
template<typename T>
inline T deg2rad(T angle)
{
  return angle * pi_div_180<T>();
}

/** Convert an angle in radians to an angle in degrees.
 * @param angle dito
 * @return angle in degrees.
 **/
template<typename T>
inline T rad2deg(T angle)
{
  return angle / pi_div_180<T>();
}

/// Wrap @p x into the interval [0, @p full).
/// Helper function for float, double and long double.
/// @see wrap()
template<typename T>
inline T fwrap(T x, T full)
{
  x = std::fmod(x, full);
  if (x < 0) x += full;
  return x;
}

/// Wrap @p x into the interval [0, @p full).
/// Unspecialized case, works only for integral types.
template<typename T>
inline T wrap(T x, T full)
{
  x %= full;
  if (x < 0) x += full;
  return x;
}

/// Wrap @p x into the interval [0, @p full).
/// Template specialization for float.
template<>
inline float wrap(float x, float full)
{
  return fwrap(x, full);
}

/// Wrap @p x into the interval [0, @p full).
/// Template specialization for double.
template<>
inline double wrap(double x, double full)
{
  return fwrap(x, full);
}

/// Wrap @p x into the interval [0, @p full).
/// Template specialization for long double.
template<>
inline long double wrap(long double x, long double full)
{
  return fwrap(x, full);
}

template<typename T>
inline T wrap_two_pi(T x)
{
  // local variable to avoid warning message about conversion
  T two = 2;
  return wrap(x, two * pi<T>());
}

/** Find a power of 2 which is >= a given number.
 * @param number number for which to find next power of 2
 * @return power of 2 above (or equal to) \b number 
 * @note For all @p number%s <= 1 the result is 1;
 **/
template<typename T>
inline T next_power_of_2(T number)
{
  T power_of_2 = 1;
  while (power_of_2 < number) power_of_2 *= 2;
  return power_of_2;
}

/** Return the absolute maximum of a series of numbers.
 * @param begin beginning of range
 * @param end         end of range
 * @return maximum, this is always >= 0.
 **/
template<typename I>
inline typename std::iterator_traits<I>::value_type
max_amplitude(I begin, I end)
{
  using T = typename std::iterator_traits<I>::value_type;
  return std::accumulate(begin, end, T()
      , [] (T current, T next) { return std::max(current, std::abs(next)); });
}

/** Root Mean Square (RMS) value of a series of numbers.
 * @param begin beginning of range
 * @param end         end of range
 * @return RMS value
 **/
template<typename I>
inline typename std::iterator_traits<I>::value_type rms(I begin, I end)
{
  using T = typename std::iterator_traits<I>::value_type;

  // inner product: sum of squares
  // divided by number: mean
  // sqrt: root
  return std::sqrt(std::inner_product(begin, end, begin, T())
      / static_cast<T>(std::distance(begin, end)));
}

/** Check if there are only zeros in a range.
 * @return @b false as soon as a non-zero value is encountered
 **/
template<typename I>
bool has_only_zeros(I first, I last)
{
  while (first != last) if (*first++ != 0) return false;
  return true;
}

/** Raised cosine (function object).
 * Result ranges from 0 to 1.
 **/
template<typename T>
class raised_cosine
{
  public:
    using argument_type = T;
    using result_type = T;

    /// Constructor. @param period of a full cosine oscillation
    explicit raised_cosine(T period = 0) : _period(period) {}

    /// Function call operator
    T operator()(T in) const
    {
      // local variable to avoid warning about conversion
      T half = 0.5;
      return std::cos(in * 2 * pi<T>() / _period) * half + half;
    }

  private:
    const T _period;
};

/** Function object for linear interpolation.
 * @tparam T result type
 * @tparam U input type
 * @warning If @p U is an integral type and @p T is floating point, don't
 *   forget that the cast operation takes some processing time. It may be more
 *   efficient to use the same floating point type for both @p T and @p U.
 **/
template<typename T, typename U = T>
class linear_interpolator
{
  public:
    using argument_type = U;
    using result_type = T;

    /// Default constructor
    linear_interpolator() : _first(), _increment() {}

    /// Constructor with range and optional length.
    /// @see set()
    linear_interpolator(result_type first, result_type last
        , argument_type length = argument_type(1))
    {
      this->set(first, last, length);
    }

    /// Set range and optional length.
    /// @param first output value if input is zero
    /// @param last output value if input is @p length
    /// @param length length of interval on which to interpolate
    void set(result_type first, result_type last
        , argument_type length = argument_type(1))
    {
      _first = first;
      _increment = (last - first) / length;
    }

    /// Function call operator
    result_type operator()(argument_type x)
    {
      return _first + result_type(x) * _increment;
    }

  private:
    result_type _first, _increment;
};

/// Helper function for automatic template type deduction
template<typename T>
linear_interpolator<T>
make_linear_interpolator(T first, T last)
{
  return linear_interpolator<T>(first, last);
}

/// Helper function for automatic template type deduction
template<typename T, typename U>
linear_interpolator<T, U>
make_linear_interpolator(T first, T last, U length)
{
  return linear_interpolator<T, U>(first, last, length);
}

/// Identity function object. Function call returns a const reference to input.
template<typename T>
struct identity { const T& operator()(const T& in) { return in; } };

}  // namespace math
}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
