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
/// Legacy 2D %Position class and helper functions (implementation).

#include <cmath> // for atan2(), sqrt()
#include <ostream>

#include "legacy_position.h"
#include "legacy_orientation.h"
#include "apf/math.h"

Position::Position(const float x, const float y) :
  x(x),
  y(y)
{}

Position::Position(const ssr::Pos& three_d_pos) :
  x(three_d_pos.x),
  y(three_d_pos.y)
{}

Position::operator ssr::Pos()
{
  return {this->x, this->y};
}

Position& Position::operator+=(const Position& other)
{
  x += other.x;
  y += other.y;
  return *this;
}

Position& Position::operator-=(const Position& other)
{
  x -= other.x;
  y -= other.y;
  return *this;
}

bool Position::operator==(const Position& other) const
{
  return x == other.x && y == other.y;
}

bool Position::operator!=(const Position& other) const
{
  return !this->operator==(other);
}

/** convert the orientation given by the position vector (x,y) to an
 * Orientation.
 * @return Orientation with the corresponding azimuth value
 * @warning Works only for 2D!
 **/
Orientation Position::orientation() const
{
  return Orientation(atan2(y, x) / apf::math::pi_div_180<float>());
}

float Position::length() const
{
  return sqrt(apf::math::square(x) + apf::math::square(y));
}

/** ._
 * @param angle angle in degrees.
 * @return the resulting position
 **/
Position& Position::rotate(float angle)
{
  // angle phi in radians!
  float phi = apf::math::deg2rad(this->orientation().azimuth + angle);
  float radius = this->length();
  return *this = Position(radius * cos(phi), radius * sin(phi));
}

// this is a 2D implementation!
Position& Position::rotate(const Orientation& rotation)
{
  return this->rotate(rotation.azimuth);
}

Position operator-(const Position& a, const Position& b)
{
  Position temp(a);
  return temp -= b;
}

Position operator+(const Position& a, const Position& b)
{
  Position temp(a);
  return temp += b;
}

Position operator-(const Position& a)
{
  return Position(-a.x, -a.y);
}

/** _.
 * @param point
 * @param orientation
 * @return Angle in radians.
 **/
float angle(const Position& point, const Orientation& orientation)
{
  return angle(point.orientation(), orientation);
}

std::ostream& operator<<(std::ostream& stream, const Position& position)
{
  stream << "x = " << position.x << ", y = " << position.y;
  return stream;
}
