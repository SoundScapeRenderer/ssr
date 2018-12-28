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
/// Abstract %Subscriber (definition).

#ifndef SSR_SUBSCRIBER_H
#define SSR_SUBSCRIBER_H

#include <set>
#include <stdint.h>

#include "ssr_global.h"
#include "source.h"
#include "loudspeaker.h"

namespace ssr
{

using jack_nframes_t = uint32_t; // from <jack/types.h>

/** Abstract interface to Scene class (and similar classes).
 **/
struct Subscriber
{
  /// dtor. does nothing.
  virtual ~Subscriber() {}

  virtual void set_loudspeakers(const Loudspeaker::container_t& loudspeakers) = 0;

  /// Create a new source with default values.
  /// @param id ID of the new source
  virtual void new_source(id_t id) {(void)id;}

  /// Remove a source from the Scene.
  /// @param id ID of the source
  virtual void delete_source(id_t id)  = 0;

  /// delete all sources, quite obviously.
  virtual void delete_all_sources()  = 0;

  /// Set source position.
  /// @param id ID of the source
  /// @param position new position
  virtual bool set_source_position(id_t id, const Position& position)  = 0;

  /// Set source orientation.
  /// @param id ID of the source
  /// @param orientation new orientation
  virtual bool set_source_orientation(id_t id,
      const Orientation& orientation)  = 0;

  /// Set source gain.
  /// @param id ID of the source
  /// @param gain new gain
  /// @attention 2nd argument is given by const reference to facilitate the
  /// _publish() function in the Controller class.
  virtual bool set_source_gain(id_t id, const float& gain)  = 0;

        /// Set instantaneous level of audio stream.
  /// @param id ID of the source
  /// @param level new level (linear scale)
  /// @param intitiator not implemented
  virtual bool set_source_signal_level(const id_t id,
      const float& level)  = 0;

  /// mute/unmute source.
  /// @param id ID of the source
  /// @param mute mute if @b true, unmute if @b false
  /// @attention 2nd argument is given by const reference to facilitate the
  /// _publish() function in the Controller class.
  virtual bool set_source_mute(id_t id, const bool& mute)  = 0;

  /// Set source name.
  /// @param id ID of the source
  /// @param name new name
  virtual bool set_source_name(id_t id, const std::string& name)  = 0;

  /// Set name of file containing impulse responses.
  /// @param id ID of the source
  /// @param name file name
  virtual bool set_source_properties_file(id_t id, const std::string& name)  = 0;

  /// Set source model (=type).
  /// @param id ID of the source
  /// @param model new model
  /// @attention 2nd argument is given by const reference to facilitate the
  /// _publish() function in the Controller class.
  virtual bool set_source_model(id_t id, const Source::model_t& model)  = 0;

  /// Set port name of a source.
  /// @param id ID of the source
  /// @param port_name JACK port name
  virtual bool set_source_port_name(id_t id, const std::string& port_name)  = 0;

  virtual bool set_source_file_name(id_t id, const std::string& file_name) = 0;

  virtual bool set_source_file_channel(id_t id, const int& file_channel) = 0;

  virtual bool set_source_position_fixed(id_t id, const bool& fixed) = 0;

  /// Set length of associated audio file.
  /// @param id ID of the source
  /// @param length length in samples
  /// @attention 2nd argument is given by const reference to facilitate the
  /// _publish() function in the Controller class.
  virtual bool set_source_file_length(id_t id, const long int& length)  = 0;

  /// Set reference position.
  /// @param position new position
  virtual void set_reference_position(const Position& position)  = 0;

  /// Set reference orientation.
  /// @param orientation new orientation
  virtual void set_reference_orientation(const Orientation& orientation)  = 0;

  virtual void set_reference_offset_position(const Position& position)  = 0;
  virtual void set_reference_offset_orientation(const Orientation& orientation)  = 0;

  /// Set master volume.
  /// @param volume volume
  virtual void set_master_volume(float volume)  = 0;

  /// Set amplitude decay exponent.
  /// @param exponent amplitude decay exponent
  virtual void set_decay_exponent(float exponent)  = 0;

  /// Set amplitude reference distance.
  /// @param distance amplitude reference distance
  virtual void set_amplitude_reference_distance(float distance)  = 0;

  /// Set master volume.
  /// @param level instantaneous overall audio level (linear scale)
  virtual void set_master_signal_level(float level)  = 0;

  /// Set CPU load.
  /// @param load CPU load in percent
  virtual void set_cpu_load(float load)  = 0;

  /// Set sample rate.
  /// @param sample_rate sample rate
  virtual void set_sample_rate(int sample_rate)  = 0;

  virtual void set_source_output_levels(id_t id, float* first, float* last) = 0;

  /// Update information about audio processing;
  virtual void set_processing_state(bool state)  = 0;

  virtual void set_transport_state(const std::pair<bool, jack_nframes_t>& state)  = 0;

  virtual void set_auto_rotation(bool auto_rotate_sources) = 0;
};

}  // namespace ssr

#endif
