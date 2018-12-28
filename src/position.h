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
/// %Position class and helper functions (definition).

#ifndef SSR_POSITION_H
#define SSR_POSITION_H

#include "orientation.h"

/** Geometric representation of a position.
 * Stores the position of a point in space and provides some helper functions.
 * If you want to speak in design patterns, you could call this a "Messenger"
 * patter. It's the most trivial of all patterns. So maybe it's not even worth
 * mentioning. But I did it anyway ...
 * @warning For now, it only uses 2 dimensions (x,y) but a z coordinate can be
 * added later, if needed.
 **/
struct Position
{
  /** with no arguments, all member variables are initialized to zero.
   * @param x x coordinate (in meters)
   * @param y y coordinate (in meters)
   **/
  explicit Position(const float x = 0, const float y = 0);

  float x; ///< x coordinate (in meters)
  float y; ///< y coordinate (in meters)

  /// length of the position vector
  float length() const;

  /// turn around the origin
  Position& rotate(float angle);
  Position& rotate(const Orientation& rotation);

  Orientation orientation() const;

  Position& operator+=(const Position& other); ///< += operator
  Position& operator-=(const Position& other); ///< -= operator
  bool operator==(const Position& other) const;  ///< == operator
  bool operator!=(const Position& other) const;  ///< != operator

  // Declaring the following operators as friend is not really necessary as
  // their fields are public anyway, but it doesn't hurt either.

  /// plus (+) operator
  friend Position operator+(const Position& a, const Position& b);
  /// minus (-) operator
  friend Position operator-(const Position& a, const Position& b);
  /// unary minus (-) operator
  friend Position operator-(const Position& a);
  /// output stream operator (<<)
  friend std::ostream& operator<<(std::ostream& stream,
      const Position& position);

  /** division (/) operator.
   * @param a dividend, a DirectionalPoint.
   * @param b divisor, any numeric Type..
   * @return quotient.
   **/
  template <typename T>
  friend Position operator/(const Position& a, const T& b)
  {
    return Position(a.x / b, a.y / b);
  }
};

/// Calculate the angle between the position vector of @a point and the
/// orientation @a orientation.
float angle(const Position& point, const Orientation& orientation);
// TODO: declare angle() also as friend of Position?

#endif
