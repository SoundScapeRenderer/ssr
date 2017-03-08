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
/// Abstract %Publisher (definition).

// TODO: use a less general name?
#ifndef SSR_PUBLISHER_H
#define SSR_PUBLISHER_H

#include <inttypes.h>  // for uint32_t

#include "source.h"
#include "ssr_global.h"

using jack_nframes_t = uint32_t; // from <jack/types.h>

namespace ssr
{

struct Subscriber;

/** Abstract interface to controller class (and similar classes).
 * All member functions are public, everyone can call them.
 * @attention For now, none of the member functions has a return value. That is
 * because if we are connected via a slow IP interface we do not want to wait
 * for transferring back the return value.
 * \par
 * Anyhow, maybe this is not a good decision. Maybe there should be return
 * values?
 **/
struct Publisher
{
  virtual ~Publisher() {} ///< dtor. does nothing

  /// load scene from XML file.
  /// @param scene_file_name name of XML file to load.
  virtual bool load_scene(const std::string& scene_file_name) = 0;
  virtual bool save_scene_as_XML(const std::string& filename) const = 0;

  virtual void start_processing() = 0; ///< start processing
  virtual void stop_processing() = 0;  ///< stop  processing

  /// create new source.
  /// @param name name of the new source
  /// @param model model (=type) of the new source
  /// @param file_or_port_name name of audio file or JACK port. If a JACK port
  /// is given, @a channel has to be zero!
  /// @param channel number of channel in audio file. If zero, a JACK port is
  /// given instead of an audio file.
  /// @param position initial position of the new source.
  /// @param orientation initial orientation of the new source.
  /// @param gain initial gain of the new source.
  virtual void new_source(const std::string& name, Source::model_t model,
      const std::string& file_or_port_name, int channel = 0,
      const Position& position = Position(), const bool pos_fix = false,
      const Orientation& orientation = Orientation(), const bool or_fix = false,
      const float gain = 1.0f, const bool mute_state = false, 
      const std::string& properties_file = "") = 0;

  /// delete a source
  /// @param[in] id id of the source to be deleted
  virtual void delete_source(id_t id) = 0;

  /// delete all sources
  virtual void delete_all_sources() = 0;

  /// set position of a source
  /// @param id source id.
  /// @param position source position.
  /// @todo include some sort of identification who sent the request.
  virtual void set_source_position(id_t id, const Position& position) = 0;
  /// set orientation of a source
  virtual void set_source_orientation(id_t id,
      const Orientation& orientation) = 0;
  /// set gain (=volume) of a source
  virtual void set_source_gain(id_t id, float gain) = 0;
  /// mute/unmute source
  virtual void set_source_mute(id_t id, bool mute) = 0;
  /// instantaneous level of audio stream
  virtual void set_source_signal_level(const id_t id,
      const float level) = 0;
  /// set name of a source
  virtual void set_source_name(id_t id, const std::string& name) = 0;
  virtual void set_source_properties_file(id_t id, const std::string& name) = 0;
  /// set model (=type) of a source
  virtual void set_source_model(id_t id, Source::model_t model) = 0;
  /// set JACK port of a source
  virtual void
  set_source_port_name(id_t id, const std::string& port_name) = 0;
  virtual void
  set_source_position_fixed(id_t id, const bool fix) = 0;

  /// set position of the reference
  /// @param position well, the position
  virtual void set_reference_position(const Position& position) = 0;
  /// set orientation of the reference
  virtual void set_reference_orientation(const Orientation& orientation) = 0;
  virtual void set_reference_offset_position(const Position& position) = 0;
  virtual void set_reference_offset_orientation(const Orientation& orientation) = 0;

  /// set master volume of the whole scene
  virtual void set_master_volume(float volume) = 0;

  /// set amplitude decay exponent
  virtual void
  set_decay_exponent(float exponent) = 0;

  /// set amplitude reference distance
  virtual void
  set_amplitude_reference_distance(float distance) = 0;

  /// set instantaneous overall audio level (linear scale)
  virtual void set_master_signal_level(float level) = 0;

  /// set CPU load in percent as estimated by JACK
  virtual void set_cpu_load(const float load) = 0;

  /// sets the sample rate in all subscribers
  virtual void publish_sample_rate(const int sample_rate) = 0;

  /// returns what type the current renderer actually is
  virtual std::string get_renderer_name() const = 0;

  /// show head in GUI?
  virtual bool show_head() const = 0;

  virtual void transport_start() = 0; ///< start JACK transport

  virtual void transport_stop() = 0;  ///< stop  JACK transport
  /// set JACK transport location
  virtual bool transport_locate(float time_in_sec) = 0;
  /// This is temporarily used to calibrate the tracker
  virtual void calibrate_client() = 0;

  virtual void set_processing_state(bool state) = 0;

  virtual void set_auto_rotation(bool auto_rotate_sources) = 0;

  virtual void subscribe(Subscriber* subscriber) = 0;
  virtual void unsubscribe(Subscriber* subscriber) = 0;
  virtual std::string get_scene_as_XML() const = 0;
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
