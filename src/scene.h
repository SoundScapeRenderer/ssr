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

#include <map>
#include <vector>
#include <cassert> // for assert()
#include <memory>

#include "subscriber.h"
#include "maptools.h" // for get_item()
#include "source.h"
#include "loudspeaker.h"

namespace ssr
{

/// %Scene. Contains current location and other data about sources, the
// reference point and other things.
class Scene : public Subscriber
{
  public:
    /// A map of sources.
    using source_map_t = std::map<id_t, Source>;
    /// A vector of loudspeakers.
    using loudspeakers_t = std::vector<Loudspeaker>;

    Scene();  ///< ctor
    ~Scene(); ///< dtor

    // from Subscriber
    virtual void set_loudspeakers(const Loudspeaker::container_t& container);
    virtual void new_source(id_t id);
    virtual void delete_source(id_t id);
    virtual void delete_all_sources();
    virtual bool set_source_position(id_t id, const Position& position);
    virtual bool set_source_orientation(id_t id
        , const Orientation& orientation);
    virtual bool set_source_gain(id_t id, const float& gain);

    virtual bool set_source_signal_level(id_t id, const float& level);

    virtual bool set_source_mute(id_t id, const bool& mute);
    virtual bool set_source_name(id_t id, const std::string& name);
    virtual bool set_source_properties_file(id_t id, const std::string& name);
    virtual bool set_source_model(id_t id, const Source::model_t& model);
    virtual bool set_source_port_name(id_t id, const std::string& port_name);
    virtual bool set_source_file_name(id_t id, const std::string& file_name);
    virtual bool set_source_file_channel(id_t id, const int& file_channel);
    virtual bool set_source_position_fixed(id_t id, const bool& fixed);
    virtual bool set_source_file_length(id_t id, const long int& length);

    virtual void set_reference_position(const Position& position);
    virtual void set_reference_orientation(const Orientation& orientation);

    virtual void set_reference_offset_position(const Position& position);
    virtual void set_reference_offset_orientation(const Orientation& orientation);

    virtual void set_master_volume(float volume);

    virtual void set_decay_exponent(float exponent);

    virtual void set_amplitude_reference_distance(float dist);

    virtual void set_master_signal_level(float level);

    virtual void set_cpu_load(float load);

    virtual void set_sample_rate(int sample_rate);

    virtual void set_source_output_levels(id_t id, float* first, float* last);

    virtual void set_processing_state(bool state);
    //virtual void set_transport_state(JackClient::State state);
    virtual void set_transport_state(
        const std::pair<bool, jack_nframes_t>& state);

    virtual void set_auto_rotation(bool auto_rotate_sources);

    loudspeakers_t::size_type get_number_of_loudspeakers() const;

    //    std::pair<bool, jack_nframes_t> get_transport_state() const;

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
    Source get_source(id_t id) const;
    /// get source position
    std::unique_ptr<Position> get_source_position(id_t id) const;
    /// get source orientation
    std::unique_ptr<Orientation> get_source_orientation(id_t id) const;
    /// get source model (=type)
    Source::model_t get_source_model(id_t id) const;
    /// get source gain
    float get_source_gain(id_t id) const;
    /// get source mute state
    bool get_source_mute_state(id_t id) const;
    /// get source name
    std::string get_source_name(id_t id) const;

    bool get_source_position_fixed(id_t id) const;

    std::string get_source_properties_file(id_t id) const;

    /// check if renderer is processing
    bool get_processing_state() const;
    bool is_playing() const;
    jack_nframes_t get_transport_position() const;

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
    /// conversion constructor for the Source type. Speaking more exactly, you
    /// need a constructor of the following form:
    /// <code>T::T(const pair<id_t,Source>&)</code>
    /// @par
    /// Your type @a T can include any of the members of Source, this way you
    /// can obtain any subset of data you desire. If you want all available
    /// data, just use the Source type itself.
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
      // of the Scene class and defined further down this file.

      if (container_traits<Container<T, Args...>>::has_reserve)
      {
        container_traits<Container<T, Args...>>::reserve(container,
            _source_map.size());
      }
      for (const auto& source: _source_map)
      {
        // type conversion constructor T::T(const pair<id_t,Source>&) needed!
        container.push_back(T(source));
      }
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

  protected:
    source_map_t _source_map;     ///< container for sources

  private:
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
    int _sample_rate;  ///< sample rate
    /// order of mirror sources. if 0, mirror sources are deactivated
    int mirror_order;
    bool _processing_state;  ///< is renderer processing?
    bool           _transport_playing;
    jack_nframes_t _transport_position; ///< current position in the audio file in samples
    bool _auto_rotate_sources;

    template<typename T, typename PointerToMember> bool _set_source_member(
        id_t id, PointerToMember member, const T& arg)
    {
      auto source_ptr = maptools::get_item(_source_map, id);
      if (!source_ptr)
      {
        VERBOSE("Source " << id << " doesn't exist!");
        return false;
      }
      source_ptr->*member = arg;
      return true;
    }

    /// helper function template for get_*()
    template<typename T, typename PointerToMember> bool _get_source_member(
        id_t id, PointerToMember member, T& arg) const
    {
      const Source* const source_ptr = maptools::get_item(_source_map, id);
      if (!source_ptr)
      {
        VERBOSE("Source " << id << " doesn't exist!");
        return false;
      }
      arg = source_ptr->*member;
      return true;
    }

    // forward declaration of the container traits class
    template<typename Container> struct container_traits; // nested
    // partial specialization:
    template<typename T>         struct container_traits<std::vector<T>>;
};

////////////////////////////////////////////////////////////////////////////////
// Definition of the nested struct container_traits
////////////////////////////////////////////////////////////////////////////////

/// traits class to check if a container has a reserve() member function
// default template for any type of container
template<typename Container> struct Scene::container_traits
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
struct Scene::container_traits<std::vector<T>>
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
