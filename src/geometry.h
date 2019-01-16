/******************************************************************************
 * Copyright © 2019 SSR Contributors                                          *
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
/// Data types and helper functions for positions, rotations etc.

#ifndef SSR_GEOMETRY_H
#define SSR_GEOMETRY_H

#include <cmath>  // for std::sin, std::cos, std::sqrt, ...

#include <gml/vec.hpp>
#include <gml/quaternion.hpp>

#include "api.h"  // for Pos and Rot

namespace ssr
{

/// Three-dimensional vector type.  Implicitly convertible to/from Pos.
/// @see quat, https://github.com/ilmola/gml
struct vec3 : gml::vec3
{
  using gml::vec3::vec3;
  using gml::vec3::operator=;
  vec3(const gml::vec3& other) : gml::vec3{other} {}
  vec3(const Pos& pos) : gml::vec3{pos.x, pos.y, pos.z} {}
  operator Pos()
  {
    return {(*this)[0], (*this)[1], (*this)[2]};
  }
};

/// Quaternion type for rotations.  Implicitly convertible to/from Rot.
/// @see vec3, https://github.com/ilmola/gml
struct quat : gml::quat
{
  using gml::quat::quat;
  using gml::quat::operator=;
  quat(const gml::quat& other) : gml::quat{other} {}
  quat(const Rot& rot) : gml::quat{rot.w, gml::vec3{rot.x, rot.y, rot.z}} {}
  operator Rot()
  {
    return {this->imag[0], this->imag[1], this->imag[2], this->real};
  }
};

/// Build a unit quaternion representing the rotation
/// from u to v. The input vectors need not be normalised.
/// From http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
inline quat fromtwovectors(vec3 u, vec3 v)
{
    float norm_u_norm_v = std::sqrt(gml::dot(u, u) * gml::dot(v, v));
    float real_part = norm_u_norm_v + gml::dot(u, v);
    vec3 w;

    if (real_part < 1.e-6f * norm_u_norm_v)
    {
        /* If u and v are exactly opposite, rotate 180 degrees
         * around an arbitrary orthogonal axis. Axis normalisation
         * can happen later, when we normalise the quaternion. */
        real_part = 0.0f;
        w = std::abs(u[0]) > std::abs(u[2]) ? vec3(-u[1],  u[0], 0.f)
                                            : vec3(  0.f, -u[2], u[1]);
    }
    else
    {
        /* Otherwise, build quaternion the standard way. */
        w = gml::cross(u, v);
    }
    return gml::normalize(quat(real_part, w));
}

inline quat look_at(vec3 from, vec3 to)
{
  return fromtwovectors({0.0f, 1.0f, 0.0f}, to - from);
}

}  // namespace ssr

#endif
