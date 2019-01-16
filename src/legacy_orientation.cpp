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
/// Legacy 2D %Orientation class and helper function(s) (implementation).

#include <ostream>

#include "legacy_orientation.h"
#include "apf/math.h"
#include "ssr_global.h"

namespace
{
  // Convert 2D azimuth in radians to 3D rotation.
  ssr::Rot azi2quat(float azimuth)
  {
    return {0, 0, std::sin(azimuth / 2), std::cos(azimuth / 2)};
  }

  // Convert 3D rotation to 2D azimuth in radians.
  // NB: This doesn't take care of Gimbal Lock!
  float quat2azi(ssr::Rot rot)
  {
    auto [x, y, z, w] = rot;
    return std::atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));
  }
}

/// ctor. @param azimuth azimuth (in degrees)
Orientation::Orientation(const float azimuth) :
  azimuth(azimuth)
{}

/// Convert from 3D rotation.
Orientation::Orientation(const ssr::Rot& three_d_rot) :
  azimuth(apf::math::rad2deg(quat2azi(three_d_rot)) + 90.0f)
{}

Orientation::operator ssr::Rot()
{
  return azi2quat(apf::math::deg2rad(this->azimuth - 90.0f));
}

/** - operator.
 * @return difference of Orientations
 **/
Orientation operator-(const Orientation& lhs, const Orientation& rhs)
{
  return Orientation(lhs.azimuth - rhs.azimuth);
}

/** + operator.
 **/
Orientation operator+(const Orientation& lhs, const Orientation& rhs)
{
  return Orientation(lhs.azimuth + rhs.azimuth);
}

/** unary -operator.
 * @return negative Orientation
 **/
Orientation operator-(const Orientation& rhs)
{
  return Orientation(-rhs.azimuth);
}

/** += operator.
 * @param other addend.
 * @return sum of Orientations
 **/
Orientation& Orientation::operator+=(const Orientation& other)
{
  azimuth += other.azimuth;
  return *this;
}

/** -= operator.
 * @param other minuend.
 * @return difference of Orientations
 **/
Orientation& Orientation::operator-=(const Orientation& other)
{
  azimuth -= other.azimuth;
  return *this;
}

/** ._
 * @param angle angle in degrees.
 * @return the resulting orientation
 **/
Orientation& Orientation::rotate(float angle)
{
  this->azimuth += angle;
  return *this;
}

// this is only a 2D implementation!
Orientation& Orientation::rotate(const Orientation& rotation)
{
  return this->rotate(rotation.azimuth);
}

/** _.
 * @param a One orientation
 * @param b Another orientation
 * @return Angle between the two orientations in radians. If the angle of @a b
 * is bigger than the angle of @a a, the result is negative.
 * @warning 2D implementation!
 **/
float angle(const Orientation& a, const Orientation& b)
{
  return apf::math::deg2rad(a.azimuth - b.azimuth);
}

/// output stream operator (<<)
std::ostream& operator<<(std::ostream& stream, const Orientation& orientation)
{
  stream << "azimuth = " << orientation.azimuth;
  return stream;
}
