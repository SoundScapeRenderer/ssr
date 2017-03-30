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
/// %Loudspeaker class (definition).

#ifndef SSR_LOUDSPEAKER_H
#define SSR_LOUDSPEAKER_H

#include <string>
#include <vector>

#include "directionalpoint.h"

/// Class for saving loudspeaker information.
struct Loudspeaker : DirectionalPoint
{
  typedef std::vector<Loudspeaker> container_t;

  /// %Loudspeaker type.
  enum model_t
  {
    unknown = 0, ///< unknown loudspeaker type
    // TODO: find better names and better descriptions
    normal,      ///< normal loudspeaker
    subwoofer    ///< always on, regardless of source positions
  };

  /// ctor
  explicit Loudspeaker(
      const DirectionalPoint& point = DirectionalPoint(),
      model_t model = normal, float weight = 1.0f, float delay = 0.0f) :
    DirectionalPoint(point), // base class copy ctor
    model(model),
    weight(weight),
    delay(delay),
    mute(false),
    active(false)
  {}

  // auto-generated copy ctor and assignment operator are OK.

  //std::string name;
  //std::string audio_file_name;
  //std::string audio_file_channel;
  //std::string port_name;
  model_t model; ///< type of loudspeaker
  float weight;   /// linear!
  float delay;   /// in seconds
  bool mute;     ///< mute/unmute
  bool active;   ///< active/inactive for a given source
  //float gain;    ///< gain

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

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
