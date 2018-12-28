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
/// %Source class (definition).

#ifndef SSR_SOURCE_H
#define SSR_SOURCE_H

#include <vector>
#include <string>
#include <istream> // for operator>>

#include "directionalpoint.h"

/// Class for saving source information
struct Source : DirectionalPoint
{
  /** %Source type.
   * This enum can be extended arbitrarily, but for starters only point source
   * and plane wave will be implemented.
   **/
  using model_t = enum
  {
    unknown = 0, /// unknown source model
    point,       ///< point source
    plane,       ///< plane wave
    line,        ///< line source (not implemented!)
    directional,  ///< directional source (not implemented!)
    extended     ///< spatially extended source (not implemented!)
  };

  /// ctor.
  /// @param position position
  /// @param orientation orientation
  explicit Source(size_t outputs,
      const Position&    position    = Position(),
      const Orientation& orientation = Orientation()) :
    DirectionalPoint(position, orientation), // base class ctor
    audio_file_channel(0),
    file_length(0),
    model(Source::point),
    mute(0),
    gain(1.0),
    signal_level(0.0),
    output_levels(outputs),
    has_mirror_sources(false),
    fixed_position(false),
    properties_file(""),
    doppler_effect(false)
  {}

  // TODO: get rid of output levels? remove one of the constructors?
  explicit Source(
      const Position&    position    = Position(),
      const Orientation& orientation = Orientation()) :
    DirectionalPoint(position, orientation), // base class ctor
    audio_file_channel(0),
    file_length(0),
    model(Source::point),
    mute(0),
    gain(1.0),
    signal_level(0.0),
    has_mirror_sources(false),
    fixed_position(false),
    properties_file(""),
    doppler_effect(false)
  {}

  std::string name;               ///< source name
  std::string audio_file_name;    ///< audio file
  int audio_file_channel;         ///< channel of audio file
  std::string port_name;          ///< JACK port name
  long int file_length;           ///< length of audio file, 0 if no file exists
  model_t model;                  ///< source model (=type)
  bool mute;                      ///< mute/unmute
  float gain;                     ///< source gain (volume)
  float signal_level;             ///< instantaneous level of audio stream (linear scale, between 0 and 1)
  std::vector<float> output_levels;
  bool has_mirror_sources;        ///< see Scene::mirror_order
  bool fixed_position;            ///< static position or not
  std::string properties_file;    ///< path to file where BRIRs reside (if available)
  bool doppler_effect;            ///< doppler effect enabled

  friend std::istream& operator>>(std::istream& input, model_t& model)
  {
    std::string temp;
    input >> temp;
    //if (input.fail()) return input; // redundant?
    if (temp == "point") model = point;
    else if (temp == "plane") model = plane;
    else if (temp == "line") model = line;
    else if (temp == "directional") model = directional;
    // everything else (including empty string on failure) doesn't change model
    else input.setstate(std::ios_base::badbit);
    return input;
  }

  friend std::ostream& operator<<(std::ostream& output, const model_t& model)
  {
    switch (model)
    {
      case point:
        output << "point";
        break;
      case plane:
        output << "plane";
        break;
      case line:
        output << "line";
        break;
      case directional:
        output << "directional";
        break;
      default:
        output << "unknown";
        break;
    }
    return output;
  }
};

#endif
