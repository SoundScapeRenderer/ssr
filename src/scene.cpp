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
/// %Scene class (implementation).

#include <cassert>

#include "scene.h"
#include "source.h"

ssr::Scene::Scene() :
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

ssr::Scene::~Scene()
{
  //maptools::purge(_source_map);
}

ssr::Scene::loudspeakers_t::size_type ssr::Scene::get_number_of_loudspeakers() const
{
  return _loudspeakers.size();
}

void ssr::Scene::set_loudspeakers(const Loudspeaker::container_t& loudspeakers)
{
  // mind the difference between _loudspeakers and loudspeakers!
  if (_loudspeakers.size() != 0)
  {
    ERROR("BUG: Loudspeaker list can only be set once for a Scene!");
    _loudspeakers.clear();
  }
  // reserve memory to avoid unnecessary re-allocation
  _loudspeakers.reserve(loudspeakers.size());
  for (const auto& ls: loudspeakers)
  {
    // construction of temporary variable with type conversion ctor
    // and copy construction into vector
    _loudspeakers.push_back(Loudspeaker(ls));
  }
}

void ssr::Scene::new_source(const id_t id)
{
  // do nothing if id already exists:
  if (maptools::get_item(_source_map, id) == nullptr)
  {
     VERBOSE("Adding source " << id << " to source map!");
    _source_map[id] = Source(_loudspeakers.size());
  }
}

void ssr::Scene::delete_source(id_t id)
{
  // this should call the destructor for the Source object.
  // IMPORTANT: the map holds the Sources directly, no pointers!
  _source_map.erase(id);
}

void ssr::Scene::delete_all_sources()
{
  // this should call the destructor for all Source objects.
  // IMPORTANT: the map holds the Sources directly, no pointers!
  _source_map.clear();
}

bool ssr::Scene::set_source_position(id_t id, const Position& position)
{
  return _set_source_member(id, &Source::position, position);
}

bool ssr::Scene::set_source_orientation(id_t id, const Orientation& orientation)
{
  return _set_source_member(id, &Source::orientation, orientation);
}

bool ssr::Scene::set_source_gain(id_t id, const float& gain)
{
  return _set_source_member(id, &Source::gain, gain);
}

bool ssr::Scene::set_source_signal_level(id_t id, const float& level)
{
  return _set_source_member(id, &Source::signal_level, level);
}


bool ssr::Scene::set_source_mute(id_t id, const bool& mute)
{
  return _set_source_member(id, &Source::mute, mute);
}

bool ssr::Scene::set_source_name(id_t id, const std::string& name)
{
  return _set_source_member(id, &Source::name, name);
}

bool ssr::Scene::set_source_properties_file(id_t id, const std::string& name)
{
  return _set_source_member(id, &Source::properties_file, name);
}

bool ssr::Scene::set_source_port_name(id_t id, const std::string& port_name)
{
  return _set_source_member(id, &Source::port_name, port_name);
}

bool ssr::Scene::set_source_file_name(id_t id, const std::string& file_name)
{
  return _set_source_member(id, &Source::audio_file_name, file_name);
}

bool ssr::Scene::set_source_file_channel(id_t id, const int& file_channel)
{
  return _set_source_member(id, &Source::audio_file_channel, file_channel);
}

bool ssr::Scene::set_source_position_fixed(id_t id, const bool& fixed)
{
  return _set_source_member(id, &Source::fixed_position, fixed);
}

bool ssr::Scene::set_source_file_length(id_t id, const long int& length)
{
  return _set_source_member(id, &Source::file_length, length);
}

void ssr::Scene::set_reference_position(const Position& position)
{
  _reference.position = position;
}

void ssr::Scene::set_reference_orientation(const Orientation& orientation)
{
  _reference.orientation = orientation;
}

void ssr::Scene::set_reference_offset_position(const Position& position)
{
  _reference_offset.position = position;
}

void ssr::Scene::set_reference_offset_orientation(const Orientation& orientation)
{
  _reference_offset.orientation = orientation;
}

bool ssr::Scene::set_source_model(id_t id, const Source::model_t& model)
{
  return _set_source_member(id, &Source::model, model);
}

// linear volume!
void ssr::Scene::set_master_volume(float volume)
{
  _master_volume = volume;
}

void ssr::Scene::set_decay_exponent(float exponent)
{
  _decay_exponent = exponent;
}
void ssr::Scene::set_amplitude_reference_distance(float dist)
{
  _amplitude_reference_distance = dist;
}

// linear scale!
void ssr::Scene::set_master_signal_level(float level)
{
  _master_signal_level = level;
}

void ssr::Scene::set_cpu_load(float load)
{
  _cpu_load = load;
}

void ssr::Scene::set_sample_rate(int sr)
{
  _sample_rate = sr;
}

void ssr::Scene::set_source_output_levels(id_t id, float* first, float* last)
{
  Source* const source_ptr = maptools::get_item(_source_map, id);

  if (!source_ptr)
  {
    ERROR("Source " << id << " doesn't exist!");
    return;
  }

  if (source_ptr->output_levels.size() == size_t(std::distance(first,last)))
  {
    std::copy(first, last, source_ptr->output_levels.begin());
  }
}

void ssr::Scene::set_processing_state(bool state)
{
  _processing_state = state;
}

/// _. @return processing state
bool ssr::Scene::get_processing_state() const
{
  return _processing_state;
}

/// _. @return master volume
float ssr::Scene::get_master_volume() const
{
  return _master_volume;
}

/// _. @return amplitude decay exponent
float ssr::Scene::get_decay_exponent() const
{
  return _decay_exponent;
}

/// _. @return amplitude reference distance
float ssr::Scene::get_amplitude_reference_distance() const
{
  return _amplitude_reference_distance;
}

/// _. @return instantaneous overall audio signal level
float ssr::Scene::get_master_signal_level() const
{
  return _master_signal_level;
}

/// _. @return CPU load in percent
float ssr::Scene::get_cpu_load() const
{
  return _cpu_load;
}

/// _. @return current sample rate
int ssr::Scene::get_sample_rate() const
{
  return _sample_rate;
}


//void ssr::Scene::set_transport_state(JackClient::State state)
void ssr::Scene::set_transport_state(const std::pair<bool, jack_nframes_t>& state)
{
  //_transport_state = state;
  _transport_playing = state.first;
  _transport_position = state.second;
}

void ssr::Scene::set_auto_rotation(bool auto_rotate_sources)
{
  _auto_rotate_sources = auto_rotate_sources;
}

bool ssr::Scene::is_playing() const
{
  //return _transport_state.playing;
  return _transport_playing;
}

ssr::jack_nframes_t ssr::Scene::get_transport_position() const
{
  //return _transport_state.position;
  return _transport_position;
}

bool ssr::Scene::get_auto_rotation() const
{
  return _auto_rotate_sources;
}

Source ssr::Scene::get_source(id_t id) const
{
  auto source = Source(_loudspeakers.size());
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
std::unique_ptr<Position> ssr::Scene::get_source_position(id_t id) const
{
  std::unique_ptr<Position> position(new Position); // standard ctor
  if (_get_source_member(id, &Source::position, *position)) return position;
  position.reset(); // set to NULL
  return position;
}

/** _.
 * @param id ID of the source
 * @return position of the source
 * @warning If @a id is not found, a unique_ptr to NULL is returned!
 **/
std::unique_ptr<Orientation> ssr::Scene::get_source_orientation(id_t id) const
{
  std::unique_ptr<Orientation> orientation(new Orientation); // standard ctor
  if (_get_source_member(id, &Source::orientation, *orientation)) return orientation;
  orientation.reset(); // set to NULL
  return orientation;
}

/** _.
 * @param id ID of the source
 * @return source model
 * @warning If @a id is not found, Source::unknown is returned
 **/
Source::model_t ssr::Scene::get_source_model(id_t id) const
{
  Source::model_t model;
  if (_get_source_member(id, &Source::model, model)) return model;
  return Source::unknown;
}

/** _.
 * @param id ID of the source
 * @return linear source gain (=volume)
 * @warning If @a id is not found, 0 is returned
 **/
float ssr::Scene::get_source_gain(id_t id) const
{
  float gain;
  if (_get_source_member(id, &Source::gain, gain)) return gain;
  return 0;
}

/** _.
 * @param id ID of the source
 * @return mute state
 * @warning If @a id is not found, false is returned
 **/
bool ssr::Scene::get_source_mute_state(id_t id) const
{
  bool state;
  if (_get_source_member(id, &Source::mute, state)) return state;
  return false;
}

/** _.
 * @param id ID of the source
 * @return source name
 * @warning If @a id is not found, an empty string is returned
 **/
std::string ssr::Scene::get_source_name(id_t id) const
{
  std::string name;
  if (_get_source_member(id, &Source::name, name)) return name;
  return std::string("");
}

bool ssr::Scene::get_source_position_fixed(id_t id) const
{
  bool boolean;
  if (_get_source_member(id, &Source::fixed_position, boolean)) return boolean;

  return false;
}

std::string ssr::Scene::get_source_properties_file(id_t id) const
{
  std::string name;
  if (_get_source_member(id, &Source::properties_file, name)) return name;
  return std::string("");
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
