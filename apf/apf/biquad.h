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
/// Second order recursive filter and more.

#ifndef APF_BIQUAD_H
#define APF_BIQUAD_H

#include <iosfwd>  // for std::ostream
#include <cmath>  // for std::pow(), std::tan(), std::sqrt(), ...
#include <complex>
#include <vector>
#include <cassert>  // for assert()

#include "apf/denormalprevention.h"
#include "apf/math.h"

namespace apf
{

// TODO: make macros for trivial operators (see iterator.h)
// TODO: combine SosCoefficients and LaplaceCoefficients in common class
// template and use typedef with dummy template arguments.

/// Coefficients of digital recursive filter (second order section).
/// @tparam T Internal data type
template<typename T>
struct SosCoefficients
{
  SosCoefficients(T b0_ = T(), T b1_ = T(), T b2_ = T()
                             , T a1_ = T(), T a2_ = T())
    : b0(b0_), b1(b1_), b2(b2_)
             , a1(a1_), a2(a2_)
  {}

  T b0, b1, b2;
  T     a1, a2;

  SosCoefficients& operator+=(const SosCoefficients& rhs)
  {
    b0 += rhs.b0; b1 += rhs.b1; b2 += rhs.b2;
                  a1 += rhs.a1; a2 += rhs.a2;
    return *this;
  }

  SosCoefficients operator+(const SosCoefficients& rhs) const
  {
    auto tmp = SosCoefficients(*this);
    return tmp += rhs;
  }

  SosCoefficients& operator*=(T rhs)
  {
    b0 *= rhs; b1 *= rhs; b2 *= rhs;
               a1 *= rhs; a2 *= rhs;
    return *this;
  }

  SosCoefficients operator*(T rhs) const
  {
    auto tmp = SosCoefficients(*this);
    return tmp *= rhs;
  }

  SosCoefficients& operator/=(T rhs)
  {
    b0 /= rhs; b1 /= rhs; b2 /= rhs;
               a1 /= rhs; a2 /= rhs;
    return *this;
  }

  SosCoefficients operator/(T rhs) const
  {
    auto tmp = SosCoefficients(*this);
    return tmp /= rhs;
  }

  friend SosCoefficients operator*(T lhs, const SosCoefficients& rhs)
  {
    auto temp = SosCoefficients(rhs);
    return temp *= lhs;
  }

  friend SosCoefficients
  operator-(const SosCoefficients& lhs, const SosCoefficients& rhs)
  {
    return {lhs.b0 - rhs.b0, lhs.b1 - rhs.b1, lhs.b2 - rhs.b2
                           , lhs.a1 - rhs.a1, lhs.a2 - rhs.a2};
  }

  friend std::ostream&
  operator<<(std::ostream& stream, const SosCoefficients& c)
  {
    stream << "b0: " << c.b0 << ", b1: " << c.b1 << ", b2: " << c.b2
                             << ", a1: " << c.a1 << ", a2: " << c.a2;
    return stream;
  }
};

/// Coefficients of analog recursive filter.
/// @tparam T Internal data type
template<typename T>
struct LaplaceCoefficients
{
  LaplaceCoefficients(T b0_ = T(), T b1_ = T(), T b2_ = T()
                                 , T a1_ = T(), T a2_ = T())
    : b0(b0_), b1(b1_), b2(b2_)
             , a1(a1_), a2(a2_)
  {}

  T b0, b1, b2;
  T     a1, a2;
};

/** Direct Form II recursive filter of second order.
 * @tparam T internal type of states and coefficients
 * @tparam DenormalPrevention method of denormal prevention (see apf::dp)
 * @see Cascade, bilinear()
 **/
template<typename T, template<typename> class DenormalPrevention = apf::dp::ac>
class BiQuad : public SosCoefficients<T> , private DenormalPrevention<T>
{
  public:
    using argument_type = T;
    using result_type = T;

    BiQuad() : w0(), w1(), w2() {}

    /// Assignment operator.
    /// Change coefficients when operator '=' is called with SosCoefficients.
    /// @param c New set of coefficients
    /// @note state is unchanged!
    BiQuad& operator=(const SosCoefficients<T>& c)
    {
      this->SosCoefficients<T>::operator=(c);
      return *this;
    }

    /// Process filter on single sample.
    /// @param in input sample
    /// @return output sample
    result_type operator()(argument_type in)
    {
      w0 = w1;
      w1 = w2;
      w2 = in - this->a1*w1 - this->a2*w0;

      this->prevent_denormals(w2);

      in = this->b0*w2 + this->b1*w1 + this->b2*w0;

      return in;
    }

    T w0, w1, w2;
};

/// %Cascade of filter sections.
/// @tparam S section type, e.g. BiQuad
template<typename S, typename Container = std::vector<S>>
class Cascade
{
  public:
    using argument_type = typename S::argument_type;
    using result_type = typename S::result_type;
    using size_type = typename Container::size_type;

    /// Constructor.
    explicit Cascade(size_type n) : _sections(n) {}

    /// Overwrite sections with new content.
    /// @tparam I Iterator type for arguments
    /// @param first Begin iterator
    /// @param last End iterator
    template<typename I>
    void set(I first, I last)
    {
      assert(_sections.size() == size_type(std::distance(first, last)));

      std::copy(first, last, _sections.begin());
    }

    /// Process all sections on single sample.
    /// @param in Input sample
    /// @return Output sample
    result_type operator()(argument_type in)
    {
      for (auto& section: _sections)
      {
        in = section(in);
      }
      return in;
    }

    /// Process all sections on audio block.
    /// @tparam In Iterator type for input samples
    /// @tparam Out Iterator type for output samples
    /// @param first Iterator to first input sample
    /// @param last Iterator to (one past) last input sample
    /// @param result Iterator to first output sample
    template<typename In, typename Out>
    void execute(In first, In last, Out result)
    {
      using out_t = typename std::iterator_traits<Out>::value_type;

      while (first != last)
      {
        *result++ = static_cast<out_t>(this->operator()(*first));
        ++first;
      }
    }

    size_type number_of_sections() const { return _sections.size(); }

  private:
    Container _sections;
};

namespace internal
{

/** Roots-to-polynomial conversion.
 * @tparam T precision of data
 * @param Roots 2x2 roots matrix
 * @param poly1 reference to first-order-coefficient of output polynomial
 * @param poly2 reference to second-order-coefficient of output polynomial
 * @note zeroth-order is ignored for this special case!
 **/
template<typename T>
void roots2poly(T Roots[2][2], T& poly1, T& poly2)
{
  T two = 2.0;

  std::complex<T> eig[2];

  T tmp_arg = std::pow((Roots[0][0]+Roots[1][1])/two, two)
                    + Roots[0][1]*Roots[1][0] - Roots[0][0]*Roots[1][1];

  if (tmp_arg > 0)
  {
    eig[0] = (Roots[0][0]+Roots[1][1])/two + std::sqrt(tmp_arg);
    eig[1] = (Roots[0][0]+Roots[1][1])/two - std::sqrt(tmp_arg);
  }
  else
  {
    eig[0] = std::complex<T>((Roots[0][0]+Roots[1][1])/two, std::sqrt(-tmp_arg));
    eig[1] = std::complex<T>((Roots[0][0]+Roots[1][1])/two, -std::sqrt(-tmp_arg));
  }

  poly1 = real(-eig[0] - eig[1]);
  poly2 = real(-eig[1] * -eig[0]);
}

}  // namespace internal

/** Bilinear transform.
 * @tparam T internal data type
 * @param coeffs_in coefficients of filter design in Laplace domain
 * @param fs sampling rate
 * @param fp prewarping frequency
 * @return coefficients in z-domain
 * @see BiQuad
 **/
template<typename T>
SosCoefficients<T> bilinear(LaplaceCoefficients<T> coeffs_in, int fs, int fp)
{
  SosCoefficients<T> coeffs_out;

  T one = 1.0;
  T two = 2.0;

  // prewarp
  T fp_temp = static_cast<T>(fp) * (two * apf::math::pi<T>());
  T lambda = fp_temp / std::tan(fp_temp / static_cast<T>(fs) / two) / two;

  // calculate state space representation
  T A[2][2] = { { -coeffs_in.a1, -coeffs_in.a2 }, { 1.0, 0.0 } };
  T B[2] = { 1.0, 0.0 };
  T C[2] = { coeffs_in.b1-coeffs_in.a1, coeffs_in.b2-coeffs_in.a2 };
  T D = 1.0;

  T t = one / lambda;
  T r = std::sqrt(t);

  T T1[2][2] = { { (t/two)*A[0][0] + one, (t/two)*A[0][1] },
                 { (t/two)*A[1][0], (t/two)*A[1][1] + one } };
  T T2[2][2] = { { -(t/two)*A[0][0] + one, -(t/two)*A[0][1] },
                 { -(t/two)*A[1][0], -(t/two)*A[1][1] + one} };

  // solve linear equation systems
  T det = T2[0][0]*T2[1][1] - T2[0][1]*T2[1][0];
  T Ad[2][2] = { { (T1[0][0]*T2[1][1] - T1[1][0]*T2[0][1]) / det,
                   (T1[0][1]*T2[1][1] - T1[1][1]*T2[0][1]) / det },
                 { (T1[1][0]*T2[0][0] - T1[0][0]*T2[1][0]) / det,
                   (T1[1][1]*T2[0][0] - T1[0][1]*T2[1][0]) / det } };
  T Bd[2] = { (t/r) * (B[0]*T2[1][1] - B[1]*T2[0][1]) / det,
              (t/r) * (B[1]*T2[0][0] - B[0]*T2[1][0]) / det };
  T Cd[2] = { (C[0]*T2[1][1] - C[1]*T2[1][0]) / det,
              (C[1]*T2[0][0] - C[0]*T2[0][1]) / det };
  T Dd = (B[0]*Cd[0] + B[1]*Cd[1]) * (t/two) + D;

  Cd[0] *= r;
  Cd[1] *= r;

  // convert roots to polynomial
  internal::roots2poly(Ad, coeffs_out.a1, coeffs_out.a2);

  T Tmp[2][2] = { { Ad[0][0]-Bd[0]*Cd[0], Ad[0][1]-Bd[0]*Cd[1] },
                  { Ad[1][0]-Bd[1]*Cd[0], Ad[1][1]-Bd[1]*Cd[1] } };

  internal::roots2poly(Tmp, coeffs_out.b1, coeffs_out.b2);

  coeffs_out.b0 = Dd;
  coeffs_out.b1 += (Dd-one)*coeffs_out.a1;
  coeffs_out.b2 += (Dd-one)*coeffs_out.a2;

  return coeffs_out;
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
