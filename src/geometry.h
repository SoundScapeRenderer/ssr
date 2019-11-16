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
#include <gml/mat.hpp>

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
  if (std::abs(dot(y, up)) > 0.999999f)
  {
    return std::nullopt;
  }
  auto x = cross(y, up);
  x = normalize(x);
  auto z = cross(x, y);
  auto rotation_matrix = gml::mat3x3{x, y, z};
  return gml::qdecomposeRotate(gml::mat4x4{rotation_matrix});
}

}  // namespace ssr

#endif
