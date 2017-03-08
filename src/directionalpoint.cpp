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
/// Geometry information of a point including an orientation (implementation).

#include <ostream>
#include <cmath> // for cos()

#include "directionalpoint.h"

/** ctor.
 * @param position position
 * @param orientation orientation
 **/
DirectionalPoint::DirectionalPoint(const Position& position,
    const Orientation& orientation) :
  position(position),
  orientation(orientation)
{}

/** += operator
 * @param other addend
 * @return reference to itself after adding @a other
 **/
DirectionalPoint& DirectionalPoint::operator+=(const DirectionalPoint& other)
{
  position    += other.position;
  orientation += other.orientation;
  return *this;
}

/** -= operator
 * @param other minuend
 * @return reference to itself after subtracting @a other
 **/
DirectionalPoint& DirectionalPoint::operator-=(const DirectionalPoint& other)
{
  position    -= other.position;
  orientation -= other.orientation;
  return *this;
}

/** Distance between plane and point.
 * \par Algorithm
 * The plane is defined by the DirectionalPoint (Position and Orientation)
 * properties of *this (=the current object) where its Position is the point
 * \f$\displaystyle\mathbf{p}={p_x \choose p_y}\f$ somewhere on
 * the plane and its Orientation is its normal vector \f$\mathbf{n}\f$.
 * The point we want to get the distance to is given by
 * \f$\displaystyle\mathbf{r}={r_x \choose r_y}\f$.
 * The distance \f$l\f$ is calculated as the inner product of the vector from
 * \f$\mathbf{p}\f$ to \f$\mathbf{r}\f$ with the normal vector \f$\mathbf{n}\f$.
 * \par
 * \f$ l = (\mathbf{r} - \mathbf{p}) \cdot \mathbf{n} \f$
 * \par
 * Note that the distance \f$l\f$ can be negative, if the point \f$\mathbf{r}\f$
 * is in the half-space opposite to the normal vector.
 * \par
 * Because we have the normal vector only as an angle (its length is irrelevant
 * to us), we calculate the inner
 * product as the product of the absolute values and the angle between the two
 * vectors. Therefore, the distance is given as
 * \f$ l = |\mathbf{r} - \mathbf{p}| \cos\phi \f$, where \f$\phi\f$ is the angle
 * between \f$ (\mathbf{r} - \mathbf{p}) \f$ and \f$ \mathbf{n} \f$.
 *
 * \par finally, the equation:
 * \f$ \displaystyle l = \sqrt{(r_x-p_x)^2+(r_y-p_y)^2}
 * \cos\left(\mathop{\mathrm{atan}}\left(\frac{r_y-p_y}{r_x-p_x}\right) -
 * \phi_\mathbf{n}\right) \f$
 * \par
 * The order of the subtrahends doesn't matter because the cosine function is
 * symmetric.
 *
 * \par Alternative
 * The distance could also be calculated with the use of the "sine rule"
 * (de:Sinussatz). I don't know which one is computationally more efficient.
 *
 * @param point The distance to this point is returned
 * @return The distance between a plane (represented by the current object) and
 * @a point (in meters).
 * @warning This is only a 2D implementation! The z-value, elevation and tilt
 * are ignored.
 **/
float DirectionalPoint::plane_to_point_distance(const Position& point) const
{
  Position temp = point - this->position;
  // the order of the arguments to angle() doesn't matter because cos()
  // is symmetric.
  return temp.length() * cos(angle(this->orientation,temp.orientation()));
}

/** ._
 * @param angle angle in degrees.
 * @return the resulting point
 * @warning only possible for 2D! For a more general application use the
 * overloaded member function rotate(const Orientation&).
 **/
DirectionalPoint& DirectionalPoint::rotate(float angle)
{
  this->position.rotate(angle);
  this->orientation.rotate(angle);
  return *this;
}

DirectionalPoint& DirectionalPoint::rotate(const Orientation& rotation)
{
  this->position.rotate(rotation);
  this->orientation.rotate(rotation);
  return *this;
}

DirectionalPoint& DirectionalPoint::transform(const DirectionalPoint& t)
{
  this->position.rotate(t.orientation);
  this->position += t.position;
  this->orientation.rotate(t.orientation);
  return *this;
}

/** _.
 * @param a
 * @param b
 * @return Angle in radians between the two Orientations.
 **/
float angle(const DirectionalPoint& a, const DirectionalPoint& b)
{
  return angle(a.orientation, b.orientation);
}

/** plus (+) operator.
 * @param a one addend.
 * @param b the other one.
 * @return sum
 **/
DirectionalPoint operator+(const DirectionalPoint& a, const DirectionalPoint& b)
{
  DirectionalPoint temp(a);
  return temp += b;
}

/** minus (-) operator.
 * @param a subtrahend.
 * @param b minuend.
 * @return difference
 **/
DirectionalPoint operator-(const DirectionalPoint& a, const DirectionalPoint& b)
{
  DirectionalPoint temp(a);
  return temp -= b;
}

/** output stream operator (<<).
 * @param stream the stream
 * @param point the point you want to throw into the stream
 * @return a reference to the stream
 **/
std::ostream& operator<<(std::ostream& stream, const DirectionalPoint& point)
{
  stream << point.position << ", " << point.orientation;
  return stream;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
