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
/// NetworkSubscriber class (implementation).

#include "ssr_global.h"  // for ERROR()
#include "networksubscriber.h"
#include "apf/stringtools.h"
#include "apf/math.h" // for linear2dB()
#include "connection.h"
#include "legacy_position.h"  // for Position
#include "legacy_orientation.h"  // for Orientation

using apf::str::A2S;

void
ssr::NetworkSubscriber::_send_message(const std::string& str)
{
  _connection.write(str);
}

void
ssr::NetworkSubscriber::_send_source_message(const std::string& first_part
    , id_t id, const std::string& second_part)
{
  auto source_number = _connection.get_source_number(id);
  if (source_number != 0)
  {
    _send_message(first_part + A2S(source_number) + second_part);
  }
  else
  {
    ERROR("Source ID \"" << id << "\" not found");
  }
}

void
ssr::NetworkSubscriber::source_level(id_t id, float level)
{
  _send_source_message("<update><source id='", id
      , "' level='" + A2S(apf::math::linear2dB(level)) + "'/></update>");
}

void
ssr::NetworkSubscriber::delete_source(id_t id)
{
  _send_source_message(
      "<update><delete><source id='", id, "' /></delete></update>");
}


void
ssr::NetworkSubscriber::source_position(id_t id, const Pos& pos)
{
  const Position position{pos};
  _send_source_message("<update><source id='", id, "'><position x='"
    + A2S(position.x) + "' y='" + A2S(position.y) + "'/></source></update>");
}

void
ssr::NetworkSubscriber::source_fixed(id_t id, bool fixed)
{
  _send_source_message("<update><source id='", id, "'><position fixed='"
    + A2S(fixed) + "'/></source></update>");
}

void
ssr::NetworkSubscriber::source_rotation(id_t id, const Rot& rot)
{
  const Orientation orientation{rot};
  _send_source_message("<update><source id='", id,  "'><orientation azimuth='"
    + A2S(orientation.azimuth) + "'/></source></update>");
}

void
ssr::NetworkSubscriber::source_volume(id_t id, float gain)
{
  _send_source_message("<update><source id='", id
      , "' volume='" + A2S(apf::math::linear2dB(gain)) + "'/></update>");
}

void
ssr::NetworkSubscriber::source_mute(id_t id, bool mute)
{
  _send_source_message("<update><source id='", id, "' mute='" + A2S(mute)
    + "'/></update>");
}

void
ssr::NetworkSubscriber::source_model(id_t id, const std::string& model)
{
  _send_source_message("<update><source id='", id, "' model='" + model
    + "'/></update>");
}

void
ssr::NetworkSubscriber::reference_position(const Pos& pos)
{
  const Position position{pos};
  _send_message("<update><reference><position x='" + A2S(position.x)
    + "' y='" + A2S(position.y) + "'/></reference></update>");
}

void
ssr::NetworkSubscriber::reference_rotation(const Rot& rot)
{
  const Orientation orientation{rot};
  _send_message("<update><reference><orientation azimuth='"
    + A2S(orientation.azimuth) + "'/></reference></update>");
}

void
ssr::NetworkSubscriber::reference_offset_position(const Pos& pos)
{
  const Position position{pos};
  _send_message("<update><reference_offset><position x='" + A2S(position.x)
    + "' y='" + A2S(position.y) + "'/></reference_offset></update>");
}

void
ssr::NetworkSubscriber::reference_offset_rotation(const Rot& rot)
{
  const Orientation orientation{rot};
  _send_message("<update><reference_offset><orientation azimuth='"
    + A2S(orientation.azimuth) + "'/></reference_offset></update>");
}

void
ssr::NetworkSubscriber::master_volume(float volume)
{
  _send_message("<update><scene volume='" + A2S(apf::math::linear2dB(volume))    + "'/></update>");
}

void
ssr::NetworkSubscriber::output_activity(id_t id, float* first, float* last)
{
  std::string ms = "<update><source id='" + A2S(id) + "' output_level='";

  for ( ; first != last; ++first)
  {
    ms += A2S(*first);
    ms += " ";
  }
  ms += "'/></update>";
  _send_message(ms);
}
