/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://spatialaudio.net/ssr                           ssr@spatialaudio.net *
 ******************************************************************************/

/// @file
/// Coefficients for the IIR filters in DcaRenderer

#ifndef SSR_DCACASCADE_H
#define SSR_DCACASCADE_H

#include <cmath>  // for std::pow()

#include "apf/biquad.h"
#include "apf/iterator.h"

namespace ssr
{

namespace internal
{

template<typename T> struct LaplaceCoeffsBase;  // no implementation!
template<>
struct LaplaceCoeffsBase<double> { static const double laplace_coeffs[][2]; };
template<>
struct LaplaceCoeffsBase<float> { static const float laplace_coeffs[][2]; };

const double LaplaceCoeffsBase<double>::laplace_coeffs[][2] = {
  #include "laplace_coeffs_double.h"
};
const float LaplaceCoeffsBase<float>::laplace_coeffs[][2] = {
  #include "laplace_coeffs_float.h"
};

}  // namespace internal

/// Coefficients for the IIR filters in DcaRenderer
template<typename T>
class DcaCoefficients : public std::vector<apf::SosCoefficients<T>>
                      , private internal::LaplaceCoeffsBase<T>
{
  private:
    using _base = std::vector<apf::SosCoefficients<T>>;

  public:
    enum source_t { point_source, plane_wave };

    /// Constructor.
    /// @throw std::logic_error if desired order is not supported.
    DcaCoefficients(size_t order, size_t sample_rate, float array_radius
        , float speed_of_sound)
      : _base(order == 0 ? 1 : (order + 1) / 2)  // round up
      // calculate starting index to read Laplace-domain coefficients
      // for formula see http://oeis.org/A002620 (quarter squares)
      // plus 1 for all indices except for order 0
      , _coeffs_begin(!!order + order * order / 4)
      , _sample_rate(sample_rate)
      , _array_radius(array_radius)
      , _speed_of_sound(speed_of_sound)
    {
      if (_coeffs_begin
          >= sizeof(this->laplace_coeffs) / sizeof(*this->laplace_coeffs))
      {
        throw std::logic_error("DcaCoefficients: Order " + apf::str::A2S(order)
            + " is not supported!");
      }
    }

    void reset(float distance, source_t source_type)
    {
      auto iter
        = apf::make_transform_iterator(this->laplace_coeffs + _coeffs_begin
            , Scaler(distance, source_type
              , _sample_rate, _array_radius, _speed_of_sound));

      std::copy(iter, iter + this->size(), this->begin());
    }

    void swap(DcaCoefficients& other)
    {
      // TODO: actually swap this stuff?
      assert(_coeffs_begin == other._coeffs_begin);
      assert(_sample_rate == other._sample_rate);
      assert(_array_radius == other._array_radius);
      assert(_speed_of_sound == other._speed_of_sound);

      this->_base::swap(other);
    }

    friend std::ostream&
    operator<<(std::ostream& stream, const DcaCoefficients& c)
    {
      std::copy(c.begin(), c.end()
          , std::ostream_iterator<typename _base::value_type>(stream, "\n"));
      return stream;
    }

  private:
    class Scaler
    {
      public:
        using result_type = apf::SosCoefficients<T>;

        Scaler(float distance, source_t source_type
            , size_t sample_rate, float array_radius, float speed_of_sound)
          : _scale_factor_one(speed_of_sound / distance)
          , _scale_factor_two(speed_of_sound / array_radius)
          , _source_type(source_type)
          , _sample_rate(sample_rate)
        {}

        result_type operator()(const T row[2]) const
        {
          T two = 2.0;

          auto c = apf::LaplaceCoefficients<T>();

          c.b1 = row[0];
          c.b2 = row[1];
          if (_source_type == point_source)
          {
            c.b1 *= _scale_factor_one;
            c.b2 *= std::pow(_scale_factor_one, two);
          }

          // Note: _scale_factor_two does not depend on the source parameters!
          // Setting a1 and a2 in the constructor might reduce processing time
          c.a1 = row[0] * _scale_factor_two;
          c.a2 = row[1] * std::pow(_scale_factor_two, two);

          // TODO: make prewarping frequency an (optional) parameter?
          return apf::bilinear(c, _sample_rate, 1000);
        }

      private:
        const T _scale_factor_one, _scale_factor_two;
        const source_t _source_type;
        const size_t _sample_rate;
    };

    const size_t _coeffs_begin;
    const size_t _sample_rate;
    const float _array_radius;
    const float _speed_of_sound;
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
