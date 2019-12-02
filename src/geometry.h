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

#include <optional>
#include <cmath>  // for std::asin(), std::atan2()

#include <gml/vec.hpp>
#include <gml/quaternion.hpp>
#include <gml/mat.hpp>
#include <gml/util.hpp>  // for gml::degrees(), gml::radians()

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
  operator Pos() const
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
  operator Rot() const
  {
    return {this->imag[0], this->imag[1], this->imag[2], this->real};
  }
};

inline quat normalize(const quat& q)
{
  return gml::normalize(q);
}

/// See https://AudioSceneDescriptionFormat.readthedocs.io/quaternions.html
inline quat angles2quat(float azimuth, float elevation, float roll)
{
  return normalize(
    gml::qrotate(gml::radians(azimuth),   {0.0f, 0.0f, 1.0f}) *
    gml::qrotate(gml::radians(elevation), {1.0f, 0.0f, 0.0f}) *
    gml::qrotate(gml::radians(roll),      {0.0f, 1.0f, 0.0f}));
}

/// See https://AudioSceneDescriptionFormat.readthedocs.io/quaternions.html
inline std::array<float, 3> quat2angles(const Rot& rot)
{
  // NB: Rot uses xyzw convention, math derivations use abcd (= wxyz):
  auto [b, c, d, a] = rot;
  auto sin_elevation = 2 * (a * b + c * d);
  if (0.999999f < sin_elevation)
  {
    // elevation ~= 90°
    return {
      gml::degrees(std::atan2(2 * (a * c + b * d), 2 * (a * b - c * d))),
      90.0f,
      0.0f};
  }
  else if (sin_elevation < -0.999999f)
  {
    // elevation ~= -90°
    return {
      gml::degrees(std::atan2(-2 * (a * c + b * d), 2 * (c * d - a * b))),
      -90.0f,
      0.0f
    };
  }
  return {
    gml::degrees(std::atan2(2 * (a * d - b * c), 1 - 2 * (b*b + d*d))),
    gml::degrees(std::asin(sin_elevation)),
    gml::degrees(std::atan2(2 * (a * c - b * d), 1 - 2 * (b*b + c*c)))
  };
}

inline std::optional<quat> look_rotation(vec3 from, vec3 to)
{
  auto y = to - from;
  auto y_length = length(y);
  if (y_length < 0.000001f)
  {
    return std::nullopt;
  }
  y /= y_length;
  vec3 up{0.0f, 0.0f, 1.0f};
  if (0.999999f < std::abs(dot(y, up)))
  {
    return std::nullopt;
  }
  auto x = cross(y, up);
  x = normalize(x);
  auto z = cross(x, y);
  auto rotation_matrix = gml::mat3x3{x, y, z};
  return normalize(gml::qdecomposeRotate(gml::mat4x4{rotation_matrix}));
}

}  // namespace ssr

#endif
