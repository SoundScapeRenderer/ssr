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
/// Legacy scene class (implementation), superseded by Scene.

#include <cassert>

#include "legacy_scene.h"
#include "legacy_source.h"

ssr::LegacyScene::LegacyScene() :
  // reference looking straight ahead (in positive y-direction)
  _reference(Position(0, 0), Orientation(90)),
  _reference_offset(Position(0, 0), Orientation(0)),
  _master_volume(1.0f),
  _decay_exponent(1.0f),
  _amplitude_reference_distance(3.0f),
  _master_signal_level(0.0f),
  _cpu_load(0.0f), _sample_rate(0u),
  _processing_state(true),
  _auto_rotate_sources(true)
{}

ssr::LegacyScene::loudspeakers_t::size_type ssr::LegacyScene::get_number_of_loudspeakers() const
{
  return _loudspeakers.size();
}


/// _. @return processing state
bool ssr::LegacyScene::get_processing_state() const
{
  return _processing_state;
}

/// _. @return master volume
float ssr::LegacyScene::get_master_volume() const
{
  return _master_volume;
}

/// _. @return amplitude decay exponent
float ssr::LegacyScene::get_decay_exponent() const
{
  return _decay_exponent;
}

/// _. @return amplitude reference distance
float ssr::LegacyScene::get_amplitude_reference_distance() const
{
  return _amplitude_reference_distance;
}

/// _. @return instantaneous overall audio signal level
float ssr::LegacyScene::get_master_signal_level() const
{
  return _master_signal_level;
}

/// _. @return CPU load in percent
float ssr::LegacyScene::get_cpu_load() const
{
  return _cpu_load;
}

/// _. @return current sample rate
int ssr::LegacyScene::get_sample_rate() const
{
  return _sample_rate;
}


bool ssr::LegacyScene::is_playing() const
{
  //return _transport_state.playing;
  return _transport_playing;
}

uint32_t ssr::LegacyScene::get_transport_position() const
{
  //return _transport_state.position;
  return _transport_position;
}

bool ssr::LegacyScene::get_auto_rotation() const
{
  return _auto_rotate_sources;
}

LegacySource ssr::LegacyScene::get_source(legacy_id_t id) const
{
  auto source = LegacySource(_loudspeakers.size());
  auto source_ptr = maptools::get_item(_source_map, id);

  if (!source_ptr) ERROR("Source " << id << " doesn't exist!");
  else source = *source_ptr;

  return source;
}

/** _.
 * @param id ID of the source
 * @return position of the source
 * @warning If @a id is not found, a unique_ptr to NULL is returned!
 **/
std::unique_ptr<Position> ssr::LegacyScene::get_source_position(legacy_id_t id) const
{
  std::unique_ptr<Position> position(new Position); // standard ctor
  if (_get_source_member(id, &LegacySource::position, *position)) return position;
  position.reset(); // set to NULL
  return position;
}

/** _.
 * @param id ID of the source
 * @return position of the source
 * @warning If @a id is not found, a unique_ptr to NULL is returned!
 **/
std::unique_ptr<Orientation> ssr::LegacyScene::get_source_orientation(legacy_id_t id) const
{
  std::unique_ptr<Orientation> orientation(new Orientation); // standard ctor
  if (_get_source_member(id, &LegacySource::orientation, *orientation)) return orientation;
  orientation.reset(); // set to NULL
  return orientation;
}

/** _.
 * @param id ID of the source
 * @return source model
 * @warning If @a id is not found, LegacySource::unknown is returned
 **/
LegacySource::model_t ssr::LegacyScene::get_source_model(legacy_id_t id) const
{
  LegacySource::model_t model;
  if (_get_source_member(id, &LegacySource::model, model)) return model;
  return LegacySource::unknown;
}

/** _.
 * @param id ID of the source
 * @return linear source gain (=volume)
 * @warning If @a id is not found, 0 is returned
 **/
float ssr::LegacyScene::get_source_gain(legacy_id_t id) const
{
  float gain;
  if (_get_source_member(id, &LegacySource::gain, gain)) return gain;
  return 0;
}

/** _.
 * @param id ID of the source
 * @return mute state
 * @warning If @a id is not found, false is returned
 **/
bool ssr::LegacyScene::get_source_mute_state(legacy_id_t id) const
{
  bool state;
  if (_get_source_member(id, &LegacySource::mute, state)) return state;
  return false;
}

/** _.
 * @param id ID of the source
 * @return source name
 * @warning If @a id is not found, an empty string is returned
 **/
std::string ssr::LegacyScene::get_source_name(legacy_id_t id) const
{
  std::string name;
  if (_get_source_member(id, &LegacySource::name, name)) return name;
  return std::string("");
}

bool ssr::LegacyScene::get_source_position_fixed(legacy_id_t id) const
{
  bool boolean;
  if (_get_source_member(id, &LegacySource::fixed_position, boolean)) return boolean;

  return false;
}

std::string ssr::LegacyScene::get_source_properties_file(legacy_id_t id) const
{
  std::string name;
  if (_get_source_member(id, &LegacySource::properties_file, name)) return name;
  return std::string("");
}
