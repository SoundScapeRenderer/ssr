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
/// %RenderSubscriber (definition).

#ifndef SSR_RENDERSUBSCRIBER_H
#define SSR_RENDERSUBSCRIBER_H

#include "api.h"
#include "ssr_global.h"  // for WARNING()

#include "rendererbase.h"  // for Source

namespace ssr
{

template<typename Renderer>
class RenderSubscriber : public api::BundleEvents
                       // TODO: new_source
                       , public api::SceneControlEvents
                       , public api::RendererControlEvents
                       , public api::RendererInformationEvents
{
public:
  RenderSubscriber(Renderer &renderer) : _renderer(renderer) {}

  void get_data(api::RendererControlEvents* subscriber)
  {
    subscriber->processing(_processing);
    subscriber->reference_position_offset(_reference_position_offset);
    subscriber->reference_rotation_offset(_reference_rotation_offset);
  }

  void get_data(api::RendererInformationEvents* subscriber)
  {
    subscriber->renderer_name(_renderer_name);
    subscriber->loudspeakers(_loudspeakers);
  }

private:
  using Source = typename RendererBase<Renderer>::Source;

  template<typename PTM, typename T>
  void _set_source_member(id_t id, PTM member, T&& arg)
  {
    auto* src = _renderer.get_source(id);
    if (!src)
    {
      WARNING("Source \"" << id << "\" does not exist.");
    }
    else
    {
      src->*member = std::forward<T>(arg);
    }
  }

  // BundleEvents

  void bundle_start() override
  {
    // TODO: implement
  }

  void bundle_stop() override
  {
    // TODO: implement
  }

  // SceneControlEvents

  void auto_rotate_sources(bool auto_rotate) override
  {
    (void) auto_rotate;
  }

  void delete_source(id_t id) override
  {
    _renderer.rem_source(id);
  }

  void source_position(id_t id, const Pos& pos) override
  {
    _set_source_member(id, &Source::position, pos);
  }

  void source_rotation(id_t id, const Rot& rot) override
  {
    _set_source_member(id, &Source::rotation, rot);
  }

  void source_volume(id_t id, float volume) override
  {
    _set_source_member(id, &Source::gain, volume);
  }

  void source_mute(id_t id, bool mute) override
  {
    _set_source_member(id, &Source::mute, mute);
  }

  void source_name(id_t, const std::string&) override
  {
    // Not used in renderer
  }

  void source_model(id_t id, const std::string& model) override
  {
    _set_source_member(id, &Source::model, model);
  }

  void source_fixed(id_t, bool) override
  {
    // Not used in renderer
  }

  void reference_position(const Pos& pos) override
  {
    _renderer.state.reference_position = pos;
  }

  void reference_rotation(const Rot& rot) override
  {
    _renderer.state.reference_rotation = rot;
  }

  void master_volume(float volume) override
  {
    _renderer.state.master_volume = volume;
  }

  void decay_exponent(float exponent) override
  {
    _renderer.state.decay_exponent = exponent;
  }

  void amplitude_reference_distance(float distance) override
  {
    _renderer.state.amplitude_reference_distance = distance;
  }

  // RendererControlEvents

  void processing(bool state) override
  {
    _renderer.state.processing = state;
    _processing = state;
  }

  void reference_position_offset(const Pos& pos) override
  {
    _renderer.state.reference_position_offset = pos;
    _reference_position_offset = pos;
  }

  void reference_rotation_offset(const Rot& rot) override
  {
    _renderer.state.reference_rotation_offset = rot;
    _reference_rotation_offset = rot;
  }

  // RendererInformationEvents (not needed in renderer!)

  void renderer_name(const std::string& name) override
  {
    _renderer_name = name;
  }

  void loudspeakers(const std::vector<Loudspeaker>& loudspeakers) override
  {
    _loudspeakers = loudspeakers;
  }

  Renderer& _renderer;

  bool _processing{false};
  Pos _reference_position_offset;
  Rot _reference_rotation_offset;
  std::string _renderer_name;
  std::vector<Loudspeaker> _loudspeakers;
};

}  // namespace ssr

#endif
