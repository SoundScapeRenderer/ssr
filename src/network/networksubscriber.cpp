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

#include "networksubscriber.h"
#include "apf/stringtools.h"
#include "apf/math.h" // for linear2dB()
#include "connection.h"

using apf::str::A2S;

// temporary hack:
//static bool previous_state = false;

ssr::NetworkSubscriber::NetworkSubscriber(Connection &connection)
  : _connection(connection)
  , _master_level(0.0)
{}

ssr::NetworkSubscriber::~NetworkSubscriber() {}

void
ssr::NetworkSubscriber::update_all_clients(std::string str)
{
  _connection.write(str);
}

void
ssr::NetworkSubscriber::send_levels()
{
  source_level_map_t::iterator i;
  std::string ms = "<update>";
  for (i=_source_levels.begin(); i!=_source_levels.end(); i++)
  {
    ms += ("<source id='" + A2S((*i).first) + "' level='"
        + A2S(apf::math::linear2dB((*i).second)) + "'/>");
  }
  ms += "</update>";
  update_all_clients(ms);
}

// Subscriber interface

void
ssr::NetworkSubscriber::set_loudspeakers(
    const Loudspeaker::container_t& loudspeakers)
{
  (void) loudspeakers;
  //not_implemented("NetworkSubscriber::set_loudspeakers()");
}

void
ssr::NetworkSubscriber::new_source(id_t id)
{
  std::string ms = "<update><source id='" + A2S(id) + "'/></update>";
  update_all_clients(ms);
}

void
ssr::NetworkSubscriber::delete_source(id_t id)
{
  _source_levels.erase(id);
  std::string ms = "<update><delete><source id='" + A2S(id) + "' />" +
    + "</delete></update>";
  update_all_clients(ms);
}

void
ssr::NetworkSubscriber::delete_all_sources()
{
  _source_levels.clear();
  std::string ms = "<update><delete><source id='0'/></delete></update>";
  update_all_clients(ms);
}

bool
ssr::NetworkSubscriber::set_source_position(id_t id, const Position& position)
{
  std::string ms = "<update><source id='" + A2S(id) + "'><position x='"
    + A2S(position.x) + "' y='" + A2S(position.y) + "'/></source></update>";
  update_all_clients(ms);
  return true;
}

bool
ssr::NetworkSubscriber::set_source_position_fixed(id_t id, const bool& fixed)
{
  std::string ms = "<update><source id='" + A2S(id) + "'><position fixed='"
    + A2S(fixed) + "'/></source></update>";
  update_all_clients(ms);
  return true;
}

bool
ssr::NetworkSubscriber::set_source_orientation(id_t id
    , const Orientation& orientation)
{
  std::string ms = "<update><source id='" + A2S(id) + "'><orientation azimuth='"
    + A2S(orientation.azimuth) + "'/></source></update>";
  update_all_clients(ms);
  return true;
}

bool
ssr::NetworkSubscriber::set_source_gain(id_t id, const float& gain)
{
  std::string ms = "<update><source id='"
    + A2S(id) + "' volume='" + A2S(apf::math::linear2dB(gain)) + "'/></update>";
  update_all_clients(ms);
  return true;
}

bool
ssr::NetworkSubscriber::set_source_mute(id_t id, const bool& mute)
{
  std::string ms = "<update><source id='" + A2S(id) + "' mute='" + A2S(mute)
    + "'/></update>";
  update_all_clients(ms);
  return true;
}

bool
ssr::NetworkSubscriber::set_source_name(id_t id, const std::string& name)
{
  (void) id;
  (void) name;
#if 0
  std::string ms = "<update><source id='" + A2S(id) + "' name='" +
    _xmlparser.replace_entities(name) + "' /></update>";
  update_all_clients(ms);
#endif
  return true;
}

bool
ssr::NetworkSubscriber::set_source_properties_file(id_t id, const std::string& name)
{
  (void) id;
  (void) name;
#if 0
  std::string ms = "<update><source id='" + A2S(id) + "' properties_file='" +
    _xmlparser.replace_entities(name) + "' /></update>";
  update_all_clients(ms);
#endif
  return true;
}

void
ssr::NetworkSubscriber::set_decay_exponent(float exponent)
{
  (void) exponent;
  //not_implemented("NetworkSubscriber::set_decay_exponent()");
  return;
}

void
ssr::NetworkSubscriber::set_amplitude_reference_distance(float distance)
{
  (void) distance;
  //not_implemented("NetworkSubscriber::set_amplitude_reference_distance()");
  return;
}

bool
ssr::NetworkSubscriber::set_source_model(id_t id, const Source::model_t& model)
{
  std::string tmp;
  tmp = A2S(model);
  if (tmp == "") return false;

  std::string ms = "<update><source id='" + A2S(id) + "' model='" + tmp
    + "'/></update>";
  update_all_clients(ms);
  return true;
}

bool
ssr::NetworkSubscriber::set_source_port_name(id_t id, const std::string& port_name)
{
  (void) id;
  (void) port_name;
#if 0
  std::string ms = "<update><source id='" + A2S(id) +
    "' port_name='" + _xmlparser.replace_entities(port_name) + "'/></update>";
  update_all_clients(ms);
#endif
  return true;
}

bool
ssr::NetworkSubscriber::set_source_file_name(id_t id, const std::string& file_name)
{
  (void) id;
  (void) file_name;
  return 1;
}

bool
ssr::NetworkSubscriber::set_source_file_channel(id_t id
    , const int& file_channel)
{
  (void) id;
  (void) file_channel;
  return 1;
}

bool
ssr::NetworkSubscriber::set_source_file_length(id_t id, const long int& length)
{
  std::string ms = "<update><source id='" + A2S(id) + "' file_length='"
    + A2S(length) + "'/></update>";
  update_all_clients(ms);
  return true;
}

void
ssr::NetworkSubscriber::set_reference_position(const Position& position)
{
  std::string ms = "<update><reference><position x='" + A2S(position.x)
    + "' y='" + A2S(position.y) + "'/></reference></update>";
  update_all_clients(ms);
}

void
ssr::NetworkSubscriber::set_reference_orientation(const Orientation& orientation)
{
  std::string ms = "<update><reference><orientation azimuth='"
    + A2S(orientation.azimuth) + "'/></reference></update>";
  update_all_clients(ms);
}

void
ssr::NetworkSubscriber::set_reference_offset_position(const Position& position)
{
  std::string ms = "<update><reference_offset><position x='" + A2S(position.x)
    + "' y='" + A2S(position.y) + "'/></reference_offset></update>";
  update_all_clients(ms);
}

void
ssr::NetworkSubscriber::set_reference_offset_orientation(const Orientation& orientation)
{
  std::string ms = "<update><reference_offset><orientation azimuth='"
    + A2S(orientation.azimuth) + "'/></reference_offset></update>";
  update_all_clients(ms);
}

void
ssr::NetworkSubscriber::set_master_volume(float volume)
{
  std::string ms = "<update><scene volume='" + A2S(apf::math::linear2dB(volume))    + "'/></update>";
  update_all_clients(ms);
}

void
ssr::NetworkSubscriber::set_source_output_levels(id_t id, float* first
    , float* last)
{
  std::string ms = "<update><source id='" + A2S(id) + "' output_level='";

  for ( ; first != last; ++first)
  {
    ms += A2S(*first);
    ms += " ";
  }
  ms += "'/></update>";
  update_all_clients(ms);
}

void
ssr::NetworkSubscriber::set_processing_state(bool state)
{
  (void) state;
}

void
ssr::NetworkSubscriber::set_transport_state(
    const std::pair<bool, jack_nframes_t>& state)
{
  // temporary hack: only start/stop is forwarded, the "time" in samples is
  // ignored
  static bool previous_state = false;
  if (previous_state != state.first)
  {
    std::string ms = "<update><state transport='";
    ms += state.first?"start":"stop";
    ms += "'/></update>";
    update_all_clients(ms);
    previous_state = state.first;
  }
}

void 
ssr::NetworkSubscriber::set_auto_rotation(bool auto_rotate_sources)
{
  (void) auto_rotate_sources;
}

void
ssr::NetworkSubscriber::set_cpu_load(float load)
{
  (void)load;
  // temporarily disabled:
  /*
  std::string ms = "<update><cpu load='" + A2S(load) + "'/></update>";
  update_all_clients(ms);
  */
}

void
ssr::NetworkSubscriber::set_sample_rate(int sr)
{
  (void)sr;
}

void
ssr::NetworkSubscriber::set_master_signal_level(float level)
{
  _master_level = level;
  //std::string ms = "<update><master level='" + A2S(level) +
  //  "'/></update>";
  //update_all_clients(ms);
}

bool
ssr::NetworkSubscriber::set_source_signal_level(const id_t id, const float& level)
{
  _source_levels[id] = level;
  std::string ms = "<update><source id='" + A2S(id) + "' level='" + A2S(level)
    + "'/></update>";
  //update_all_clients(ms);
  return true;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
