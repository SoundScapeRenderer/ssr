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
/// %Scene class (definition).

#ifndef SSR_SCENE_H
#define SSR_SCENE_H

#include <cassert> // for assert()
#include <map>
#include <vector>

#include "api.h"
#include "ssr_global.h"  // for VERBOSE()

namespace ssr
{

/// %Scene. Contains current location and other data about sources, the
/// reference point and other things.
class Scene : public api::SceneControlEvents
            , public api::SceneInformationEvents
{
public:
  struct Source
  {
    Pos position{};
    Rot rotation{};
    bool fixed{false};  ///< Static position/rotation or not

    std::string name;
    float volume{1};
    bool mute{false};
    /// Source model (=type).  Most renderers support "point" and "plane".
    /// Theoretically, other models could be supported, like "line",
    /// "directional", "extended", ...
    std::string model{"point"};

    std::map<std::string, std::string, std::less<>> properties;
  };

  template<typename F>
  void for_each_source(F f) const
  {
    for (const auto& [id, source]: _source_map) { f(id, source); }
  }

  const Source* get_source(id_t id) const
  {
    auto item = _source_map.find(id);
    if (item == _source_map.end()) { return nullptr; }
    return &(item->second);
  }

  /// Get string ID given one-based source number.
  /// @see api::Controller::get_source_id()
  std::string get_source_id(unsigned source_number) const
  {
    if (!source_number || source_number > _source_ids.size()) { return {}; }
    return _source_ids[source_number - 1];
  }

  /// Get the one-based source number given a string ID.
  /// @see api::Controller::get_source_number()
  unsigned int get_source_number(id_t source_id) const
  {
    auto result = std::find(_source_ids.begin(), _source_ids.end(), source_id);
    if (result == _source_ids.end())
    {
      return 0;
    }
    return result - _source_ids.begin() + 1;
  }

  Pos get_reference_position() const { return _reference_position; }
  Rot get_reference_rotation() const { return _reference_rotation; }

  bool get_auto_rotation() const
  {
    return _auto_rotate_sources;
  }

  void get_data(SceneControlEvents* subscriber)
  {
    assert(subscriber);
    subscriber->auto_rotate_sources(_auto_rotate_sources);
    subscriber->reference_position(_reference_position);
    subscriber->reference_rotation(_reference_rotation);
    subscriber->master_volume(_master_volume);
    // TODO: master volume correction?
    subscriber->decay_exponent(_decay_exponent);
    subscriber->amplitude_reference_distance(_amplitude_reference_distance);

    this->for_each_source([subscriber](auto id, auto& source) {
        subscriber->source_position(id, source.position);
        subscriber->source_rotation(id, source.rotation);
        subscriber->source_volume(id, source.volume);
        subscriber->source_mute(id, source.mute);
        subscriber->source_name(id, source.name);
        subscriber->source_model(id, source.model);
        subscriber->source_fixed(id, source.fixed);
    });
  }

  void get_data(SceneInformationEvents* subscriber)
  {
    this->for_each_source([subscriber](auto id, auto& source) {
        // Sources have to be created first, then they can be updated
        subscriber->new_source(id);
        for (const auto& item: source.properties)
        {
          subscriber->source_property(id, item.first, item.second);
        }
    });
  }

private:
  template<typename PTM, typename T>
  void _set_source_member(id_t id, PTM member, T&& arg)
  {
    auto src = _source_map.find(id);
    if (src == _source_map.end())
    {
      WARNING("Source \"" << id << "\" doesn't exist!");
    }
    else
    {
      src->second.*member = std::forward<T>(arg);
    }
  }

  // SceneControlEvents

  void auto_rotate_sources(bool auto_rotate) override
  {
    _auto_rotate_sources = auto_rotate;
  }

  void delete_source(id_t id) override
  {
    auto erased = _source_map.erase(id);
    assert(erased < 2);
    if (erased == 0)
    {
      WARNING("Source \"" << id << "\" not available, couldn't delete");
    }
    else
    {
      std::remove(_source_ids.begin(), _source_ids.end(), id);
    }
  }

  void source_position(id_t id, const Pos& position) override
  {
    _set_source_member(id, &Source::position, position);
  }

  void source_rotation(id_t id, const Rot& rotation) override
  {
    _set_source_member(id, &Source::rotation, rotation);
  }

  void source_volume(id_t id, float volume) override
  {
    _set_source_member(id, &Source::volume, volume);
  }

  void source_mute(id_t id, bool mute) override
  {
    _set_source_member(id, &Source::mute, mute);
  }

  void source_name(id_t id, const std::string& name) override
  {
    _set_source_member(id, &Source::name, name);
  }

  void source_model(id_t id, const std::string& model) override
  {
    _set_source_member(id, &Source::model, model);
  }

  void source_fixed(id_t id, bool fixed) override
  {
    _set_source_member(id, &Source::fixed, fixed);
  }

  void reference_position(const Pos& position) override
  {
    _reference_position = position;
  }

  void reference_rotation(const Rot& rotation) override
  {
    _reference_rotation = rotation;
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

  // SceneInformationEvents

  void new_source(id_t id) override
  {
    VERBOSE("Adding source \"" << id << "\" to source map");
    auto [iter, inserted] = _source_map.try_emplace(id);
    if (inserted)
    {
      _source_ids.push_back(id);
    }
    else
    {
      ERROR("Source \"" << id << "\" already existed in the source map");
    }
  }

  void source_property(id_t id, const std::string& key
      , const std::string& value) override
  {
    auto src = _source_map.find(id);
    if (src == _source_map.end())
    {
      ERROR("Source \"" << id << "\" doesn't exist!");
    }
    else
    {
      src->second.properties[key] = value;
    }
  }

  std::map<std::string, Source, std::less<>> _source_map;
  /// List of source IDs, ordered by creation time.
  std::vector<std::string> _source_ids;

  Pos _reference_position{};
  Rot _reference_rotation{};

  float _master_volume{1};  ///< master volume (linear)
  float _master_volume_correction{1};  ///< dito (linear scale)
  float _decay_exponent{1};  ///< dito
  /// distance where plane sources are as loud as the other source types
  float _amplitude_reference_distance{3};

  bool _auto_rotate_sources{true};

  // TODO: this should be removed at some point:
  friend class LegacySceneWrapper;
};

}  // namespace ssr

#endif
