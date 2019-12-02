/******************************************************************************
 * Copyright © 2019 SSR Contributors                                          *
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
/// Receives events from the SSR to be passed on via FUDI.

#ifndef SSR_FUDI_SUBSCRIBER_H
#define SSR_FUDI_SUBSCRIBER_H

#include <fmt/format.h>

#include "api.h"
#include "ssr_global.h"  // for SSR_ERROR
#include "geometry.h"  // for quat2angles()

namespace ssr
{

namespace fudi
{

class Connection;

// TODO: custom inline size?
using buffer_t = fmt::memory_buffer;

class Subscriber final : public api::BundleEvents
                       , public api::SceneInformationEvents
                       , public api::SceneControlEvents
                       , public api::RendererInformationEvents
                       , public api::RendererControlEvents
                       , public api::TransportFrameEvents
                       , public api::SourceMetering
                       , public api::MasterMetering
                       , public api::OutputActivity
                       , public api::CpuLoad
{
public:
  explicit Subscriber(Connection& connection, api::Publisher& controller)
    : _connection{connection}
    , _controller{controller}
    , _buffer{std::make_shared<buffer_t>()}
  {}

  // NB: Already subscribed events are automatically unsubscribed first
  void subscribe(std::string_view name)
  {
    if (name == "scene")
    {
      _sub_scene_control.reset();
      _sub_scene_information.reset();
      _sub_scene_information = _controller.subscribe()->scene_information(this);
      _sub_scene_control = _controller.subscribe()->scene_control(this);
    }
    else if (name == "scene-control")
    {
      _sub_scene_control.reset();
      _sub_scene_control = _controller.subscribe()->scene_control(this);
    }
    else if (name == "scene-information")
    {
      _sub_scene_information.reset();
      _sub_scene_information = _controller.subscribe()->scene_information(this);
    }
    else if (name == "renderer")
    {
      _sub_renderer_control.reset();
      _sub_renderer_information.reset();
      _sub_renderer_information =
        _controller.subscribe()->renderer_information(this);
      _sub_renderer_control = _controller.subscribe()->renderer_control(this);
    }
    else if (name == "renderer-information")
    {
      _sub_renderer_information.reset();
      _sub_renderer_information =
        _controller.subscribe()->renderer_information(this);
    }
    else if (name == "renderer-control")
    {
      _sub_renderer_control.reset();
      _sub_renderer_control = _controller.subscribe()->renderer_control(this);
    }
    else if (name == "transport-frame")
    {
      _sub_transport.reset();
      _sub_transport = _controller.subscribe()->transport_frame(this);
    }
    else if (name == "source-metering")
    {
      _sub_source_metering.reset();
      _sub_source_metering = _controller.subscribe()->source_metering(this);
    }
    else if (name == "master-metering")
    {
      _sub_master_metering.reset();
      _sub_master_metering = _controller.subscribe()->master_metering(this);
    }
    else if (name == "output-activity")
    {
      _sub_output_activity.reset();
      _sub_output_activity = _controller.subscribe()->output_activity(this);
    }
    else if (name == "cpu-load")
    {
      _sub_cpu_load.reset();
      _sub_cpu_load = _controller.subscribe()->cpu_load(this);
    }
    else
    {
      SSR_ERROR("Unknown subscription: \"" << name << "\"");
    }
  }

  void unsubscribe(std::string_view name)
  {
    if (name == "scene")
    {
      _sub_scene_control.reset();
      _sub_scene_information.reset();
    }
    else if (name == "scene-control")
    {
      _sub_scene_control.reset();
    }
    else if (name == "scene-information")
    {
      _sub_scene_information.reset();
    }
    else if (name == "renderer")
    {
      _sub_renderer_control.reset();
      _sub_renderer_information.reset();
    }
    else if (name == "renderer-information")
    {
      _sub_renderer_information.reset();
    }
    else if (name == "renderer-control")
    {
      _sub_renderer_control.reset();
    }
    else if (name == "transport-frame")
    {
      _sub_transport.reset();
    }
    else if (name == "source-metering")
    {
      _sub_source_metering.reset();
    }
    else if (name == "master-metering")
    {
      _sub_master_metering.reset();
    }
    else if (name == "output-activity")
    {
      _sub_output_activity.reset();
    }
    else if (name == "cpu-load")
    {
      _sub_cpu_load.reset();
    }
    else
    {
      SSR_ERROR("Unknown subscription: \"" << name << "\"");
    }
  }

  void source_numbers(bool value) { _source_numbers = value; }
  void quaternions(bool value) { _quaternions = value; }

private:
  void _send_buffer(std::shared_ptr<buffer_t>);

  void _source_id(id_t id)
  {
    assert(_buffer);
    if (_source_numbers)
    {
      unsigned int number = _controller.get_source_number(id);
      fmt::format_to(*_buffer, "src {} ", number);
    }
    else
    {
      fmt::format_to(*_buffer, "src {} ", id);
    }
  }

  void _escaped_string(std::string_view str)
  {
    assert(_buffer);
    for (size_t idx = 0; idx < str.size(); ++idx)
    {
      auto previous_idx = idx;
      idx = str.find_first_of(" \n\t\r\\;", idx);
      fmt::format_to(*_buffer, str.substr(previous_idx, idx - previous_idx));
      if (idx == std::string_view::npos)
      {
        break;
      }
      fmt::format_to(*_buffer, "\\{}", str[idx]);
    }
  }

  void _pos(const Pos& pos)
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "pos {:f} {:f} {:f};\n", pos.x, pos.y, pos.z);
  }

  void _rot(const Rot& rot)
  {
    assert(_buffer);
    if (_quaternions)
    {
      fmt::format_to(*_buffer, "quat {:f} {:f} {:f} {:f};\n"
          , rot.x, rot.y, rot.z, rot.w);
    }
    else
    {
      auto [azimuth, elevation, roll] = quat2angles(rot);
      fmt::format_to(*_buffer, "rot {:f} {:f} {:f};\n"
          , azimuth, elevation, roll);
    }
  }

  // BundleEvents

  void bundle_start() override
  {
    // Nothing to be done here
  }

  void bundle_stop() override
  {
    assert(_buffer);
    if (_buffer->size())
    {
      _send_buffer(std::move(_buffer));
      _buffer = std::make_shared<buffer_t>();
    }
  }

  // SceneControlEvents

  void auto_rotate_sources(bool auto_rotate) override
  {
    (void)auto_rotate;
    // TODO
  }

  void delete_source(id_t id) override
  {
    (void)id;
    // TODO
  }

  void source_active(id_t id, bool active) override
  {
    _source_id(id);
    assert(_buffer);
    fmt::format_to(*_buffer, "active {:d};\n", active);
  }

  void source_position(id_t id, const Pos& position) override
  {
    _source_id(id);
    _pos(position);
  }

  void source_rotation(id_t id, const Rot& rotation) override
  {
    _source_id(id);
    _rot(rotation);
  }

  void source_volume(id_t id, float volume) override
  {
    _source_id(id);
    assert(_buffer);
    fmt::format_to(*_buffer, "vol {:f};\n", volume);
  }

  void source_mute(id_t id, bool mute) override
  {
    _source_id(id);
    assert(_buffer);
    fmt::format_to(*_buffer, "mute {:d};\n", mute);
  }

  void source_name(id_t id, const std::string& name) override
  {
    _source_id(id);
    assert(_buffer);
    fmt::format_to(*_buffer, "name ");
    _escaped_string(name);
    fmt::format_to(*_buffer, ";\n");
  }

  void source_model(id_t id, const std::string& model) override
  {
    _source_id(id);
    assert(_buffer);
    fmt::format_to(*_buffer, "model {};\n", model);
  }

  void source_fixed(id_t id, bool fixed) override
  {
    _source_id(id);
    assert(_buffer);
    fmt::format_to(*_buffer, "fixed {:d};\n", fixed);
  }

  void reference_position(const Pos& position) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "ref ");
    _pos(position);
  }

  void reference_rotation(const Rot& rotation) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "ref ");
    _rot(rotation);
  }

  void master_volume(float volume) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "vol {:f};\n", volume);
  }

  void decay_exponent(float exponent) override
  {
    (void)exponent;
    // TODO
  }

  void amplitude_reference_distance(float distance) override
  {
    (void)distance;
    // TODO
  }

  void transport_rolling(bool rolling) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "transport {:d};\n", rolling);
  }

  // SceneInformationEvents

  void sample_rate(int rate) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "samplerate {:d};\n", rate);
  }

  void new_source(id_t id) override
  {
    (void)id;
    // TODO
  }

  void source_property(id_t id, const std::string& key
                              , const std::string& value) override
  {
    (void)id;
    (void)key;
    (void)value;
    // TODO
  }

  // RendererControlEvents

  void processing(bool processing_state) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "processing {:d};\n", processing_state);
  }

  void reference_position_offset(const Pos& position) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "ref offset ");
    _pos(position);
  }

  void reference_rotation_offset(const Rot& rotation) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "ref offset ");
    _rot(rotation);
  }

  // RendererInformationEvents

  void renderer_name(const std::string& name) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "renderer-name ");
    _escaped_string(name);
    fmt::format_to(*_buffer, ";\n");
  }

  void loudspeakers(const std::vector<Loudspeaker>& loudspeakers) override
  {
    (void)loudspeakers;
    // TODO
  }

  // TransportFrameEvents

  void transport_frame(uint32_t frame) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "frame {:d};\n", frame);
  }

  // SourceMetering

  void source_level(id_t id, float level) override
  {
    _source_id(id);
    assert(_buffer);
    fmt::format_to(*_buffer, "level {:f};\n", level);
  }

  // MasterMetering

  void master_level(float level) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "level {:f};\n", level);
  }

  // OutputActivity

  void output_activity(id_t id, float* begin, float* end) override
  {
    if (begin == end) { return; }
    _source_id(id);
    assert(_buffer);
    fmt::format_to(*_buffer, "output-activity");
    for (auto* ptr = begin; ptr != end; ++ptr)
    {
      fmt::format_to(*_buffer, " {:f}", *ptr);
    }
    fmt::format_to(*_buffer, ";\n");
  }

  // CpuLoad

  void cpu_load(float load) override
  {
    assert(_buffer);
    fmt::format_to(*_buffer, "cpu-load {:.2f};\n", load);
  }

  Connection& _connection;
  api::Publisher& _controller;
  std::shared_ptr<buffer_t> _buffer;

  bool _source_numbers{true};
  bool _quaternions{false};

  std::unique_ptr<api::Subscription> _sub_scene_information;
  std::unique_ptr<api::Subscription> _sub_scene_control;
  std::unique_ptr<api::Subscription> _sub_renderer_information;
  std::unique_ptr<api::Subscription> _sub_renderer_control;
  std::unique_ptr<api::Subscription> _sub_transport;
  std::unique_ptr<api::Subscription> _sub_source_metering;
  std::unique_ptr<api::Subscription> _sub_master_metering;
  std::unique_ptr<api::Subscription> _sub_output_activity;
  std::unique_ptr<api::Subscription> _sub_cpu_load;
};

}  // namespace fudi

}  // namespace ssr

#endif
