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
/// Legacy loudspeaker class (definition).  Superseded by Loudspeaker.

#ifndef SSR_LEGACY_LOUDSPEAKER_H
#define SSR_LEGACY_LOUDSPEAKER_H

#include <string>
#include <vector>

#include "api.h"  // for Loudspeaker
#include "legacy_directionalpoint.h"

/// Class for saving loudspeaker information.
struct LegacyLoudspeaker : DirectionalPoint
{
  typedef std::vector<LegacyLoudspeaker> container_t;

  /// %Loudspeaker type.
  enum model_t
  {
    unknown = 0, ///< unknown loudspeaker type
    // TODO: find better names and better descriptions
    normal,      ///< normal loudspeaker
    subwoofer    ///< always on, regardless of source positions
  };

  /// ctor
  explicit LegacyLoudspeaker(
      const DirectionalPoint& point = DirectionalPoint(),
      model_t model = normal, float weight = 1.0f, float delay = 0.0f) :
    DirectionalPoint(point), // base class copy ctor
    model(model),
    weight(weight),
    delay(delay)
  {}

  /// Constructor from "modern" Loudspeaker.
  LegacyLoudspeaker(const ssr::Loudspeaker& other)
    : LegacyLoudspeaker{{other.position, other.rotation}
        , other.model == "subwoofer" ? subwoofer : normal}
  {}

  operator ssr::Loudspeaker()
  {
    return {this->position, this->orientation
      , this->model == subwoofer ? "subwoofer" : ""};
  }

  model_t model; ///< type of loudspeaker
  float weight;   /// linear!
  float delay;   /// in seconds
  bool mute{false};     ///< mute/unmute

  friend std::istream& operator>>(std::istream& input, model_t& model)
  {
    std::string temp;
    input >> temp;
    //if (input.fail()) return input; // redundant?
    if (temp == "normal") model = normal;
    else if (temp == "subwoofer") model = subwoofer;
    // everything else (including empty string on failure) doesn't change model
    else input.setstate(std::ios_base::badbit);
    return input;
  }

  friend std::ostream& operator<<(std::ostream& output, const model_t& model)
  {
    switch (model)
    {
      case normal:
        output << "normal";
        break;
      case subwoofer:
        output << "subwoofer";
        break;
      default:
        output << "unknown";
        break;
    }
    return output;
  }
};

#endif
