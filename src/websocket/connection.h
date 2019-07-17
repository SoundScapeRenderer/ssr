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
/// A WebSocket connection.

#ifndef SSR_WEBSOCKET_CONNECTION_H
#define SSR_WEBSOCKET_CONNECTION_H

#include <optional>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "../api.h"
#include "ssr_global.h"  // for ERROR

namespace ssr
{

/// WebSocket interface
namespace ws
{

using websocketpp::connection_hdl;
using server_t = websocketpp::server<websocketpp::config::asio>;
using message_ptr = server_t::message_ptr;

namespace json = rapidjson;

/// @warning This is slow! It is only used for generating error messages.
inline std::ostream& operator<<(std::ostream& stream, const json::Value& value)
{
  json::OStreamWrapper osw(stream);
  json::Writer<json::OStreamWrapper> writer(osw);
  value.Accept(writer);
  return stream;
}

namespace internal
{
  inline std::optional<Pos> parse_position(const json::Value& value)
  {
    if (!value.IsArray() || value.Size() != 3)
    {
      SSR_ERROR("Position must be a JSON list with 3 elements, not " << value);
      return std::nullopt;
    }
    const auto& x = value[0];
    const auto& y = value[1];
    const auto& z = value[2];
    if (!x.IsNumber() || !y.IsNumber() || !z.IsNumber())
    {
      SSR_ERROR("Position must be a list of numbers, not " << value);
      return std::nullopt;
    }
    return Pos{static_cast<float>(x.GetDouble())
             , static_cast<float>(y.GetDouble())
             , static_cast<float>(z.GetDouble())};
  }

  inline std::optional<Rot> parse_rotation(const json::Value& value)
  {
    if (!value.IsArray() || value.Size() != 4)
    {
      SSR_ERROR("Rotation must be a JSON list with 4 elements, not " << value);
      return std::nullopt;
    }
    const auto& x = value[0];
    const auto& y = value[1];
    const auto& z = value[2];
    const auto& w = value[3];
    if (!x.IsNumber() || !y.IsNumber() || !z.IsNumber() || !w.IsNumber())
    {
      SSR_ERROR("Rotation must be a list of numbers, not " << value);
      return std::nullopt;
    }
    return Rot{static_cast<float>(x.GetDouble())
             , static_cast<float>(y.GetDouble())
             , static_cast<float>(z.GetDouble())
             , static_cast<float>(w.GetDouble())};
  }
}  // namespace internal


class Connection : public api::BundleEvents
                 , public api::SceneControlEvents
                 , public api::SceneInformationEvents
                 , public api::RendererControlEvents
                 , public api::RendererInformationEvents
                 , public api::TransportFrameEvents
                 , public api::SourceMetering
                 , public api::MasterMetering
                 , public api::OutputActivity
                 , public api::CpuLoad
{
public:
  explicit Connection(connection_hdl hdl, server_t& server
      , api::Publisher& controller, size_t buffer_size = 128 * 1024)
    : _hdl{hdl}
    , _server{server}
    , _controller{controller}
    , _in_buffer(buffer_size)
    , _out_buffer(buffer_size)
    , _out_allocator{_out_buffer.data(), _out_buffer.size()}
    , _bundle_subscription{_controller.subscribe()->bundle(this)}
  {}

  void on_message(message_ptr msg);

private:
  using Allocator = json::MemoryPoolAllocator<>;
  using Encoding = json::UTF8<char>;
  using Document = json::GenericDocument<Encoding, Allocator, Allocator>;
  using StringBuffer = json::GenericStringBuffer<Encoding, Allocator>;
  using Writer = json::Writer<StringBuffer, Encoding, Encoding, Allocator>;

  // BundleEvents

  void bundle_start() override
  {
    // Nothing to do here.  All data should be empty.
  }

  void bundle_stop() override
  {
    Document commands{json::kArrayType, &_out_allocator, 1024, &_out_allocator};

    assert(_state.IsObject());
    if (!_state.ObjectEmpty())
    {
      commands.PushBack("state", _out_allocator);
      commands.PushBack(_state.Move(), _out_allocator);
      _state.SetObject();
    }
    assert(_new_source.IsObject());
    if (!_new_source.ObjectEmpty())
    {
      commands.PushBack("new-src", _out_allocator);
      commands.PushBack(_new_source.Move(), _out_allocator);
      _new_source.SetObject();
    }
    assert(_modify_source.IsObject());
    if (!_modify_source.ObjectEmpty())
    {
      commands.PushBack("mod-src", _out_allocator);
      commands.PushBack(_modify_source.Move(), _out_allocator);
      _modify_source.SetObject();
    }
    assert(_delete_source.IsArray());
    if (!_delete_source.Empty())
    {
      commands.PushBack("del-src", _out_allocator);
      commands.PushBack(_delete_source.Move(), _out_allocator);
      _delete_source.SetArray();
    }
    if (!commands.Empty())
    {
      StringBuffer buffer{&_out_allocator};
      Writer writer(buffer, &_out_allocator);
      commands.Accept(writer);
      _server.send(_hdl, buffer.GetString(), websocketpp::frame::opcode::text);
      SSR_VERBOSE3("output allocator usage: " << _out_allocator.Size());
      _out_allocator.Clear();
    }
  }

  // SceneControlEvents

  void auto_rotate_sources(bool auto_rotate) override
  {
    _update_object(_state, "auto-rotate-sources", auto_rotate);
  }

  void delete_source(id_t id) override
  {
    _delete_source.PushBack(json::Value{id, _out_allocator}, _out_allocator);
  }

  void source_position(id_t id, const Pos& position) override
  {
    _set_source_property(id, "pos", _to_list(position));
  }

  void source_rotation(id_t id, const Rot& rotation) override
  {
    _set_source_property(id, "rot", _to_list(rotation));
  }

  void source_volume(id_t id, float volume) override
  {
    _set_source_property(id, "volume", volume);
  }

  void source_mute(id_t id, bool mute) override
  {
    _set_source_property(id, "mute", mute);
  }

  void source_name(id_t id, const std::string& name) override
  {
    _set_source_property(id, "name", name);
  }

  void source_model(id_t id, const std::string& model) override
  {
    _set_source_property(id, "model", model);
  }

  void source_fixed(id_t id, bool fixed) override
  {
    _set_source_property(id, "fixed", fixed);
  }

  void reference_position(const Pos& position) override
  {
    _update_object(_state, "ref-pos", _to_list(position));
  }

  void reference_rotation(const Rot& rotation) override
  {
    _update_object(_state, "ref-rot", _to_list(rotation));
  }

  void master_volume(float volume) override
  {
    _update_object(_state, "master-volume", volume);
  }

  void decay_exponent(float exponent) override
  {
    _update_object(_state, "decay-exponent", exponent);
  }

  void amplitude_reference_distance(float distance) override
  {
    _update_object(_state, "amplitude-reference-distance", distance);
  }

  // SceneInformationEvents

  void sample_rate(int rate) override
  {
    _update_object(_state, "sample-rate", rate);
  }

  void new_source(id_t id) override
  {
    _add_member(_new_source, id, json::Value{json::kObjectType});
  }

  void source_property(id_t id, const std::string& key
                              , const std::string& value) override
  {
    _set_source_property(id, key, value);
  }

  void transport_rolling(bool rolling) override
  {
    _update_object(_state, "transport-rolling", rolling);
  }

  // RendererControlEvents

  void processing(bool processing_state) override
  {
    _update_object(_state, "processing", processing_state);
  }

  void reference_position_offset(const Pos& position) override
  {
    _update_object(_state, "ref-pos-offset", _to_list(position));
  }

  void reference_rotation_offset(const Rot& rotation) override
  {
    _update_object(_state, "ref-rot-offset", _to_list(rotation));
  }

  // RendererInformationEvents

  void renderer_name(const std::string& name) override
  {
    _update_object(_state, "renderer-name", name);
  }

  void loudspeakers(const std::vector<Loudspeaker>& loudspeakers) override
  {
    json::Value loudspeaker_list{json::kArrayType};
    loudspeaker_list.Reserve(loudspeakers.size(), _out_allocator);
    for (const auto& loudspeaker: loudspeakers)
    {
      json::Value loudspeaker_object{json::kObjectType};
      _add_member(loudspeaker_object, "pos", _to_list(loudspeaker.position));
      _add_member(loudspeaker_object, "rot", _to_list(loudspeaker.rotation));
      if (loudspeaker.model != "")
      {
        _add_member(loudspeaker_object, "model", loudspeaker.model);
      }
      loudspeaker_list.PushBack(loudspeaker_object.Move(), _out_allocator);
    }
    _update_object(_state, "loudspeakers", loudspeaker_list.Move());
  }

  // TransportFrameEvents

  void transport_frame(uint32_t frame) override
  {
    _update_object(_state, "frame", frame);
  }

  // SourceMetering

  void source_level(id_t id, float level) override
  {
    _set_source_property(id, "level", level);
  }

  // MasterMetering

  void master_level(float level) override
  {
    _update_object(_state, "master-level", level);
  }

  // OutputActivity

  void output_activity(id_t id, float* begin, float* end) override
  {
    json::Value output_activity{json::kArrayType};
    output_activity.Reserve(std::distance(begin, end), _out_allocator);
    for (float* ptr = begin; ptr != end; ++ptr)
    {
      output_activity.PushBack(*ptr, _out_allocator);
    }
    if (!output_activity.Empty())
    {
      _set_source_property(id, "output-activity", output_activity.Move());
    }
  }

  // CpuLoad

  void cpu_load(float load) override
  {
    _update_object(_state, "cpu", load);
  }

  // End of inherited member functions

  /// For std::string, a copy is made
  template<typename T>
  void
  _add_member(json::Value& object, const std::string& key, T&& value)
  {
    assert(object.IsObject());
    // NB: key is copied:
    object.AddMember(json::Value{key, _out_allocator}, std::forward<T>(value)
        , _out_allocator);
  }

  /// For string literals, no copy
  template<std::size_t N, typename T>
  void
  _add_member(json::Value& object, const char (&key)[N], T&& value)
  {
    assert(object.IsObject());
    object.AddMember(json::GenericStringRef<char>{key, N-1}
        , std::forward<T>(value), _out_allocator);
  }

  template<typename K, typename V>
  void
  _update_object(json::Value& object, K&& key, V&& value)
  {
    assert(object.IsObject());
    auto iter = object.FindMember(key);
    if (iter != object.MemberEnd())
    {
      iter->value = std::forward<V>(value);
    }
    else
    {
      _add_member(object, std::forward<K>(key), std::forward<V>(value));
    }
  }

  template<typename K>
  void
  _update_object(json::Value& object, K&& key, const std::string& value)
  {
    _update_object(object, std::forward<K>(key)
        , json::Value{value, _out_allocator});
  }

  template<typename K, typename V>
  void _set_source_property(id_t id, K&& key, V&& value)
  {
    auto iter = _new_source.FindMember(id);
    if (iter != _new_source.MemberEnd())
    {
      _update_object(iter->value, std::forward<K>(key), std::forward<V>(value));
    }
    else
    {
      auto modify_iter = _modify_source.FindMember(id);
      if (modify_iter != _modify_source.MemberEnd())
      {
        _update_object(modify_iter->value, std::forward<K>(key)
                                         , std::forward<V>(value));
      }
      else
      {
        json::Value object{json::kObjectType};
        _update_object(object, std::forward<K>(key), std::forward<V>(value));
        _add_member(_modify_source, id, object.Move());
      }
    }
  }

  json::Value _to_list(const Pos& position)
  {
    json::Value coordinates{json::kArrayType};
    coordinates.Reserve(3, _out_allocator);
    coordinates.PushBack(position.x, _out_allocator);
    coordinates.PushBack(position.y, _out_allocator);
    coordinates.PushBack(position.z, _out_allocator);
    return coordinates;
  }

  json::Value _to_list(const Rot& rotation)
  {
    json::Value quaternion{json::kArrayType};
    quaternion.Reserve(4, _out_allocator);
    quaternion.PushBack(rotation.x, _out_allocator);
    quaternion.PushBack(rotation.y, _out_allocator);
    quaternion.PushBack(rotation.z, _out_allocator);
    quaternion.PushBack(rotation.w, _out_allocator);
    return quaternion;
  }

  json::Value _new_source{json::kObjectType};
  json::Value _modify_source{json::kObjectType};
  json::Value _delete_source{json::kArrayType};
  json::Value _state{json::kObjectType};

  connection_hdl _hdl;
  server_t& _server;
  api::Publisher& _controller;

  std::vector<char> _in_buffer;
  std::vector<char> _out_buffer;
  Allocator _out_allocator;

  // NB: The subscriptions are defined last, they will get destroyed first!
  std::unique_ptr<api::Subscription> _bundle_subscription;
  std::unique_ptr<api::Subscription> _scene_control_subscription;
  std::unique_ptr<api::Subscription> _scene_information_subscription;
  std::unique_ptr<api::Subscription> _renderer_control_subscription;
  std::unique_ptr<api::Subscription> _renderer_information_subscription;
  std::unique_ptr<api::Subscription> _transport_frame_subscription;
  std::unique_ptr<api::Subscription> _source_metering_subscription;
  std::unique_ptr<api::Subscription> _master_metering_subscription;
  std::unique_ptr<api::Subscription> _output_activity_subscription;
  std::unique_ptr<api::Subscription> _cpu_load_subscription;
};


void
Connection::on_message(message_ptr msg)
{
  auto& payload = msg->get_raw_payload();  // writable buffer

  Allocator alloc{_in_buffer.data(), _in_buffer.size()};
  Document command_list{&alloc, 1024, &alloc};

  command_list.ParseInsitu(payload.data());
  if (command_list.HasParseError())
  {
    SSR_ERROR("Unable to parse JSON (error " << command_list.GetParseError()
        << "): " << payload);
    return;
  }
  if (!command_list.IsArray())
  {
    SSR_ERROR("JSON message must be an array, not " << payload);
    return;
  }
  // NB: This can be re-used for multiple consecutive commands, but control has
  //     to be given back during "subscribe" and "unsubscribe":
  std::unique_ptr<api::Controller> control{nullptr};

  auto size{command_list.Size()};
  for (json::SizeType i = 0; i < size; i += 2)
  {
    if (i + 1 == size)
    {
      SSR_ERROR("Expected command/value pairs, not a single " << command_list[i]);
      return;
    }
    const auto& command = command_list[i];
    const auto& value = command_list[i + 1];
    if (command == "subscribe")
    {
      if (!value.IsArray())
      {
        SSR_ERROR("Expected list of subscriptions, not " << value);
        return;
      }
      control.reset();
      auto subscribe = _controller.subscribe();
      for (const auto& subscription: value.GetArray())
      {
        if (subscription == "scene")
        {
          if (_scene_information_subscription || _scene_control_subscription)
          {
            SSR_VERBOSE("Already subscribed: scene");
          }
          else
          {
            // NB: "information" first (to get "new_source"), then "control"
            _scene_information_subscription
              = subscribe->scene_information(this);
            _scene_control_subscription = subscribe->scene_control(this);
          }
        }
        else if (subscription == "renderer")
        {
          if (_renderer_information_subscription
              || _renderer_control_subscription)
          {
            SSR_VERBOSE("Already subscribed: renderer");
          }
          else
          {
            _renderer_information_subscription
              = subscribe->renderer_information(this);
            _renderer_control_subscription = subscribe->renderer_control(this);
          }
        }
        else if (subscription == "transport-frame")
        {
          if (_transport_frame_subscription)
          {
            SSR_VERBOSE("Already subscribed: transport-frame");
          }
          else
          {
            _transport_frame_subscription = subscribe->transport_frame(this);
          }
        }
        else if (subscription == "source-metering")
        {
          if (_source_metering_subscription)
          {
            SSR_VERBOSE("Already subscribed: source-metering");
          }
          else
          {
            _source_metering_subscription = subscribe->source_metering(this);
          }
        }
        else if (subscription == "master-metering")
        {
          if (_master_metering_subscription)
          {
            SSR_VERBOSE("Already subscribed: master-metering");
          }
          else
          {
            _master_metering_subscription = subscribe->master_metering(this);
          }
        }
        else if (subscription == "output-activity")
        {
          if (_output_activity_subscription)
          {
            SSR_VERBOSE("Already subscribed: output-activity");
          }
          else
          {
            _output_activity_subscription = subscribe->output_activity(this);
          }
        }
        else if (subscription == "cpu-load")
        {
          if (_cpu_load_subscription)
          {
            SSR_VERBOSE("Already subscribed: cpu-load");
          }
          else
          {
            _cpu_load_subscription = subscribe->cpu_load(this);
          }
        }
        else
        {
          SSR_ERROR("Unknown subscription: " << subscription);
        }
      }
    }
    else if (command == "unsubscribe")
    {
      if (!value.IsArray())
      {
        SSR_ERROR("Expected list of subscriptions to cancel, not " << value);
        return;
      }
      control.reset();
      for (const auto& subscription: value.GetArray())
      {
        if (subscription == "scene")
        {
          _scene_control_subscription.reset();
          _scene_information_subscription.reset();
        }
        else if (subscription == "renderer")
        {
          _renderer_control_subscription.reset();
          _renderer_information_subscription.reset();
        }
        else if (subscription == "transport-frame")
        {
          _transport_frame_subscription.reset();
        }
        else if (subscription == "source-metering")
        {
          _source_metering_subscription.reset();
        }
        else if (subscription == "master-metering")
        {
          _master_metering_subscription.reset();
        }
        else if (subscription == "output-activity")
        {
          _output_activity_subscription.reset();
        }
        else if (subscription == "cpu-load")
        {
          _cpu_load_subscription.reset();
        }
        else
        {
          SSR_ERROR("Unknown subscription to cancel: " << subscription);
        }
      }
    }
    else if (command == "state")
    {
      if (!value.IsObject())
      {
        SSR_ERROR("Expected JSON object, not " << value);
        return;
      }
      if (!control)
      {
        control = _controller.take_control();
      }
      for (const auto& member: value.GetObject())
      {
        // SceneControlEvents (except source-related ones)
        if (member.name == "auto-rotate-sources")
        {
          if (!member.value.IsBool())
          {
            SSR_ERROR("auto-rotate-sources needs a boolean value, not "
                << member.value);
            return;
          }
          control->auto_rotate_sources(member.value.GetBool());
        }
        else if (member.name == "ref-pos")
        {
          auto pos = internal::parse_position(member.value);
          if (!pos) { return; }
          control->reference_position(*pos);
        }
        else if (member.name == "ref-rot")
        {
          auto rot = internal::parse_rotation(member.value);
          if (!rot) { return; }
          control->reference_rotation(*rot);
        }
        else if (member.name == "master-volume")
        {
          if (!member.value.IsNumber())
          {
            SSR_ERROR("master-volume needs a number, not " << member.value);
            return;
          }
          control->master_volume(member.value.GetDouble());
        }
        else if (member.name == "decay-exponent")
        {
          if (!member.value.IsNumber())
          {
            SSR_ERROR("decay-exponent needs a number, not " << member.value);
            return;
          }
          control->decay_exponent(member.value.GetDouble());
        }
        else if (member.name == "amplitude-reference-distance")
        {
          if (!member.value.IsNumber())
          {
            SSR_ERROR("amplitude-reference-distance needs a number, not "
                << member.value);
            return;
          }
          control->amplitude_reference_distance(member.value.GetDouble());
        }
        // RendererControlEvents
        else if (member.name == "processing")
        {
          if (!member.value.IsBool())
          {
            SSR_ERROR("processing needs a boolean value, not " << member.value);
            return;
          }
          control->processing(member.value.GetBool());
        }
        else if (member.name == "ref-pos-offset")
        {
          auto pos = internal::parse_position(member.value);
          if (!pos) { return; }
          control->reference_position_offset(*pos);
        }
        else if (member.name == "ref-rot-offset")
        {
          auto rot = internal::parse_rotation(member.value);
          if (!rot) { return; }
          control->reference_rotation_offset(*rot);
        }
        // Further events
        else if (member.name == "transport-rolling")
        {
          if (!member.value.IsBool())
          {
            SSR_ERROR("transport-rolling needs a boolean value, not "
                << member.value);
            return;
          }
          if (member.value.GetBool())
          {
            control->transport_start();
          }
          else
          {
            control->transport_stop();
          }
        }
        else if (member.name == "frame")
        {
          if (!member.value.IsInt())
          {
            SSR_ERROR("Invalid transport frame: " << member.value);
            return;
          }
          control->transport_locate_frames(member.value.GetInt());
        }
        else if (member.name == "tracker")
        {
          if (!member.value.IsString()
              || member.value.GetString() != std::string("reset"))
          {
            SSR_ERROR("Invalid value for tracker: " << member.value);
            return;
          }
          control->reset_tracker();
        }
        else
        {
          SSR_ERROR("Unknown property: " << member.name);
        }
      }
    }
    else if (command == "new-src")
    {
      if (!control)
      {
        control = _controller.take_control();
      }
      if (!value.IsArray())
      {
        SSR_ERROR("Expected list of new sources, not " << value);
        return;
      }
      for (const auto& source: value.GetArray())
      {
        if (!source.IsObject())
        {
          SSR_ERROR("Expected list of objects, not " << value);
          return;
        }
        std::string id;
        std::string name;
        std::string model;
        std::optional<std::string> audio_file;
        std::optional<int> channel;
        std::optional<int> port_number;
        std::optional<Pos> position;
        std::optional<Rot> rotation;
        bool fixed{false};
        float volume{1};
        bool mute{false};
        std::string properties_file;
        for (const auto& member: source.GetObject())
        {
          if (member.name == "id")
          {
            if (!member.value.IsString())
            {
              SSR_ERROR("Invalid source ID: " << member.value);
              return;
            }
            id = member.value.GetString();
          }
          else if (member.name == "name")
          {
            if (!member.value.IsString())
            {
              SSR_ERROR("Invalid source name: " << member.value);
              return;
            }
            name = member.value.GetString();
          }
          else if (member.name == "model")
          {
            if (!member.value.IsString())
            {
              SSR_ERROR("Invalid source model: " << member.value);
              return;
            }
            model = member.value.GetString();
          }
          else if (member.name == "audio-file")
          {
            if (!member.value.IsString())
            {
              SSR_ERROR("Invalid audio file name: " << member.value);
              return;
            }
            audio_file = member.value.GetString();
          }
          else if (member.name == "port-number")
          {
            if (!member.value.IsInt())
            {
              SSR_ERROR("Invalid port number: " << member.value);
              return;
            }
            port_number = member.value.GetInt();
          }
          else if (member.name == "channel")
          {
            if (!member.value.IsInt())
            {
              SSR_ERROR("Invalid channel number: " << member.value);
              return;
            }
            channel = member.value.GetInt();
          }
          else if (member.name == "pos")
          {
            position = internal::parse_position(member.value);
            if (!position) { return; }
          }
          else if (member.name == "rot")
          {
            rotation = internal::parse_rotation(member.value);
            if (!rotation) { return; }
          }
          else if (member.name == "fixed")
          {
            if (!member.value.IsBool())
            {
              SSR_ERROR("Invalid fixed state: " << member.value);
              return;
            }
            fixed = member.value.GetBool();
          }
          else if (member.name == "volume")
          {
            if (!member.value.IsNumber())
            {
              SSR_ERROR("Invalid volume: " << member.value);
              return;
            }
            volume = member.value.GetDouble();
          }
          else if (member.name == "mute")
          {
            if (!member.value.IsBool())
            {
              SSR_ERROR("Invalid mute state: " << member.value);
              return;
            }
            mute = member.value.GetBool();
          }
          else if (member.name == "properties-file")
          {
            if (!member.value.IsString())
            {
              SSR_ERROR("Invalid properties file name: " << member.value);
              return;
            }
            properties_file = member.value.GetString();
          }
          else
          {
            SSR_WARNING("Unknown source property: " << member.name);
          }
        }

        if (audio_file && port_number)
        {
          SSR_ERROR("New source cannot have both audio-file and port-number");
          return;
        }
        if (port_number && channel)
        {
          SSR_ERROR("New source cannot have both port-number and channel number");
          return;
        }
        if (!channel)
        {
          channel = audio_file ? 1 : 0;
        }

        std::string file_name_or_port_number;
        if (audio_file)
        {
          file_name_or_port_number = *audio_file;
        }
        else if (port_number)
        {
          file_name_or_port_number = apf::str::A2S(*port_number);
        }
        else
        {
          SSR_ERROR("Either audio-file or port-number needed for new source");
          return;
        }
        control->new_source(id, name, model, file_name_or_port_number, *channel
            , position.value_or(Pos{}), rotation.value_or(Rot{}), fixed, volume
            , mute, properties_file);
      }
    }
    else if (command == "mod-src")
    {
      if (!control)
      {
        control = _controller.take_control();
      }
      for (const auto& source: value.GetObject())
      {
        if (!source.value.IsObject())
        {
          SSR_ERROR("Source must be a JSON object, not " << source.value);
          return;
        }
        assert(source.name.IsString());
        const std::string id = source.name.GetString();
        for (const auto& attr: source.value.GetObject())
        {
          // source-related SceneControlEvents (except delete_source)
          if (attr.name == "pos")
          {
            auto pos = internal::parse_position(attr.value);
            if (!pos) { return; }
            control->source_position(id, *pos);
          }
          else if (attr.name == "rot")
          {
            auto rot = internal::parse_rotation(attr.value);
            if (!rot) { return; }
            control->source_rotation(id, *rot);
          }
          else if (attr.name == "volume")
          {
            if (!attr.value.IsNumber())
            {
              SSR_ERROR("Invalid source volume: " << attr.value);
              return;
            }
            control->source_volume(id, attr.value.GetDouble());
          }
          else if (attr.name == "mute")
          {
            if (!attr.value.IsBool())
            {
              SSR_ERROR("Invalid source mute state: " << attr.value);
              return;
            }
            control->source_mute(id, attr.value.GetBool());
          }
          else if (attr.name == "name")
          {
            if (!attr.value.IsString())
            {
              SSR_ERROR("Invalid source name: " << attr.value);
              return;
            }
            control->source_name(id, attr.value.GetString());
          }
          else if (attr.name == "model")
          {
            if (!attr.value.IsString())
            {
              SSR_ERROR("Invalid source model: " << attr.value);
              return;
            }
            control->source_model(id, attr.value.GetString());
          }
          else if (attr.name == "fixed")
          {
            if (!attr.value.IsBool())
            {
              SSR_ERROR("Invalid source fixed state: " << attr.value);
              return;
            }
            control->source_fixed(id, attr.value.GetBool());
          }
        }
      }
    }
    else if (command == "del-src")
    {
      if (!value.IsArray())
      {
        SSR_ERROR("Expected list of source IDs to delete, not " << value);
        return;
      }
      if (!control)
      {
        control = _controller.take_control();
      }
      for (const auto& delinquent: value.GetArray())
      {
        if (!delinquent.IsString())
        {
          SSR_ERROR("Expected string ID of source to delete, not " << delinquent);
          return;
        }
        control->delete_source(delinquent.GetString());
      }
    }
    else if (command == "load-scene")
    {
      if (!value.IsString())
      {
        SSR_ERROR("Expected scene file name, not " << value);
        return;
      }
      if (!control)
      {
        control = _controller.take_control();
      }
      try
      {
        control->load_scene(value.GetString());
      }
      catch (std::exception& e)
      {
        SSR_ERROR("Couldn't load scene: " << e.what());
      }
    }
    else if (command == "save-scene")
    {
      SSR_WARNING("save-scene not implemented (yet?)");
    }
    else if (command == "time")
    {
      SSR_WARNING("Message timestamps might be implemented in the future ...");
    }
    else
    {
      SSR_ERROR("Unknown command: " << command);
      return;
    }
  }
  SSR_VERBOSE3("input allocator usage: " << alloc.Size());
}

}  // namespace ws

}  // namespace ssr

#endif
