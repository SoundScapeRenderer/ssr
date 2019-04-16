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
/// Legacy scene class (definition), superseded by Scene.

#ifndef SSR_LEGACY_SCENE_H
#define SSR_LEGACY_SCENE_H

#include <map>
#include <vector>
#include <cassert> // for assert()
#include <memory>

#include "apf/stringtools.h"  // for S2RV()
#include "api.h"
#include "maptools.h" // for get_item()
#include "legacy_source.h"
#include "legacy_loudspeaker.h"
#include "ssr_global.h"  // for VERBOSE()

namespace ssr
{

/// Legacy scene. Contains current location and other data about sources, the
/// reference point and other things.
class LegacyScene : public api::SceneControlEvents
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
    using legacy_id_t = unsigned int;

    /// A map of sources.
    using source_map_t = std::map<legacy_id_t, LegacySource>;
    /// A vector of loudspeakers.
    using loudspeakers_t = std::vector<LegacyLoudspeaker>;

    LegacyScene();  ///< ctor

    loudspeakers_t::size_type get_number_of_loudspeakers() const;

    /// get master volume
    float get_master_volume() const;

    /// get master volume with amplitude correction considered
    float get_corrected_master_volume() const;

    /// get amplitude decay exponent
    float get_decay_exponent() const;

    /// get amplitude reference distance
    float get_amplitude_reference_distance() const;

    /// get master audio level
    float get_master_signal_level() const;

    /// get CPU load in percent estimated by JACK
    float get_cpu_load() const;

    /// get current sample rate
    int get_sample_rate() const;

    /// get source
    LegacySource get_source(legacy_id_t id) const;
    /// get source position
    std::unique_ptr<Position> get_source_position(legacy_id_t id) const;
    /// get source orientation
    std::unique_ptr<Orientation> get_source_orientation(legacy_id_t id) const;
    /// get source model (=type)
    LegacySource::model_t get_source_model(legacy_id_t id) const;
    /// get source gain
    float get_source_gain(legacy_id_t id) const;
    /// get source mute state
    bool get_source_mute_state(legacy_id_t id) const;
    /// get source name
    std::string get_source_name(legacy_id_t id) const;

    bool get_source_position_fixed(legacy_id_t id) const;

    std::string get_source_properties_file(legacy_id_t id) const;

    /// check if renderer is processing
    bool get_processing_state() const;
    bool is_playing() const;
    uint32_t get_transport_position() const;

    bool get_auto_rotation() const;

    // temporarily with inline implementation
    // should return 0 in case of doubt.
    // will be removed in the near future
    /// deprecated!
    inline virtual int get_max_no_of_sources() const
    {
      return 100;
    }

    /// get a list of all sources.
    /// @param container (initially empty) list of sources.
    /// @pre
    /// For the template argument @a T you can use any type which has a
    /// conversion constructor for the LegacySource type. Speaking more exactly, you
    /// need a constructor of the following form:
    /// <code>T::T(const pair<legacy_id_t,LegacySource>&)</code>
    /// @par
    /// Your type @a T can include any of the members of LegacySource, this way you
    /// can obtain any subset of data you desire. If you want all available
    /// data, just use the LegacySource type itself.
    /// @warning if a std::vector is used as container, the function
    /// <code>get_number_of_sources()</code> is called to reserve the
    /// necessary memory to avoid memory re-allocation.
    ///
    /// Because a fancy template template is used, any container type with one
    /// template argument can be used, like std::list, std::vector, ...
    /// as long as it has the following member functions: .begin(), .end(),
    /// .push_back().
    template<template<typename, typename...> class Container, typename T,
    typename... Args>
    void get_sources(Container<T, Args...>& container) const
    {
      assert(container.empty());

      // the following struct container_traits is declared in the private part
      // of the LegacyScene class and defined further down this file.

      if (container_traits<Container<T, Args...>>::has_reserve)
      {
        container_traits<Container<T, Args...>>::reserve(container,
            _source_map.size());
      }
      for (const auto& source: _source_map)
      {
        // type conversion constructor T::T(const pair<legacy_id_t,LegacySource>&) needed!
        container.push_back(T(source));
      }
    }


    template<typename F>
    void for_each_source(F f) const
    {
      for (const auto& [id, source]: _source_map) { f(id, source); }
    }


    /// get a list of all loudspeakers.
    /// @warning This doesn't return a valid value for the "active" field! (if
    /// your type T has it)
    ///
    /// ID of the loudspeaker == its position in the container
    /// container[0] has ID 1, container[1] has ID 2, ...
    template<template<typename, typename> class Container, typename T,
    typename Allocator>
    void get_loudspeakers(Container<T, Allocator>& container,
        bool absolute_position = true) const
    {
      assert(container.empty());

      if (container_traits<Container<T, Allocator>>::has_reserve)
      {
        container_traits<Container<T, Allocator>>::reserve(container,
            _loudspeakers.size());
      }

      for (loudspeakers_t::const_iterator i = _loudspeakers.begin();
          i != _loudspeakers.end(); ++i)
      {
        T temp(*i);                // copy ctor. is called
        if (absolute_position)
        {
          // T has to be derived from DirectionalPoint
          temp.DirectionalPoint::operator=(temp.transform(_reference));
        }
        container.push_back(temp); // copy ctor. is called again
      }
    }

    /// get current reference position/orientation.
    /// @return position/orientation of the reference point.
    DirectionalPoint get_reference() const
    {
      return _reference;
    }

    DirectionalPoint get_reference_offset() const
    {
      return _reference_offset;
    }

    bool show_head() const
    {
      return _loudspeakers.empty();
    }

    std::string get_renderer_name() const
    {
      return _renderer_name;
    }

    std::string string_id(legacy_id_t id) const
    {
      for (const auto& [str, num]: _source_id_map)
      {
        if (num == id)
        {
          return str;
        }
      }
      throw std::runtime_error("Source doesn't exist");
    }

  protected:
    source_map_t _source_map;     ///< container for sources
    /// Mapping from string IDs to legacy numeric IDs.
    std::map<std::string, legacy_id_t, std::less<>> _source_id_map;

  private:
    // from SceneControlEvents

    void auto_rotate_sources(bool auto_rotate) override
    {
      _auto_rotate_sources = auto_rotate;
    }

    void delete_source(id_t id) override
    {
      if (auto iter = _source_id_map.find(id); iter != _source_id_map.end())
      {
        VERBOSE("LegacyScene: deleting source \"" << iter->first << "\"");
        // this should call the destructor for the LegacySource object.
        // IMPORTANT: the map holds the Sources directly, no pointers!
        _source_map.erase(iter->second);
        _source_id_map.erase(iter->first);
      }
    }

    void source_position(id_t id, const Pos& position) override
    {
      _set_source_member(id, &LegacySource::position, position);
    }

    void source_rotation(id_t id, const Rot& rotation) override
    {
      _set_source_member(id, &LegacySource::orientation, rotation);
    }

    void source_volume(id_t id, float volume) override
    {
      _set_source_member(id, &LegacySource::gain, volume);
    }

    void source_mute(id_t id, bool mute) override
    {
      _set_source_member(id, &LegacySource::mute, mute);
    }

    void source_name(id_t id, const std::string& name) override
    {
      _set_source_member(id, &LegacySource::name, name);
    }

    void source_model(id_t id, const std::string& model) override
    {
      _set_source_member(id, &LegacySource::model
          , apf::str::S2RV(model, LegacySource::unknown));
    }

    void source_fixed(id_t id, bool fixed) override
    {
      _set_source_member(id, &LegacySource::fixed_position, fixed);
    }

    void reference_position(const Pos& position) override
    {
      _reference.position = position;
    }

    void reference_rotation(const Rot& rotation) override
    {
      _reference.orientation = rotation;
    }

    void master_volume(float volume) override
    {
      _master_volume = volume;
    }

    void decay_exponent(float exponent) override
    {
      _decay_exponent = exponent;
    }

    void amplitude_reference_distance(float distance) override
    {
      _amplitude_reference_distance = distance;
    }

    // from SceneInformationEvents

    void sample_rate(int rate) override
    {
      _sample_rate = rate;
    }

    void new_source(id_t id) override
    {
      auto [iter, inserted] = _source_id_map.try_emplace(id, _next_source_id);
      if (inserted)
      {
        ++_next_source_id;

        VERBOSE("LegacyScene: Adding source \"" << iter->first
            << "\" to source map");
        _source_map[iter->second] = LegacySource(_loudspeakers.size());
      }
      else
      {
        WARNING("LegacyScene: Source ID \"" << id << "\" already exists");
      }
    }

    virtual void source_property(id_t id, const std::string& key
                                        , const std::string& value) override
    {
      if (key == "audio-file")
      {
        _set_source_member(id, &LegacySource::audio_file_name, value);
      }
      else if (key == "audio-file-channel")
      {
        _set_source_member(id, &LegacySource::audio_file_channel
            , apf::str::S2RV(value, 0));
      }
      else if (key == "audio-file-length")
      {
        _set_source_member(id, &LegacySource::file_length
            , apf::str::S2RV(value, 0));
      }
      else if (key == "port-name")
      {
        _set_source_member(id, &LegacySource::port_name, value);
      }
      else if (key == "properties-file")
      {
        _set_source_member(id, &LegacySource::properties_file, value);
      }
      else
      {
        WARNING("LegacyScene: Unknown property: \"" << key << "\"");
      }
    }

    void transport_rolling(bool rolling) override
    {
      _transport_playing = rolling;
    }

    // from RendererControlEvents

    void processing(bool processing_state) override
    {
      _processing_state = processing_state;
    }

    void reference_position_offset(const Pos& position) override
    {
      _reference_offset.position = position;
    }

    void reference_rotation_offset(const Rot& rotation) override
    {
      Orientation orientation(rotation);
      orientation.azimuth -= 90.0f;  // Undo angle conversion offset
      _reference_offset.orientation = orientation;
    }

    // from RendererInformationEvents

    void renderer_name(const std::string& name) override
    {
      _renderer_name = name;
    }

    void loudspeakers(const std::vector<Loudspeaker>& loudspeakers) override
    {
      _loudspeakers.assign(loudspeakers.begin(), loudspeakers.end());
    }

    // from TransportFrameEvents

    void transport_frame(uint32_t frame) override
    {
      _transport_position = frame;
    }

    // from SourceMetering

    void source_level(id_t id, float level) override
    {
      _set_source_member(id, &LegacySource::signal_level, level);
    }

    // from MasterMetering

    void master_level(float level) override
    {
      _master_signal_level = level;
    }

    // from OutputActivity

    void output_activity(id_t id, float* begin, float* end) override
    {
      if (auto iter = _source_id_map.find(id); iter != _source_id_map.end())
      {
        if (auto source_ptr = maptools::get_item(_source_map, iter->second)
            ; source_ptr)
        {
          assert(source_ptr);
          if (source_ptr->output_levels.size()
              == size_t(std::distance(begin, end)))
          {
            std::copy(begin, end, source_ptr->output_levels.begin());
          }
          else
          {
            VERBOSE("LegacyScene: Wrong number of outputs");
          }
        }
      }
      else
      {
        VERBOSE("LegacyScene: Source ID \"" << id << "\" doesn't exist");
      }
    }

    // from CpuLoad

    void cpu_load(float load) override
    {
      _cpu_load = load;
    }

    loudspeakers_t _loudspeakers;
    DirectionalPoint _reference;  ///< position/orientation of the reference
    DirectionalPoint _reference_offset;
    float _master_volume;         ///< master volume (linear)
    float _master_volume_correction; ///< dito (linear scale)
    float _decay_exponent; ///< dito
    float _amplitude_reference_distance; ///< distance where plane sources are
                                         ///< as loud as the other source types
    float _master_signal_level; ///< instantaneous overall signal level (linear)
    float _cpu_load;              ///< CPU load in percent
    std::string _renderer_name;
    int _sample_rate;  ///< sample rate
    /// order of mirror sources. if 0, mirror sources are deactivated
    int mirror_order;
    bool _processing_state;  ///< is renderer processing?
    bool           _transport_playing;
    uint32_t _transport_position; ///< current position in the audio file in samples
    bool _auto_rotate_sources;

    template<typename T, typename PointerToMember>
    void _set_source_member(id_t id, PointerToMember member, T&& arg)
    {
      if (auto iter = _source_id_map.find(id); iter != _source_id_map.end())
      {
        if (auto source_ptr = maptools::get_item(_source_map, iter->second)
            ; source_ptr)
        {
          source_ptr->*member = std::forward<T>(arg);
          return;
        }
      }
      VERBOSE("LegacyScene: Source ID \"" << id << "\" doesn't exist!");
    }

    /// helper function template for get_*()
    template<typename T, typename PointerToMember> bool _get_source_member(
        legacy_id_t id, PointerToMember member, T& arg) const
    {
      const LegacySource* source_ptr = maptools::get_item(_source_map, id);
      if (!source_ptr)
      {
        VERBOSE("LegacyScene: Source " << id << " doesn't exist!");
        return false;
      }
      arg = source_ptr->*member;
      return true;
    }

    // forward declaration of the container traits class
    template<typename Container> struct container_traits; // nested
    // partial specialization:
    template<typename T>         struct container_traits<std::vector<T>>;

    unsigned int _next_source_id{1};
};

////////////////////////////////////////////////////////////////////////////////
// Definition of the nested struct container_traits
////////////////////////////////////////////////////////////////////////////////

/// traits class to check if a container has a reserve() member function
// default template for any type of container
template<typename Container> struct LegacyScene::container_traits
{
  static const bool has_reserve = false;
  template<typename U> static void reserve(Container&, const U&)
  {
    // does nothing, because generally, there is no reserve() member
    // function an we will never call it anyway.
    // We only declare it to get no errors from the compiler.
  }
};

/// traits class to check if a container has a reserve() member function
// partial specialization of the above template.
template<typename T>
struct LegacyScene::container_traits<std::vector<T>>
{
  static const bool has_reserve = true;
  template<typename U> static void reserve(std::vector<T>& v, const U& space)
  {
    v.reserve(space);
  }
};
// if any other STL container has a reserve() member function it should be
// added here as a specialization.

// TODO: allow Allocator template argument with default std::allocator<T>
// but: default template arguments may not be used in partial specializations
// introducing this argument into the default template as well would make this
// already quite complicated template thingy even more complicated!

}  // namespace ssr

#endif
