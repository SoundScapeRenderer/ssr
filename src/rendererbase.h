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
/// Renderer base class.

#ifndef SSR_RENDERERBASE_H
#define SSR_RENDERERBASE_H

#include <string>
#include <cstring>  // for std::strcmp()

#include "apf/mimoprocessor.h"
#include "apf/shareddata.h"
#include "apf/container.h"  // for distribute_list()
#include "apf/parameter_map.h"
#include "apf/math.h"  // for dB2linear()

#include "maptools.h"
#include "geometry.h"  // for vec3

#ifdef ENABLE_DYNAMIC_ASDF
#include "dynamic_scene.h"
#endif

#ifndef SSR_QUERY_POLICY
#define SSR_QUERY_POLICY apf::disable_queries
#endif

namespace ssr
{

/** Renderer base class.
 * The parallel rendering engine uses the non-blocking datastructure RtList to
 * communicate between realtime and non-realtime threads.
 * All non-realtime accesses to RtList%s have to be locked with
 * get_scoped_lock() to ensure single-reader/single-writer operation.
 **/
template<typename Derived>
class RendererBase : public apf::MimoProcessor<Derived
                     , APF_MIMOPROCESSOR_INTERFACE_POLICY
                     , SSR_QUERY_POLICY>
{
  private:
    using _base = apf::MimoProcessor<Derived, APF_MIMOPROCESSOR_INTERFACE_POLICY
      , SSR_QUERY_POLICY>;

  public:
    using typename _base::rtlist_t;
    using typename _base::ScopedLock;
    using typename _base::sample_type;

    using _base::_fifo;

    class Source;
    // TODO: try to remove this:
    using SourceBase = Source;
    class Output;

#ifdef SSR_SHARED_IO_BUFFERS
    // Copying the input buffers is only needed if the backend re-uses its input
    // buffers as output buffers (e.g. Puredata externals) and if the renderer
    // doesn't copy the buffers anyway at some point (as most renderers do).
    class Input : public _base::Input
    {
      public:
        using iterator = typename std::vector<sample_type>::const_iterator;
        using typename _base::Input::Params;

        explicit Input(const Params& p)
          : _base::Input(p)
          , _buffer(this->parent.block_size())
        {}

        APF_PROCESS(Input, _base::Input)
        {
          std::copy(this->buffer.begin(), this->buffer.end(), _buffer.begin());
        }

        iterator begin() const { return _buffer.begin(); }
        iterator end() const { return _buffer.end(); }

      private:
        std::vector<sample_type> _buffer;
    };
#else
    using Input = typename _base::DefaultInput;
#endif


    struct State
    {
      State(apf::CommandQueue& fifo, const apf::parameter_map& params)
        : reference_position(fifo)
        , reference_rotation(fifo)
        , reference_position_offset(fifo)
        , reference_rotation_offset(fifo)
        , master_volume(fifo, 1)
        , processing(fifo, true)
        , decay_exponent(fifo, params.get<sample_type>("decay_exponent", 1))
        , amplitude_reference_distance(fifo
            , params.get<sample_type>("amplitude_reference_distance", 3))
      {}

      apf::SharedData<Pos> reference_position;
      apf::SharedData<Rot> reference_rotation;
      apf::SharedData<Pos> reference_position_offset;
      apf::SharedData<Rot> reference_rotation_offset;
      apf::SharedData<sample_type> master_volume;
      apf::SharedData<bool> processing;
      apf::SharedData<sample_type> decay_exponent;
      apf::SharedData<sample_type> amplitude_reference_distance;
    } state;

    // If you don't need a list proxy, just use a reference to the list
    template<typename L, typename ListProxy, typename DataMember>
    class AddToSublistCommand : public apf::CommandQueue::Command
    {
      public:
        AddToSublistCommand(L input, ListProxy output, DataMember member)
          : _input(input)
          , _output(output)
          , _member(member)
        {}

        virtual void execute()
        {
          apf::distribute_list(_input, _output, _member);
        }

        // Empty function, because no cleanup is necessary.
        virtual void cleanup() {}

      private:
        L _input;
        ListProxy _output;
        DataMember _member;
    };

#ifdef ENABLE_DYNAMIC_ASDF
    struct Process;

    std::vector<DynamicSourceInfo>
    load_dynamic_scene(const std::string& scene_file_name
        , const std::string& input_port_prefix);
#endif

    template<typename L, typename ListProxy, typename DataMember>
    void add_to_sublist(const L& input, ListProxy output, DataMember member)
    {
      _fifo.push(new AddToSublistCommand<L, ListProxy, DataMember>(
            input, output, member));
    }

    template<typename L, typename ListProxy, typename DataMember>
    class RemFromSublistCommand : public apf::CommandQueue::Command
    {
      public:
        RemFromSublistCommand(const L input, ListProxy output
            , DataMember member)
          : _input(input)
          , _output(output)
          , _member(member)
        {}

        virtual void execute()
        {
          apf::undistribute_list(_input, _output, _member, _garbage);
        }

        virtual void cleanup()
        {
          // Nothing to be done. _garbage is taken care of in the destructor.
        }

      private:
        const L _input;
        ListProxy _output;
        DataMember _member;
        typename std::remove_const<L>::type _garbage;
    };

    template<typename L, typename ListProxy, typename DataMember>
    void rem_from_sublist(const L& input, ListProxy output, DataMember member)
    {
      _fifo.push(new RemFromSublistCommand<L, ListProxy, DataMember>(
            input, output, member));
    }

    std::string add_source(id_t id
        , const apf::parameter_map& p = apf::parameter_map()
        , const float* file_source_ptr = nullptr);
    void rem_source(id_t id);
    void rem_all_sources();

    Source* get_source(id_t id);

    // May only be used in realtime thread!
    const rtlist_t& get_source_list() const { return _source_list; }

    bool show_head() const { return _show_head; }

    // TODO: proper solution for getting the reproduction setup
    template<typename SomeListType>
    void get_loudspeakers(SomeListType&) const {}

    auto get_scoped_lock() { return std::make_unique<ScopedLock>(_lock); }

    const sample_type master_volume_correction;  // linear

#ifdef ENABLE_DYNAMIC_ASDF
    using dynamic_source_list_t = std::vector<std::optional<ssr::Transform>>;
    // NB: It's probably not a good idea to make this public:
    apf::SharedData<std::unique_ptr<dynamic_source_list_t>> dynamic_sources;
    // NB: This is only ever accessed from the realtime thread:
    Transform dynamic_reference{};

    bool freewheeling;
#endif

  protected:
    RendererBase(const apf::parameter_map& p);

    // TODO: make private?
    sample_type _master_level;

    rtlist_t _source_list;

    // TODO: find a better solution to get loudspeaker vs. headphone renderer
    bool _show_head;

  private:
    apf::parameter_map _add_params(const apf::parameter_map& params)
    {
      auto temp = params;
      temp.set("name", params.get("name", Derived::name()));
      return temp;
    }

    std::map<std::string, Source*, std::less<>> _source_map;

    size_t _next_id_suffix = 0;

    std::mutex _lock;

#ifdef ENABLE_DYNAMIC_ASDF
    int jack_sync_callback(jack_transport_state_t
        , jack_position_t *pos) override
    {
      if (_scene == nullptr)
      {
        return 1;
      }
      return _scene.get()->seek(pos->frame);
    }

    apf::SharedData<std::unique_ptr<DynamicScene>> _scene;
    uint64_t usleeptime;
#endif
};

/** Constructor.
 * @param p Parameters for RendererBase and MimoProcessor
 **/
template<typename Derived>
RendererBase<Derived>::RendererBase(const apf::parameter_map& p)
  : _base(_add_params(p))
  , state(_fifo, p)
  , master_volume_correction(apf::math::dB2linear(
        this->params.get("master_volume_correction", 0.0)))
#ifdef ENABLE_DYNAMIC_ASDF
  , dynamic_sources(_fifo)
  , freewheeling(this->params.get("freewheeling", false))
#endif
  , _master_level()
  , _source_list(_fifo)
  , _show_head(true)
#ifdef ENABLE_DYNAMIC_ASDF
  , _scene(_fifo)
#endif
{}

/** Create a new source.
 * @return ID of new source
 * @throw unknown whatever the Derived::Source constructor throws
 **/
template<typename Derived>
std::string
RendererBase<Derived>::add_source(id_t requested_id
    , const apf::parameter_map& p, const float* file_source_ptr)
{
  std::string id = requested_id;
  if (id.size() && _source_map.find(id) != _source_map.end())
  {
    throw std::runtime_error("Source with ID \"" + id + "\" already exists");
  }
  if (id == "")
  {
    do
    {
      id = ".ssr:" + apf::str::A2S(_next_id_suffix++);
    }
    while (_source_map.find(id) != _source_map.end());
  }

  typename Derived::Source::Params src_params;
  src_params = p;
  src_params.parent = &this->derived();
  src_params.fifo = &_fifo;
  src_params.id = id;

  if (file_source_ptr == nullptr)
  {
    typename Derived::Input::Params in_params;
    in_params = p;
    auto in = this->add(in_params);
    // WARNING: if Derived::Input throws an exception, the SSR crashes!
    src_params.input = in;
  }
  else
  {
#ifdef ENABLE_DYNAMIC_ASDF
    src_params.file_source_ptr = file_source_ptr;
#else
    assert(false);
#endif
  }

  typename Derived::Source* src;
  try
  {
    src = _source_list.add(new typename Derived::Source(src_params));
  }
  catch (...)
  {
    if (src_params.input)
    {
      auto* input = const_cast<typename Derived::Input*>(src_params.input);
      this->rem(input);
    }
    throw;
  }

  // This cannot be done in the Derived::Source constructor because then the
  // connections to the Outputs are active before the Source is properly added
  // to the source list:
  src->connect();

  _source_map[id] = src;
  return id;
}

template<typename Derived>
void RendererBase<Derived>::rem_source(id_t id)
{
  auto delinquent = _source_map.find(id);

  if (delinquent == _source_map.end())
  {
    // TODO: warning?
    return;
  }

  auto* source = delinquent->second;

  _source_map.erase(delinquent);

  assert(source);
  source->derived().disconnect();

  auto* input = const_cast<typename Derived::Input*>(source->_input);
  _source_list.rem(source);

  // TODO: really remove the corresponding Input?
  // ATTENTION: there may be several sources using the input! (or not?)

  if (input != nullptr)
  {
    this->rem(input);
  }
}

template<typename Derived>
void RendererBase<Derived>::rem_all_sources()
{
  while (!_source_map.empty())
  {
    this->rem_source(_source_map.begin()->first);
  }
}


template<typename Derived>
typename RendererBase<Derived>::Source*
RendererBase<Derived>::get_source(id_t id)
{
  auto iter = _source_map.find(id);
  if (iter == _source_map.end())
  {
    return nullptr;
  }
  else
  {
    return iter->second;
  }
}


#ifdef ENABLE_DYNAMIC_ASDF
// NB: The APF_PROCESS macro doesn't work here because of the use of CRTP.
template<typename Derived>
struct RendererBase<Derived>::Process : _base::Process
{
  Process(Derived& parent)
    : _base::Process(parent)
  {
    const auto& scene = parent._scene.get();
    if (!scene) return;

    assert(parent.dynamic_sources != nullptr);
    auto& source_list = *parent.dynamic_sources.get();
    assert(source_list.size() == scene->file_sources() + scene->live_sources());

    auto [rolling, transport_frame] = parent.get_transport_state();

    while (true)
    {
      auto result = scene->update_audio_data(rolling);
      if (result == ASDF_STREAMING_SUCCESS)
      {
        break;
      }
      else if (result == ASDF_STREAMING_EMPTY_BUFFER)
      {
        if (parent.freewheeling)
        {
          // Do nothing, just try again later ...
        }
        else
        {
          SSR_ERROR("ASDF streaming: empty buffer");
          throw std::runtime_error("exiting callback");
        }
      }
      else if (result == ASDF_STREAMING_INCOMPLETE_SEEK)
      {
        SSR_ERROR("Bug: incomplete seek");
        throw std::runtime_error("exiting callback");
      }
      else if (result == ASDF_STREAMING_SEEK_WHILE_ROLLING)
      {
        SSR_ERROR("Bug: seek while rolling");
        throw std::runtime_error("exiting callback");
      }
      else
      {
        assert(false);
      }
      std::this_thread::sleep_for(std::chrono::microseconds(parent.usleeptime));
    }

    {
      auto t = scene->get_reference_transform(transport_frame);
      auto rotation = t.rot;
      if (rotation != parent.dynamic_reference.rot) {
        parent.dynamic_reference.rot = rotation;
        parent.state.reference_rotation.set_from_rt_thread(rotation);
      }
      auto position = t.pos;
      if (position != parent.dynamic_reference.pos) {
        parent.dynamic_reference.pos = position;
        parent.state.reference_position.set_from_rt_thread(position);
      }
      auto volume = t.vol;
      if (volume != parent.dynamic_reference.vol) {
        parent.dynamic_reference.vol = volume;
        parent.state.master_volume.set_from_rt_thread(std::move(volume));
      }
    }

    // NB: Functions for checking dynamic sources are not thread safe,
    //     therefore we call them in a loop from a single thread.

    for (auto& source: apf::cast_proxy<typename Derived::Source, rtlist_t>(
          parent._source_list))
    {
      size_t source_number = source.dynamic_number;
      if (source_number == static_cast<size_t>(-1))
      {
        continue;  // This source is not part of the dynamic ASDF scene
      }
      auto transform = scene->get_source_transform(
          source_number, transport_frame);
      auto& target_transform = source_list[source_number];
      if (transform)
      {
        if (target_transform == std::nullopt)
        {
          source.active.set_from_rt_thread(true);
          target_transform = ssr::Transform{};
        }

        auto rotation = transform->rot;
        if (rotation != target_transform->rot)
        {
          target_transform->rot = rotation;
          source.rotation.set_from_rt_thread(rotation);
        }

        auto position = transform->pos;
        if (position != target_transform->pos)
        {
          target_transform->pos = position;
          source.position.set_from_rt_thread(position);
        }

        auto volume = transform->vol;
        if (volume != target_transform->vol)
        {
          target_transform->vol = volume;
          source.gain.set_from_rt_thread(std::move(volume));
        }
      }
      else
      {
        if (target_transform)
        {
          source.active.set_from_rt_thread(false);
        }
        target_transform = std::nullopt;
      }
    }
  }
};


/// This has to be called while the controller lock is held.
/// All existing sources must be removed before calling this.
template<typename Derived>
std::vector<DynamicSourceInfo>
RendererBase<Derived>::load_dynamic_scene(const std::string& scene_file_name
    , const std::string& input_port_prefix)
{
  assert(_source_map.empty());

  // NB: This is important because the audio thread checks _scene first
  _scene = nullptr;

  // Wait for sources to be deleted, to avoid source ID clashes with new ones.
  // Also, make sure the audio thread runs at least once with an empty scene.
  this->wait_for_rt_thread();

  // TODO: make configuration option!
  float buffer_time = 2.0;  // seconds, will be rounded up to JACK block size increments
  float sleep_time = 0.1;  // seconds
  uint32_t buffer_blocks = std::ceil(buffer_time * this->sample_rate() /
    this->block_size());
  this->usleeptime = sleep_time * 1000.0f * 1000.0f;
  auto scene = std::make_unique<DynamicScene>(
      scene_file_name, this->sample_rate(), this->block_size(),
      buffer_blocks, this->usleeptime);

  auto file_sources = scene->file_sources();
  auto live_sources = scene->live_sources();
  auto total_sources = file_sources + live_sources;

  // TODO: different way to store list of dynamic sources?

  // TODO: extend with "state" information?
  this->dynamic_sources = std::make_unique<dynamic_source_list_t>(
      total_sources);

  // TODO: reset "state buffer"?

  std::vector<DynamicSourceInfo> sources;
  sources.reserve(total_sources);

  for (size_t i = 0; i < total_sources; i++)
  {
    apf::parameter_map p;
    auto source = scene->get_sourceinfo(i);

    // TODO:
    //p.set("properties-file", ???);

    const float* file_source_ptr = nullptr;
    if (i < file_sources)
    {
      file_source_ptr = scene->file_source_ptr(i);
    }
    else
    {
      assert(file_source_ptr == nullptr);  // A JACK port will be created
      p.set("connect-to", input_port_prefix + source.port);
    }

    p.set("dynamic-number", i);

    try
    {
      auto id = this->add_source(source.id, p, file_source_ptr);
      assert(source.id == "" || id == source.id);  // IDs must be unique
      assert(id != "");
      source.id = id;
    }
    catch (std::exception& e)
    {
      this->rem_all_sources();
      throw;
    }
    sources.push_back(std::move(source));
  }
  SSR_VERBOSE("Loaded scene with "
      << file_sources << " file source(s) and "
      << live_sources << " live source(s).");
  _scene = std::move(scene);
  return sources;
}
#endif


/// A sound source.
template<typename Derived>
class RendererBase<Derived>::Source
                  : public _base::template ProcessItem<typename Derived::Source>
                  , public apf::has_begin_and_end<typename Input::iterator>
{
  private:
    using SourceBase
      = typename _base::template ProcessItem<typename Derived::Source>;

  public:
    using sample_type
      = typename std::iterator_traits<typename Input::iterator>::value_type;
#ifdef ENABLE_DYNAMIC_ASDF
    // dynamic scenes only support float
    static_assert(std::is_same_v<sample_type, float>);
#endif

    friend class RendererBase<Derived>;  // rem_source() needs access to _input

    struct Params : apf::parameter_map
    {
      Derived* parent = nullptr;
      const typename Derived::Input* input = nullptr;
#ifdef ENABLE_DYNAMIC_ASDF
      const float* file_source_ptr = nullptr;
#endif
      apf::CommandQueue* fifo = nullptr;
      std::string id;

      using apf::parameter_map::operator=;
    };

    explicit Source(const Params& p)
      : parent(*(p.parent ? p.parent : throw std::logic_error(
              "Bug (RendererBase::Source): parent == NULL!")))
      , active(*p.fifo, false)
      , position(*(p.fifo ? p.fifo : throw std::logic_error(
              "Bug (RendererBase::Source): fifo == NULL!")))
      , rotation(*p.fifo)
      , gain(*p.fifo, sample_type(1.0))
      , mute(*p.fifo, false)
      , model(*p.fifo, "point")
      , weighting_factor()
      , id(p.id)
#ifdef ENABLE_DYNAMIC_ASDF
      , dynamic_number(p.template get<size_t>("dynamic-number", -1))
#endif
      , _input(p.input)
      , _pre_fader_level()
      , _level()
    {
#ifdef ENABLE_DYNAMIC_ASDF
      if (p.input == nullptr)
      {
        if (p.file_source_ptr == nullptr)
        {
          throw std::logic_error("Bug (RendererBase::Source): input == NULL "
              "&& file_source_ptr == NULL!");
        }
        static_assert(std::is_same_v<typename Input::iterator, const float*>);
        this->_begin = p.file_source_ptr;
        this->_end = p.file_source_ptr + this->parent.block_size();
      }
#else
      if (p.input == nullptr)
      {
        throw std::logic_error("Bug (RendererBase::Source): input == NULL!");
      }
#endif
    }

    APF_PROCESS(Source, SourceBase)
    {
      this->_process();
    }

    sample_type get_level() const { return _level; }

    // In the default case, the output levels are ignored
    bool get_output_levels(sample_type*, sample_type*) const { return false; }

    std::string port_name() const
    {
      if (_input == nullptr)
      {
        return "";
      }
      else
      {
        return _input->port_name();
      }
    }

    void connect() {}
    void disconnect() {}

    Derived& parent;

    apf::SharedData<bool> active;
    apf::SharedData<Pos> position;
    apf::SharedData<Rot> rotation;
    apf::SharedData<sample_type> gain;
    apf::SharedData<bool> mute;
    apf::SharedData<std::string> model;

    apf::BlockParameter<sample_type> weighting_factor;

    const std::string id;
#ifdef ENABLE_DYNAMIC_ASDF
    const size_t dynamic_number;
#endif

  protected:
    const typename Derived::Input* _input;

  private:
    void _process();

    void _level_helper(apf::enable_queries&)
    {
      _pre_fader_level
        = apf::math::max_amplitude(this->begin(), this->end());
      _level = _pre_fader_level * this->weighting_factor;
    }

    void _level_helper(apf::disable_queries&) {}

    sample_type _pre_fader_level;
    sample_type _level;
};

template<typename Derived>
void RendererBase<Derived>::Source::_process()
{
#ifdef ENABLE_DYNAMIC_ASDF
  if (_input == nullptr)
  {
    // NB: _begin and _end were initialized in the constructor.
  }
  else
  {
#endif
    assert(_input != nullptr);
    this->_begin = _input->begin();
    this->_end = _input->end();
#ifdef ENABLE_DYNAMIC_ASDF
  }
#endif

  if (!this->parent.state.processing || this->mute || !this->active)
  {
    this->weighting_factor = 0.0;
  }
  else
  {
    this->weighting_factor = this->gain;
    // If the renderer does something nonlinear, the master volume should
    // be applied to the output signal ... TODO: shall we care?
    this->weighting_factor *= this->parent.state.master_volume;
    this->weighting_factor *= this->parent.master_volume_correction;

    // apply distance attenuation
    if (std::strcmp(this->parent.name(), "BrsRenderer") != 0
         && std::strcmp(this->parent.name(), "GenericRenderer") != 0)
    {
      if (this->model != "plane")
      {
        float source_distance = length(vec3{this->position}
          - (vec3{this->parent.state.reference_position}
            + vec3{this->parent.state.reference_position_offset}));

        // no volume increase for sources closer than 0.5 m
        source_distance = std::max(source_distance, 0.5f);

       // standard 1/r: weight *= 1.0f / source_distance;
       this->weighting_factor *= 1.0f
         / pow(source_distance, this->parent.state.decay_exponent); // 1/r^e

       // plane wave always have the same amplitude independent of the amplitude
       // reference distance and the decay exponent; normalize all other sources
       // accordingly
       this->weighting_factor *=
         pow(this->parent.state.amplitude_reference_distance,
           this->parent.state.decay_exponent);
      } // if model::plane
    } // if != BRS or Generic
  } // if muted or not

  _level_helper(this->parent);

  assert(this->weighting_factor.exactly_one_assignment());
}

template<typename Derived>
class RendererBase<Derived>::Output : public _base::Output
{
  public:
    Output(const typename _base::Output::Params& p)
      : _base::Output(p)
      , _level()
    {}

    struct Process : _base::Output::Process
    {
      explicit Process(Output& o) : _base::Output::Process(o) , _out(o) {}

      ~Process()
      {
        _out._level_helper(_out.parent);
      }

      private:
        Output& _out;
    };

    sample_type get_level() const { return _level; }

  protected:
    void _level_helper(apf::enable_queries&)
    {
      _level = apf::math::max_amplitude(this->buffer.begin()
          , this->buffer.end());
    }

    void _level_helper(apf::disable_queries&) {}

  private:
    sample_type _level;
};

// This is a kind of C++ mixin class, but it also includes the CRTP
template<typename Derived, template<typename> class Base>
struct SourceToOutput : Base<Derived>
{
  using typename Base<Derived>::Input;

  struct Source : Base<Derived>::Source
  {
    using typename Base<Derived>::Source::Params;
    using sourcechannels_t = apf::fixed_vector<typename Derived::SourceChannel>;

    template<typename... Args>
    Source(const Params& p, Args&&... args)
      : Base<Derived>::Source(p)
      , sourcechannels(std::forward<Args>(args)...)
    {}

    void connect()
    {
      auto temp = std::list<typename Derived::SourceChannel*>();
      apf::append_pointers(this->sourcechannels, temp);
      this->parent.add_to_sublist(temp, apf::make_cast_proxy<Output>(
            const_cast<rtlist_t&>(this->parent.get_output_list()))
          , &Output::sourcechannels);
    }

    void disconnect()
    {
      auto temp = std::list<typename Derived::SourceChannel*>();
      apf::append_pointers(this->sourcechannels, temp);
      this->parent.rem_from_sublist(temp, apf::make_cast_proxy<Output>(
            const_cast<rtlist_t&>(this->parent.get_output_list()))
          , &Output::sourcechannels);
    }

    sourcechannels_t sourcechannels;

    private:
      using rtlist_t = typename Derived::rtlist_t;
  };

  struct Output : Base<Derived>::Output
  {
    using typename Base<Derived>::Output::Params;
    using sourcechannels_t = std::list<typename Derived::SourceChannel*>;

    Output(const Params& p) : Base<Derived>::Output(p) {}

    sourcechannels_t sourcechannels;
  };

  explicit SourceToOutput(const apf::parameter_map& params)
    : Base<Derived>(params)
  {}
};

}  // namespace ssr

#endif
