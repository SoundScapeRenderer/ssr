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
/// %Controller.

#ifndef SSR_CONTROLLER_H
#define SSR_CONTROLLER_H

#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*, HAVE_*, WITH_*
#endif

// TODO: move these includes to a more suitable location?
#include "apf/jack_policy.h"
#include "apf/cxx_thread_policy.h"

#define SSR_QUERY_POLICY apf::enable_queries

#include <libxml/xmlsave.h> // temporary hack!

#include "ssr_global.h"
#include "publisher.h"

#ifdef ENABLE_ECASOUND
#include "audioplayer.h"
#include "audiorecorder.h"
#endif

#include "xmlparser.h"
#include "configuration.h"

#ifdef ENABLE_GUI
#include "qgui.h"
#endif

#ifdef ENABLE_IP_INTERFACE
#include "server.h"
#endif

#include "tracker.h"
#ifdef ENABLE_INTERSENSE
#include "trackerintersense.h"
#endif
#ifdef ENABLE_POLHEMUS
#include "trackerpolhemus.h"
#endif
#ifdef ENABLE_VRPN
#include "trackervrpn.h"
#endif
#ifdef ENABLE_RAZOR
#include "trackerrazor.h"
#endif

#include "scene.h"  // for Scene
#include "rendersubscriber.h"

#include "posixpathtools.h"
#include "apf/math.h"
#include "apf/stringtools.h"

using Node = XMLParser::Node; ///< a node of the DOM tree

namespace ssr
{

namespace internal
{

inline void print_about_message()
{
  const std::string about_string =
    "       ___     \n"
    "      /  ___   \n"
    "  ___/  /  ___ \n"
    "    ___/  /    " PACKAGE_STRING "\n"
    "         /     \n"
    "               \n"
    "Website: <" PACKAGE_URL ">\n"
    "Contact: <" PACKAGE_BUGREPORT ">\n"
    "\n"
    SSR_COPYRIGHT
    ;

  std::cout << about_string << std::endl;
}

}  // namespace internal

/** %Controller class.
 * Has a list of objects which receive messages on events like position change
 * etc. With this list a kind of Publisher/Subscriber pattern is realized.
 **/
template<typename Renderer>
class Controller : public Publisher
{
  public:
    using loudspeaker_id_t = Loudspeaker::container_t::size_type;

    /// ctor
    Controller(int argc, char *argv[]);
    virtual ~Controller(); ///< dtor

    bool run();

    void set_source_output_levels(id_t id, float* first, float* last);

    virtual bool load_scene(const std::string& scene_file_name);
    virtual bool save_scene_as_XML(const std::string& filename) const;

    virtual void start_processing();
    virtual void stop_processing();

    virtual void new_source(const std::string& name, Source::model_t model
        , const std::string& file_name_or_port_number, int channel = 0
        , const Position& position = Position(), const bool pos_fix = false
        , const Orientation& orientation = Orientation()
        , const bool or_fix = false
        , const float gain = 1.0f, const bool muted = false
        , const std::string& properties_file = "");

    virtual void delete_source(id_t id);
    /// delete all sources in all subscribers
    virtual void delete_all_sources();

    virtual void set_source_position(id_t id, const Position& position);
    virtual void set_source_orientation(id_t id
        , const Orientation& orientation);
    void orient_source_toward_reference(const id_t id);
    void orient_all_sources_toward_reference();
    virtual void set_source_gain(id_t id, float gain);
    virtual void set_source_signal_level(const id_t id
        , const float level);
    virtual void set_source_mute(id_t id, bool mute);
    virtual void set_source_name(id_t id, const std::string& name);
    virtual void set_source_properties_file(id_t id, const std::string& name);
    virtual void set_source_model(id_t id, Source::model_t model);
    virtual void set_source_port_name(id_t id, const std::string& port_name);
    virtual void set_source_file_name(id_t id, const std::string& file_name);
    virtual void set_source_file_channel(id_t id, const int& channel);
    virtual void set_source_position_fixed(id_t id, const bool fixed);

    virtual void set_reference_position(const Position& position);
    virtual void set_reference_orientation(const Orientation& orientation);

    virtual void set_reference_offset_position(const Position& position);
    virtual void set_reference_offset_orientation(const Orientation& orientation);

    virtual void set_master_volume(float volume);

    virtual void set_decay_exponent(const float exponent);

    virtual void set_amplitude_reference_distance(const float dist);

    virtual void set_master_signal_level(float level);

    virtual void set_cpu_load(const float load);

    virtual void publish_sample_rate(const int sample_rate);

    virtual std::string get_renderer_name() const;

    virtual bool show_head() const;

    virtual void transport_start();
    virtual void transport_stop();
    virtual bool transport_locate(float time);

    virtual void calibrate_client();

    /// update JACK transport state
    void set_transport_state(const std::pair<bool, jack_nframes_t>& state);
    /// send processing state of the renderer to all subscribers.
    virtual void set_processing_state(bool state);

    virtual void set_auto_rotation(bool auto_rotate_sources);

    virtual std::string get_scene_as_XML() const;

    virtual void subscribe(Subscriber* subscriber);
    virtual void unsubscribe(Subscriber* subscriber);

    void set_loop_mode(bool loop) { _loop = loop; } ///< temporary solution!

    // temporary solution!
    void deactivate() { _renderer.deactivate(); }

  private:
    using output_list_t
      = typename Renderer::template rtlist_proxy<typename Renderer::Output>;

    class query_state;

#ifdef ENABLE_ECASOUND
    /// load the audio recorder and set it to "record enable" mode.
    void _load_audio_recorder(const std::string& audio_file_name
        , const std::string& sample_format = "16"
        , const std::string& client_name = "recorder"
        , const std::string& input_prefix = "channel");
#endif

    void _start_tracker(const std::string& type, const std::string& ports = "");
#ifdef ENABLE_GUI
    int _start_gui(const std::string& path_to_gui_images
        , const std::string& path_to_scene_menu);
#endif

    int _argc;
    char** _argv;
    conf_struct _conf;

    Scene _scene;
    /// a list of subscribers
    using subscriber_list_t = std::vector<Subscriber*>;
    /// list of objects that will be notified on all events
    subscriber_list_t _subscribers;
#ifdef ENABLE_GUI
    std::unique_ptr<QGUI> _gui;
#endif

    Renderer _renderer;

    query_state _query_state;
#ifdef ENABLE_ECASOUND
    AudioRecorder::ptr_t    _audio_recorder; ///< pointer to audio recorder
    AudioPlayer::ptr_t      _audio_player;   ///< pointer to audio player
#endif
    std::string _schema_file_name;           ///< XML Schema
    std::string _input_port_prefix;          ///< e.g. alsa_pcm:capture
#ifdef ENABLE_IP_INTERFACE
    std::unique_ptr<Server> _network_interface;
#endif
    std::unique_ptr<Tracker> _tracker;

    /// check if audio player is running and start it if necessary
    bool _audio_player_is_running();

    /// Publishing function.
    /// The first argument is a pointer to a member function of the Subscriber
    /// class, the rest are arguments to said member function.
    template<typename R, typename... FuncArgs, typename... Args>
    inline void _publish(R (Subscriber::*f)(FuncArgs...), Args&&... args)
    {
      ScopedLock guard(_subscribers_lock);
      for (auto& subscriber: _subscribers)
      {
        (subscriber->*f)(std::forward<Args>(args)...);  // ignore return value
      }
    }

    /// helper struct for a source including its source id
    struct SourceCopy : public Source
    {
      typedef std::vector<SourceCopy> container_t; ///< list of SourceCopys
      /// type conversion constructor
      SourceCopy(const std::pair<id_t, Source>& other)
        : Source(other.second) // copy ctor of base class
        , id(other.first)
      {}

      id_t id; ///< unique ID, see Scene::source_map_t
    };

    void _add_master_volume(Node& node) const;
    void _add_transport_state(Node& node) const;
    void _add_reference(Node& node) const;
    void _add_position(Node& node
        , const Position& position, const bool fixed = false) const;
    void _add_orientation(Node& node, const Orientation& orientation) const;
    void _add_loudspeakers(Node& node) const;
    void _add_sources(Node& node, const std::string& filename = "") const;
    void _add_audio_file_name(Node& node, const std::string& name
        , int channel) const;
    bool _create_spontaneous_scene(const std::string& audio_file_name);

    bool _loop; ///< part of a quick-hack. should be removed some time.  

    std::unique_ptr<typename Renderer::template ScopedThread<
      typename Renderer::QueryThread>> _query_thread;

    typename Renderer::Lock _subscribers_lock;
    using ScopedLock = typename Renderer::ScopedLock;
};

template<typename Renderer>
Controller<Renderer>::Controller(int argc, char* argv[])
  : _argc(argc)
  , _argv(argv)
  , _conf(configuration(_argc, _argv))
  , _renderer(_conf.renderer_params)
  , _query_state(query_state(*this, _renderer))
  , _tracker(nullptr)
  , _loop(false)
{
  // TODO: signal handling?

  internal::print_about_message();

  _renderer.load_reproduction_setup();

#ifndef ENABLE_IP_INTERFACE
  if (_conf.ip_server)
  {
    throw std::logic_error(_conf.exec_name
        + " was compiled without IP-server support!\n"
        "Use the --no-ip-server option to disable the IP-server.\n"
        "Type '" + _conf.exec_name + " --help' for more information.");
  }
#endif

#ifndef ENABLE_GUI
  if (_conf.gui)
  {
    throw std::logic_error(_conf.exec_name
        + " was compiled without GUI support!\n"
        "Use the --no-gui option to disable GUI.\n"
        "Type '" + _conf.exec_name + " --help' for more information.");
  }
#endif

  if (_conf.ip_server && _conf.freewheeling)
  {
    WARNING("Freewheel mode cannot be used together with IP-server. Ignored.\n"
        "Type '" + _conf.exec_name + " --help' for more information.");
    _conf.freewheeling = false;
  }

  if (_conf.freewheeling && _conf.gui)
  {
    WARNING("In 'freewheeling' mode the GUI cannot be used! Disabled.\n"
        "Type '" + _conf.exec_name + " --help' for more information.");
    _conf.gui = false;
  }

  // temporary solution:
  this->set_loop_mode(_conf.loop);

  this->subscribe(&_scene);

  this->publish_sample_rate(_renderer.sample_rate());

  std::vector<Loudspeaker> loudspeakers;
  _renderer.get_loudspeakers(loudspeakers);
  _publish(&Subscriber::set_loudspeakers, loudspeakers);

  // TODO: memory leak, subscriber is never deleted!
  auto subscriber = new RenderSubscriber<Renderer>(_renderer);
  this->subscribe(subscriber);

#ifdef ENABLE_ECASOUND
  _load_audio_recorder(_conf.audio_recorder_file_name);
#endif

  // TODO: allow specification in scene file
  this->set_auto_rotation(_conf.auto_rotate_sources);

  if (!this->load_scene(_conf.scene_file_name))
  {
    throw std::runtime_error("Couldn't load scene!");
  }

  if (_conf.freewheeling)
  {
    if (!_renderer.set_freewheel(1))
    {
      throw std::runtime_error("Unable to switch to freewheeling mode!");
    }
  }

#ifdef ENABLE_IP_INTERFACE
  if (_conf.ip_server)
  {

    VERBOSE("Starting IP Server with port " << _conf.server_port
        << " and with end-of-message character with ASCII code " << 
        _conf.end_of_message_character << ".");

    _network_interface.reset(new Server(*this, _conf.server_port
        , static_cast<char>(_conf.end_of_message_character)));
    _network_interface->start();
  }
#endif // ENABLE_IP_INTERFACE
}

template<typename Renderer>
class Controller<Renderer>::query_state
{
  public:
    query_state(Controller& controller, Renderer& renderer)
      : _controller(controller)
      , _renderer(renderer)
    {}

    void query()
    {
      _state = _renderer.get_transport_state();
      _cpu_load = _renderer.get_cpu_load();

      auto output_list = output_list_t(_renderer.get_output_list());

      _master_level = typename Renderer::sample_type();
      for (const auto& out: output_list)
      {
        _master_level = std::max(_master_level, out.get_level());
      }

      using source_list_t
        = typename Renderer::template rtlist_proxy<typename Renderer::Source>;
      auto source_list = source_list_t(_renderer.get_source_list());

      if (_source_levels.size() == source_list.size())
      {
        auto levels = _source_levels.begin();

        for (const auto& source: source_list)
        {
          levels->source_id = source.id;
          levels->source_level = source.get_level();

          levels->outputs_available = source.get_output_levels(
              &*levels->outputs.begin(), &*levels->outputs.end());

          ++levels;
        }
        _discard_source_levels = false;
      }
      else
      {
        _discard_source_levels = true;
        _new_size = source_list.size();
      }
    }

    void update()
    {
      _controller._publish(&Subscriber::set_transport_state, _state);
      _controller.set_cpu_load(_cpu_load);
      _controller.set_master_signal_level(_master_level);

      if (!_discard_source_levels)
      {
        for (auto& item: _source_levels)
        {
          _controller.set_source_signal_level(item.source_id
              , item.source_level);

          // TODO: make this a compile-time decision:
          if (item.outputs_available)
          {
            _controller.set_source_output_levels(item.source_id
                , &*item.outputs.begin(), &*item.outputs.end());
          }
        }
      }
      else
      {
        _source_levels.resize(_new_size
          , SourceLevel(_renderer.get_output_list().size()));
      }
    }

  private:
    struct SourceLevel
    {
      explicit SourceLevel(size_t n)
        : outputs_available(false)
        , outputs(n)
      {}

      typename Renderer::sample_type source_level;

      int source_id;

      bool outputs_available;
      // this may never be resized:
      std::vector<typename Renderer::sample_type> outputs;
    };

    using source_levels_t = std::vector<SourceLevel>;

    Controller& _controller;
    Renderer& _renderer;
    std::pair<bool, jack_nframes_t> _state;
    float _cpu_load;
    typename Renderer::sample_type _master_level;

    source_levels_t _source_levels;
    bool _discard_source_levels = true;
    size_t _new_size = 0;
};

template<typename Renderer>
bool Controller<Renderer>::run()
{
  // TODO: make sleep time customizable
  _query_thread.reset(Renderer::new_scoped_thread(
        typename Renderer::QueryThread(_renderer._query_fifo), 10 * 1000));

  _start_tracker(_conf.tracker, _conf.tracker_ports);

  if (!_renderer.activate())
  {
    return false;
  }

  // CAUTION: this must be called after activate()!
  // If not, an infinite recursion happens!

  _renderer.new_query(_query_state);

  if (_conf.gui)
  {
#ifdef ENABLE_GUI

    // TEMPORARY!!!
    this->start_processing();
    this->transport_locate(0.0f);
    //this->transport_start();

    if (!_start_gui(_conf.path_to_gui_images, _conf.path_to_scene_menu))
    {
      return false;
    }
#else
    // this condition has already been checked above!
    assert(false);
#endif // #ifdef ENABLE_GUI
  }
  else // without GUI
  {
    // TODO: check if IP-server is running
    // TODO: wait for shutdown command (run forever)

    // TEMPORARY!!!
    this->start_processing();
    this->transport_locate(0.0f);
    this->transport_start();
    bool keep_running = true;
    while (keep_running)
    {
      switch(fgetc(stdin))
      {
        case 'c':
          std::cout << "Calibrating client.\n";
          this->calibrate_client();
          break;
        case 'p':
          this->transport_start();
          break;
        case 'q':
          keep_running = false;
          break;
        case 'r':
          this->transport_locate(0.0f);
          break;
        case 's':
          this->transport_stop();
          break;
      }
    }
  }
  return true;
}

template<typename Renderer>
Controller<Renderer>::~Controller()
{
  //recorder->disable();
  this->stop_processing();

  // TODO: check if transport needs to be stopped
  this->transport_stop();

  if (!this->save_scene_as_XML("ssr_scene_autosave.asd"))
  {
    ERROR("Couldn't write XML scene! (It's an ugly hack anyway ...");
  }

  {
    ScopedLock guard(_subscribers_lock);
    _subscribers.clear();
  }  // unlock

  this->deactivate();
}

namespace internal
{

struct PositionPlusBool : Position
{
  PositionPlusBool()
                                 :                fixed(false) {}
  explicit PositionPlusBool(Position pos, const bool fixed = false)
                                 : Position(pos), fixed(fixed) {}
  PositionPlusBool(const float x, const float y, const bool fixed = false)
                                 : Position(x,y), fixed(fixed) {}

  bool fixed;
};

/// find position in child nodes.
/// Traverse through all child nodes of @a node and check for a @b position
/// element.
/// @param node parent node
/// @return std::unique_ptr to the obtained position, empty unique_ptr if no
/// position element was found.
/// @warning If you want to extract e.g. position and orientation it would be
/// more effective to do both (or even more) in one loop. But anyway, this
/// seems easier to use.
inline std::unique_ptr<PositionPlusBool>
get_position(const Node& node)
{
  std::unique_ptr<PositionPlusBool> temp; // temp = NULL
  if (!node) return temp; // return NULL

  for (Node i = node.child(); !!i; ++i)
  {
    if (i == "position")
    {
      float x, y;
      bool fixed;

      // if read operation successful
      if (apf::str::S2A(i.get_attribute("x"), x)
          && apf::str::S2A(i.get_attribute("y"), y))
      {
        // "fixed" indicated
        if (apf::str::S2A(i.get_attribute("fixed"), fixed))
        {
          temp.reset(new PositionPlusBool(x, y, fixed));
        }
        else // "fixed" not indicated
        {
          temp.reset(new PositionPlusBool(x, y));
        }

        return temp; // return sucessfully
      }
      else
      {
        ERROR("Invalid position!");
        return temp; // return NULL
      } // if read operation successful

    } // if (i == "position")
  }
  return temp; // return NULL
}

/// find orientation in child nodes.
/// @param node parent node
/// @see get_position
inline std::unique_ptr<Orientation>
get_orientation(const Node& node)
{
  std::unique_ptr<Orientation> temp; // temp = NULL
  if (!node) return temp;          // return NULL
  for (Node i = node.child(); !!i; ++i)
  {
    if (i == "orientation")
    {
      float azimuth;
      if (apf::str::S2A(i.get_attribute("azimuth"), azimuth))
      {
        temp.reset(new Orientation(azimuth));
        return temp; // return sucessfully
      }
      else
      {
        ERROR("Invalid orientation!");
        return temp; // return NULL
      }
    }
  }
  return temp;       // return NULL
}

/** get attribute of a node.
 * @param node the node you want to have the attribute of.
 * @param attribute name of attribute
 * @param default_value default return value if something goes wrong
 * @return value default_value on error.
 **/
template<typename T>
T
get_attribute_of_node(const Node& node, const std::string attribute
    , const T default_value)
{
  if (!node) return default_value;
  return apf::str::S2RV(node.get_attribute(attribute), default_value);
}

/** check for file/port
 * @param node parent node
 * @param file_name_or_port_number a string where the obtained file name or
 * port number is stored.
 * @return channel number, 0 if port was given.
 * @note on error, @p file_name_or_port_number is set to the empty string ""
 * and 0 is returned.
 **/
inline int
get_file_name_or_port_number(const Node& node
    , std::string& file_name_or_port_number)
{
  for (Node i = node.child(); !!i; ++i)
  {
    if (i == "file")
    {
      file_name_or_port_number = get_content(i);
      int channel = apf::str::S2RV(i.get_attribute("channel"), 1);
      // TODO: raise error if channel is negative?
      assert(channel >= 0);
      return channel;
    }
    else if (i == "port")
    {
      file_name_or_port_number = get_content(i);
      return 0;
    }
  }
  // nothing found:
  file_name_or_port_number = "";
  return 0;
}

} // end of anonymous namespace

template<typename Renderer>
void
Controller<Renderer>::subscribe(Subscriber* const subscriber)
{
  ScopedLock guard(_subscribers_lock);
  _subscribers.push_back(subscriber);
}

template<typename Renderer>
void
Controller<Renderer>::unsubscribe(Subscriber* subscriber)
{
  ScopedLock guard(_subscribers_lock);
  auto s = std::find(_subscribers.begin(), _subscribers.end(), subscriber);
  if (s != _subscribers.end())
  {
    _subscribers.erase(s);
  }
  else
  {
    WARNING("unsubscribe(): given subscriber not found!");
  }
}

template<typename Renderer>
bool
Controller<Renderer>::load_scene(const std::string& scene_file_name)
{
  this->stop_processing();
  // TODO: get state.

  // remove all existing sources (if any)
  this->delete_all_sources();

  if (scene_file_name == "")
  {
    VERBOSE("No scene file specified. Opening empty scene ...");
    return true;
  }

  std::string file_extension = posixpathtools::get_file_extension(scene_file_name);

  if (file_extension == "")
  {
    ERROR("File name '" << scene_file_name << "' does not have an extension.");
    return false;
  }
  else if (file_extension == "asd")
  {
    XMLParser xp; // load XML parser
    auto scene_file = xp.load_file(scene_file_name);
    if (!scene_file)
    {
      ERROR("Unable to load scene setup file '" << scene_file_name << "'!");
      return false;
    }

    if (_conf.xml_schema == "")
    {
      ERROR("No schema file specified!");
      // TODO: return true and continue anyway?
      return false;
    }
    else if (scene_file->validate(_conf.xml_schema))
    {
      VERBOSE("Valid scene setup (" << scene_file_name << ").");
    }
    else
    {
      ERROR("Error validating '" << scene_file_name << "' with schema '"
      << _conf.xml_schema << "'!");
      return false;
    }

    XMLParser::xpath_t xpath_result;

    // GET MASTER VOLUME
    float master_volume = 0.0f; // dB

    xpath_result = scene_file->eval_xpath("//scene_setup/volume");

    if (xpath_result
        && !apf::str::S2A(get_content(xpath_result->node()), master_volume))
    {
      WARNING("Invalid master volume specified in scene!");
      master_volume = 0.0f;
    }

    VERBOSE("Setting master volume to " << master_volume << " dB.");
    this->set_master_volume(apf::math::dB2linear(master_volume));

    // GET DECAY EXPONENT
    auto exponent = _conf.renderer_params.get<float>("decay_exponent");

    xpath_result = scene_file->eval_xpath("//scene_setup/decay_exponent");
    if (xpath_result)
    {
      if (!apf::str::S2A(get_content(xpath_result->node()), exponent))
      {
        WARNING("Invalid amplitude decay exponent!");
      }
    }

    // always use default value when nothing is specified
    VERBOSE("Setting amplitude decay exponent to " << exponent << ".");
    this->set_decay_exponent(exponent);

    // GET AMPLITUDE REFERENCE DISTANCE

    auto ref_dist = _conf.renderer_params.get<float>(
        "amplitude_reference_distance");  // throws on error!

    xpath_result
      = scene_file->eval_xpath("//scene_setup/amplitude_reference_distance");
    if (xpath_result)
    {
      if (!apf::str::S2A(get_content(xpath_result->node()), ref_dist))
      {
        WARNING("Invalid amplitude reference distance!");
      }
    }

    // always use default value when nothing is specified
    VERBOSE("Setting amplitude reference distance to "
        << ref_dist << " meters.");
    this->set_amplitude_reference_distance(ref_dist);

    // LOAD REFERENCE

    std::unique_ptr<internal::PositionPlusBool> pos_ptr;
    std::unique_ptr<Orientation>      dir_ptr;

    xpath_result = scene_file->eval_xpath("//scene_setup/reference");
    if (xpath_result)
    {
      // there should be only one result:
      if (xpath_result->size() != 1)
      {
        ERROR("More than one reference found in scene setup! Aborting.");
        return false;
      }
      pos_ptr = internal::get_position   (xpath_result->node());
      dir_ptr = internal::get_orientation(xpath_result->node());
    }
    else
    {
      VERBOSE("No reference point given in XML file. "
          "Using standard (= origin).");
    }
    if (!pos_ptr) pos_ptr.reset(new internal::PositionPlusBool());
    if (!dir_ptr) dir_ptr.reset(new Orientation(90));

    this->set_reference_position(*pos_ptr);
    this->set_reference_orientation(*dir_ptr);
    this->set_reference_offset_position(Position());
    this->set_reference_offset_orientation(Orientation());
   
    // LOAD SOURCES

    xpath_result = scene_file->eval_xpath("//scene_setup/source");
    if (xpath_result)
    {
      for (Node node; (node = xpath_result->node()); ++(*xpath_result))
      {
        std::string name  = node.get_attribute("name");
        std::string id    = node.get_attribute("id");
        std::string properties_file = node.get_attribute("properties_file");

        properties_file = posixpathtools::make_path_relative_to_current_dir(
            properties_file, scene_file_name);

        pos_ptr.reset(); dir_ptr.reset();
        pos_ptr = internal::get_position   (node);
        dir_ptr = internal::get_orientation(node);

        // just for the error message:
        std::string id_str;   if (id != "") id_str = " id: \"" + id + "\"";
        std::string name_str; if (name != "") name_str = " name: \""
          + name + "\"";

        Source::model_t model
          = internal::get_attribute_of_node(node, "model", Source::unknown);

        if (model == Source::unknown)
        {
          VERBOSE("Source model not defined!" << id_str << name_str
              << " Using default (= point source).");
          model = Source::point;
        }

        if ((model == Source::point) && !dir_ptr)
        {
          // orientation is optional for point sources, required for plane waves
          dir_ptr.reset(new Orientation);
        }

        if (!pos_ptr || (!dir_ptr && !_scene.get_auto_rotation()))
        {
          ERROR("Both position and orientation have to be specified for source"
              << id_str << name_str << "! Not loaded");
          continue; // next source
        }

        std::string file_name_or_port_number;
        int channel = internal::get_file_name_or_port_number(node
              , file_name_or_port_number);

        if (channel != 0)  // --> soundfile
        {
          file_name_or_port_number
            = posixpathtools::make_path_relative_to_current_dir(
              file_name_or_port_number, scene_file_name);
        }

        float gain_dB = internal::get_attribute_of_node(node, "volume", 0.0f);
        bool muted = internal::get_attribute_of_node(node, "mute", false);
        pos_ptr->fixed = internal::get_attribute_of_node(node, "fixed", false);
        // bool doppler = internal::get_attribute_of_node(node, "doppler_effect", false);

        this->new_source(name, model, file_name_or_port_number, channel
            , *pos_ptr, pos_ptr->fixed, *dir_ptr, false
            , apf::math::dB2linear(gain_dB), muted, properties_file);
      }
    }
    else
    {
      WARNING("No sources found in \"" << scene_file_name << "\"!");
    }
  }
  else // file_extension != "asd" -> try to open file as audio file
  {
    WARNING("Trying to open specified file as audio file.");
    if (!_create_spontaneous_scene(scene_file_name))
    {
      ERROR("\"" << scene_file_name << "\" could not be loaded as audio file!");
      return false;
    }
  }

  this->transport_locate(0); // go to beginning of audio files
  // TODO: only start processing if it was on before
  this->start_processing();

  return true;
}

template<typename Renderer>
bool
Controller<Renderer>::_create_spontaneous_scene(const std::string& audio_file_name)
{
#ifndef ENABLE_ECASOUND
  ERROR("Couldn't create scene from file \"" << audio_file_name
        << "\"! Ecasound was disabled at compile time.");
  return false;
#else
  size_t no_of_audio_channels;
  AudioPlayer::Soundfile::get_format(audio_file_name, no_of_audio_channels);

  if (no_of_audio_channels == 0)
  {
    WARNING("No audio channels found in file \"" << audio_file_name << "\"!");
    return false;
  }

  WARNING("Creating spontaneous scene from the audio file \""
      << audio_file_name << "\".");

  if (_renderer.params.get("name", "") == "brs")
  {
    WARNING("I don't have information on the BRIRs. I'll use default HRIRs. "
            "Everything will sound in front.");
  }

  // extract pure file name
  const std::string source_name
    = audio_file_name.substr(audio_file_name.rfind('/') + 1);

  // set master volume
  this->set_master_volume(apf::math::dB2linear(0.0f));
  this->set_decay_exponent(_conf.renderer_params.get<float>("decay_exponent"));
  this->set_amplitude_reference_distance(_conf.renderer_params.get<float>(
        "amplitude_reference_distance"));  // throws on error!
  // set reference
  this->set_reference_position(Position());
  this->set_reference_orientation(Orientation(90.0f));
  this->set_reference_offset_position(Position());
  this->set_reference_offset_orientation(Orientation());

  const float default_source_distance = 2.5f; // for mono and stereo files

  switch (no_of_audio_channels)
  {
    case 1: // mono file
      this->new_source(source_name, Source::point, audio_file_name, 1
          , Position(0.0f, default_source_distance), false, Orientation()
          , false, apf::math::dB2linear(0.0f), false, "");

      VERBOSE("Creating point source at x = "
          << apf::str::A2S(0.0f) << " mtrs, y = "
          << apf::str::A2S(default_source_distance) << " mtrs.");

      break;

    case 2: // stereo file
      {
#undef PI
        const float PI = 3.14159265358979323846;

        const float pos_x = default_source_distance * cos(PI/3.0f);
        const float pos_y = default_source_distance * sin(PI/3.0f);

        // create source
        this->new_source(source_name + " left", Source::plane, audio_file_name
            , 1, Position(-pos_x, pos_y), false, Orientation(-60), false
            , apf::math::dB2linear(0.0f), false, "");

        VERBOSE("Creating point source at x = " << apf::str::A2S(-pos_x)
            << " mtrs, y = " << apf::str::A2S(pos_y) << " mtrs.");

        // create source
        this->new_source(source_name + " right", Source::plane, audio_file_name
            , 2u, Position(pos_x, pos_y), false, Orientation(-120), false
            , apf::math::dB2linear(0.0f), false, "");

        VERBOSE("Creating point source at x = "
            << apf::str::A2S(pos_x) << " mtrs, y = "
            << apf::str::A2S(pos_y) << " mtrs.");

        break;
      }

    default:
      // init random position generator
      srand((unsigned int)time(0));

      int N = 7;

      // create one source for each audio channel
      for (size_t n = 0; n < no_of_audio_channels; n++)
      {
        // random positions between -N mrts and N mtrs
        const float pos_x = (static_cast<float>(rand()%(10*(N+1)))
            - (5.0f*(N+1))) / 10.0f;

        const float pos_y = 2.0f + (static_cast<float>(rand()%(10*(N+1)))
            - (5.0f*(N+1))) / 10.0f;

        VERBOSE("Creating point source at x = "
            << apf::str::A2S(pos_x) << " mtrs, y = "
            << apf::str::A2S(pos_y) << " mtrs.");

        // create sources
        this->new_source(source_name + " " + apf::str::A2S(n+1), Source::point
            , audio_file_name, n+1u, Position(pos_x, pos_y), false
            , Orientation(), false, apf::math::dB2linear(0.0f), false, "");
      } // for each audio channel
  } // switch
  return true;
#endif  // ENABLE_ECASOUND
}

#ifdef ENABLE_GUI
template<typename Renderer>
int
Controller<Renderer>::_start_gui(const std::string& path_to_gui_images
    , const std::string& path_to_scene_menu)
{
  // Check whether the system supports OpenGL and set default OpenGL format
  // which will be applied to QGLWidget.
  // Should be equivalent to glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
  QGLFormat gl_format = QGLFormat::defaultFormat();
  gl_format.setSampleBuffers(true);
  gl_format.setRgba(true);
  gl_format.setDoubleBuffer(true);
  gl_format.setDepth(true);
  QGLFormat::setDefaultFormat(gl_format);

  _gui.reset(new QGUI(*this, _scene, _argc, _argv,
    path_to_gui_images, path_to_scene_menu));

  // check if anti-aliasing is possible
  if (!_gui->format().sampleBuffers())
  {
    WARNING("This system does not provide sample buffer support.\n"
        "I can not enable anti-aliasing for OpenGl stuff.");
  }

  return _gui->run();
}
#endif // ENABLE_GUI

template<typename Renderer>
void
Controller<Renderer>::_start_tracker(const std::string& type, const std::string& ports)
{
  if (type == "")
  {
    return;
  }
  else if (type == "intersense")
  {
#if defined(ENABLE_INTERSENSE)
    _tracker = TrackerInterSense::create(*this, ports);

#else
    ERROR("The SSR was compiled without InterSense tracker support!");
    (void)ports;  // avoid "unused parameter" warning
    return;
#endif
  }
  // "polhemus" is allowed for backwards compatibility:
  else if (type == "fastrak" || type == "patriot" || type == "polhemus")
  {
#if defined(ENABLE_POLHEMUS)
    _tracker = TrackerPolhemus::create(*this, type, ports);
#else
    ERROR("The SSR was compiled without Polhemus tracker support!");
    (void)ports;  // avoid "unused parameter" warning
    return;
#endif
  }
  else if (type == "vrpn")
   {
 #if defined(ENABLE_VRPN)
     _tracker = TrackerVrpn::create(*this, ports);
 #else
     ERROR("The SSR was compiled without VRPN tracker support!");
     (void)ports;  // avoid "unused parameter" warning
     return;
 #endif
   }
  else if (type == "razor")
  {
#if defined(ENABLE_RAZOR)
    _tracker = TrackerRazor::create(*this, ports);
#else
    ERROR("The SSR was compiled without Razor AHRS tracker support!");
    (void)ports;  // avoid "unused parameter" warning
    return;
#endif
  }
  else
  {
    ERROR("Unknown tracker type \"" << type << "\"!");
    return;
  }

  if (!_tracker)
  {
    WARNING("Cannot find tracker. "
            "Make sure that you have the appropriate access rights "
            "to read from the port. I continue without tracker.");
  }
}

/// This is temporary!!!!
template<typename Renderer>
void
Controller<Renderer>::calibrate_client()
{
#if defined(ENABLE_INTERSENSE) || defined(ENABLE_POLHEMUS) || defined(ENABLE_VRPN) || defined(ENABLE_RAZOR)
  if (_tracker)
  {
    _tracker->calibrate();
  }
  else
  {
    WARNING("No tracker there to calibrate.");
  }
#endif
}

#ifdef ENABLE_ECASOUND
/**
 * The recorder is started as soon the JACK transport is running.
 * @param audio_file_name quite obviously, the file which will be recorded
 * to. It will have the given @a sample_format, the same number of channels as
 * the renderer has output channels and it will have the same sample rate as the
 * JACK audio server.
 * @param sample_format for more information see the ecasound(1) manpage and
 * especially the documentation of the -f option.
 * @warning This may only be used before activate() is called!
 **/
template<typename Renderer>
void
Controller<Renderer>::_load_audio_recorder(const std::string& audio_file_name
    , const std::string& sample_format, const std::string& client_name
    , const std::string& input_prefix)
{
  if (audio_file_name == "") return;

  output_list_t output_list = _renderer.get_output_list();

  size_t channels = output_list.size();
  int sample_rate = _renderer.sample_rate();

  _audio_recorder.reset(new AudioRecorder(audio_file_name
      , sample_format + "," + apf::str::A2S(channels) + ","
      + apf::str::A2S(sample_rate), "", client_name, input_prefix));

  size_t channel = 1;
  for (const auto& out: output_list)
  {
    _renderer.connect_ports(out.port_name()
        , client_name + ":" + input_prefix + "_" + apf::str::A2S(channel++));
  }
}
#endif

/** start audio processing.
 * This sets the Scene's processing state to "processing". The
 * process callback function has to check for this variable and act accordingly.
 **/
template<typename Renderer>
void
Controller<Renderer>::start_processing()
{
  if (!_scene.get_processing_state())
  {
    this->set_processing_state(true);
  }
  else
  {
    WARNING("Renderer is already processing.");
  }
}

/** Stop audio processing.
 * This sets the Scene's processing state to "ready". The
 * process callback function has to check for this variable and act
 * accordingly.
 **/
template<typename Renderer>
void
Controller<Renderer>::stop_processing()
{
  if (_scene.get_processing_state())
  {
    this->set_processing_state(false);
  }
  else
  {
    WARNING("Renderer was already stopped.");
  }
}

/** _.
 * @param state processing state.
 * @warning States are not checked for validity. To start and stop rendering,
 * use preferably start_processing() and stop_processing(). This function should
 * only be used by the respective renderer to set the state "ready" and by
 * anyone who wants to set the state "exiting".
 **/
template<typename Renderer>
void
Controller<Renderer>::set_processing_state(bool state)
{
  _publish(&Subscriber::set_processing_state, state);
}

template<typename Renderer>
void
Controller<Renderer>::set_auto_rotation(bool auto_rotate_sources)
{
  _publish(&Subscriber::set_auto_rotation, auto_rotate_sources);

  if (auto_rotate_sources) 
  {
    orient_all_sources_toward_reference();

    VERBOSE("Auto-rotation of sound sources is enabled.");
  }
  else VERBOSE("Auto-rotation of sound sources is disabled.");

}

// non-const because audioplayer could be started
template<typename Renderer>
void
Controller<Renderer>::transport_start()
{
  _renderer.transport_start();
}

// non-const because audioplayer could be started
template<typename Renderer>
void
Controller<Renderer>::transport_stop()
{
  _renderer.transport_stop();
}

/** Skips the scene to a specified instant of time
 * @ param frame instant of time in sec to locate 
 **/
template<typename Renderer>
bool
Controller<Renderer>::transport_locate(float time)
{
  // convert time to samples (cut decimal part)
  return _renderer.transport_locate(
      static_cast<jack_nframes_t>(time * _renderer.sample_rate()));
}

/** Create a new source.
 * @param name Source name
 * @param model Source model
 * @param file_name_or_port_number File name or port number (as string)
 * @param channel Channel of soundfile. If 0, a JACK portname is expected.
 * @param position initial position of the source.
 * @param orientation initial orientation of the source.
 * @param gain gain (=volume) of the source.
 * @return ID of the created source. If 0, no source was created.
 **/
template<typename Renderer>
void
Controller<Renderer>::new_source(const std::string& name
    , const Source::model_t model
    , const std::string& file_name_or_port_number, int channel
    , const Position& position, const bool pos_fixed
    , const Orientation& orientation, const bool or_fixed, const float gain
    , const bool muted, const std::string& properties_file)
{
  (void) or_fixed;

  assert(channel >= 0);

  std::string port_name;
  long int file_length = 0;

  if (channel > 0) // we're dealing with a soundfile
  {
#ifdef ENABLE_ECASOUND
    // if not already running, start AudioPlayer
    if (!_audio_player)
    {
      _audio_player = AudioPlayer::ptr_t(new AudioPlayer);
    }
    port_name = _audio_player->get_port_name(file_name_or_port_number, channel
    // the thing with _loop is a temporary hack, should be removed some time:
        , _loop);
    file_length = _audio_player->get_file_length(file_name_or_port_number);
#else
    ERROR("Couldn't open audio file \"" << file_name_or_port_number
        << "\"! Ecasound was disabled at compile time.");
    return;
#endif
  }
  else  // no audio file
  {
    if (file_name_or_port_number != "")
    {
      port_name = _conf.input_port_prefix + file_name_or_port_number;
    }
  }

  if (port_name == "")
  {
    VERBOSE("No audio file or port specified for source '" << name << "'.");
  }

  apf::parameter_map p;
  p.set("connect_to", port_name);
  p.set("properties_file", properties_file);
  id_t id;

  try
  {
    auto guard = _renderer.get_scoped_lock();
    id = _renderer.add_source(p);
  }
  catch (std::exception& e)
  {
    ERROR(e.what());
    return;
  }

  _publish(&Subscriber::new_source, id);
  // mute while transmitting data
  _publish(&Subscriber::set_source_mute, id, true);
  _publish(&Subscriber::set_source_gain, id, gain);
 
  // make sure that source orientation is handled correctly
  this->set_source_position(id, position);

  _publish(&Subscriber::set_source_position_fixed, id, pos_fixed);
  
  // make sure that source orientation is handled correctly
  this->set_source_orientation(id, orientation);
    
  // _publish(&Subscriber::set_source_orientation_fix, id, or_fix);
  _publish(&Subscriber::set_source_name, id, name);
  _publish(&Subscriber::set_source_model, id, model);
  _publish(&Subscriber::set_source_port_name, id, port_name);
  if (file_name_or_port_number != "")
  {
    _publish(&Subscriber::set_source_file_name, id, file_name_or_port_number);
    _publish(&Subscriber::set_source_file_channel, id, channel);
  }
  _publish(&Subscriber::set_source_file_length, id, file_length);
  _publish(&Subscriber::set_source_properties_file, id, properties_file);
  // finally, unmute if requested
  _publish(&Subscriber::set_source_mute, id, muted);
}

template<typename Renderer>
void
Controller<Renderer>::delete_all_sources()
{
  _publish(&Subscriber::delete_all_sources);
#ifdef ENABLE_ECASOUND
  _audio_player.reset(); // shut down audio player
#endif
  // Wait until InternalInput objects are destroyed
  _renderer.wait_for_rt_thread();
}

template<typename Renderer>
void
Controller<Renderer>::delete_source(id_t id)
{
  _publish(&Subscriber::delete_source, id);
  // TODO: stop AudioPlayer if not needed anymore?
}

template<typename Renderer>
void
Controller<Renderer>::set_source_position(const id_t id, const Position& position)
{
  // TODO: check if the client who sent the request is actually allowed to
  // change the position. (same TODO as above)

  // TODO: check if position is inside of room boundaries.
  // if not: change position (e.g. single dimensions) to an allowed position

  // check if source may be moved
  if (!_scene.get_source_position_fixed(id))
  {
    _publish(&Subscriber::set_source_position, id, position);

    // make source face the reference
    if (_scene.get_auto_rotation())
    { 
      // new orientation will be published automatically
      orient_source_toward_reference(id);
    }
  }
  else
  {
    WARNING("Source \'" << _scene.get_source_name(id) << "\' cannot be moved.");
  }
}

template<typename Renderer>
void
Controller<Renderer>::set_source_orientation(const id_t id
    , const Orientation& orientation)
{
  // TODO: validate orientation?

  // check if source may be rotated
  if (!_scene.get_source_position_fixed(id))
  {
    if (_scene.get_auto_rotation()) 
    {
      VERBOSE2("Ignoring update of source orientation."
        << " Auto-rotation is enabled.");
    }
    else 
    {
      _publish(&Subscriber::set_source_orientation, id, orientation);
    }
  }
  else
  {
    WARNING("Source \'" << _scene.get_source_name(id) 
      << "\' cannot be rotated.");
  }
}

template<typename Renderer>
void 
Controller<Renderer>::orient_source_toward_reference(const id_t id)
{
  // take reference offset into account?
  
  _publish(&Subscriber::set_source_orientation, id
    , (_scene.get_reference().position - 
        *_scene.get_source_position(id)).orientation());
}

template<typename Renderer>
void 
Controller<Renderer>::orient_all_sources_toward_reference()
{
  typename SourceCopy::container_t sources;

  _scene.get_sources(sources);

  for (const auto& source: sources)
  {
    // check if sources may be rotated
    if (!_scene.get_source_position_fixed(source.id))
    {
      // new orientation will be published automatically
      orient_source_toward_reference(source.id);
    }
  }
}

template<typename Renderer>
void
Controller<Renderer>::set_source_gain(const id_t id, const float gain)
{
  _publish(&Subscriber::set_source_gain, id, gain);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_signal_level(const id_t id, const float level)
{
  _publish(&Subscriber::set_source_signal_level, id, level);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_mute(const id_t id, const bool mute)
{
  _publish(&Subscriber::set_source_mute, id, mute);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_name(const id_t id, const std::string& name)
{
  _publish(&Subscriber::set_source_name, id, name);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_properties_file(const id_t id, const std::string& name)
{
  _publish(&Subscriber::set_source_properties_file, id, name);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_model(const id_t id, const Source::model_t model)
{
  _publish(&Subscriber::set_source_model, id, model);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_port_name(const id_t id, const std::string& port_name)
{
  _publish(&Subscriber::set_source_port_name, id, port_name);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_file_name(const id_t id, const std::string& file_name)
{
  _publish(&Subscriber::set_source_file_name, id, file_name);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_file_channel(const id_t id, const int& channel)
{
  _publish(&Subscriber::set_source_file_channel, id, channel);
}

template<typename Renderer>
void
Controller<Renderer>::set_source_position_fixed(const id_t id, const bool fixed)
{
  _publish(&Subscriber::set_source_position_fixed, id, fixed);
}

template<typename Renderer>
void
Controller<Renderer>::set_reference_position(const Position& position)
{
  _publish(&Subscriber::set_reference_position, position);

  // make sources face the reference
  if (_scene.get_auto_rotation()) orient_all_sources_toward_reference();
}

template<typename Renderer>
void
Controller<Renderer>::set_reference_orientation(const Orientation& orientation)
{
  _publish(&Subscriber::set_reference_orientation, orientation);
}

template<typename Renderer>
void
Controller<Renderer>::set_reference_offset_position(const Position& position)
{
  _publish(&Subscriber::set_reference_offset_position, position);
  
  // make sources face the reference // has no effect currently
  //if (_scene.get_auto_rotation()) orient_all_sources_toward_reference();
}

template<typename Renderer>
void
Controller<Renderer>::set_reference_offset_orientation(const Orientation& orientation)
{
  _publish(&Subscriber::set_reference_offset_orientation, orientation);
}

// linear volume!
template<typename Renderer>
void
Controller<Renderer>::set_master_volume(const float volume)
{
  // TODO: validate volume?
  _publish(&Subscriber::set_master_volume, volume);
}

template<typename Renderer>
void
Controller<Renderer>::set_decay_exponent(const float exponent)
{
  // TODO: validate exponent?
  _publish(&Subscriber::set_decay_exponent, exponent);
}

template<typename Renderer>
void
Controller<Renderer>::set_amplitude_reference_distance(const float dist)
{
  if (dist > 1.0f)
  {
    _publish(&Subscriber::set_amplitude_reference_distance, dist);
  }
  else
  {
    ERROR("Amplitude reference distance cannot be smaller than 1.");
  }
}

// linear scale
template<typename Renderer>
void
Controller<Renderer>::set_master_signal_level(float level)
{
  _publish(&Subscriber::set_master_signal_level, level);
}

template<typename Renderer>
void
Controller<Renderer>::set_cpu_load(const float load)
{
  _publish(&Subscriber::set_cpu_load, load);
}

template<typename Renderer>
void
Controller<Renderer>::publish_sample_rate(const int sample_rate)
{
  _publish(&Subscriber::set_sample_rate, sample_rate);
}

template<typename Renderer>
std::string
Controller<Renderer>::get_renderer_name() const
{
  return _renderer.params.get("name", "");
}

template<typename Renderer>
bool
Controller<Renderer>::show_head() const
{
  return _renderer.show_head();
}

template<typename Renderer>
void
Controller<Renderer>::set_source_output_levels(id_t id, float* first
    , float* last)
{
  _publish(&Subscriber::set_source_output_levels, id, first, last);
}

template<typename Renderer>
std::string
Controller<Renderer>::get_scene_as_XML() const
{
  XMLParser xp; // load XML parser
  Node node = xp.new_node("update");

  // add master volume
  _add_master_volume(node);
  // quick hack: add transport state (play/stop)
  _add_transport_state(node);
  // TODO: add other scene information?
  _add_reference(node);
  _add_loudspeakers(node);
  _add_sources(node);

  // TODO: memory of node is never freed!

  return node.to_string();
}

template<typename Renderer>
bool
Controller<Renderer>::save_scene_as_XML(const std::string& filename) const
{
  // ATTENTION: this is an ugly work-around/quick-hack!
  // TODO: the following should be included into the XMLParser wrapper!

  XMLParser xp; // load XML parser
  Node root = xp.new_node("asdf");

  Node node = root.new_child("scene_setup");

  // add master volume
  _add_master_volume(node);

  // TODO: add other scene information?
  _add_reference(node);

  _add_sources(node, filename); // ugly quick hack!

  // TODO: memory of node is never freed!

  xmlDocPtr doc = xmlNewDoc(nullptr);
  //xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
  if (!doc) return false;

  xmlDocSetRootElement(doc, root.get());

  //xmlNewNs(my_node, BAD_CAST "http://www.w3.org/1999/xhtml", nullptr);

  //xmlSaveCtxtPtr ctxt = xmlSaveToFilename(filename.c_str(), nullptr, 0);
  xmlSaveCtxtPtr ctxt = xmlSaveToFilename(filename.c_str(), nullptr
      , XML_SAVE_FORMAT);
  if (!ctxt) return false;

  xmlSaveDoc(ctxt, doc);
  int ret = xmlSaveClose(ctxt);

  if (ret < 1) return false;
  else return true;

  // Memory for "doc" is never freed!
}

template<typename Renderer>
void
Controller<Renderer>::_add_master_volume(Node& node) const
{
  float volume = _scene.get_master_volume();
  node.new_child("volume", apf::str::A2S(apf::math::linear2dB(volume)));
}

template<typename Renderer>
void
Controller<Renderer>::_add_transport_state(Node& node) const
{
  node.new_child("transport"
      , _renderer.get_transport_state().first ? "start" : "stop");
}

template<typename Renderer>
void
Controller<Renderer>::_add_reference(Node& node) const
{
  DirectionalPoint reference = _scene.get_reference();
  Node reference_node = node.new_child("reference");
  _add_position(reference_node, reference.position);
  _add_orientation(reference_node, reference.orientation);
}

template<typename Renderer>
void
Controller<Renderer>::_add_position(Node& node
    , const Position& position, const bool fixed) const
{
  Node position_node = node.new_child("position");
  position_node.new_attribute("x", apf::str::A2S(position.x));
  position_node.new_attribute("y", apf::str::A2S(position.y));
  if (fixed)
  {
    position_node.new_attribute("fixed", apf::str::A2S(fixed));
  }
}

template<typename Renderer>
void
Controller<Renderer>::_add_orientation(Node& node, const Orientation& orientation) const
{
  Node orientation_node = node.new_child("orientation");
  orientation_node.new_attribute("azimuth", apf::str::A2S(orientation.azimuth));
}

template<typename Renderer>
void
Controller<Renderer>::_add_loudspeakers(Node& node) const
{
  Loudspeaker::container_t loudspeakers;
  _scene.get_loudspeakers(loudspeakers, false); // get relative positions
  for (const auto& ls: loudspeakers)
  {
    Node loudspeaker_node = node.new_child("loudspeaker");
    loudspeaker_node.new_attribute("model", apf::str::A2S(ls.model));
    _add_position(loudspeaker_node, ls.position);
    _add_orientation(loudspeaker_node, ls.orientation);
  }
}

template<typename Renderer>
void
Controller<Renderer>::_add_sources(Node& node
    , const std::string& scene_file_name) const
{
  typename SourceCopy::container_t sources;
  _scene.get_sources(sources);
  for (const auto& source: sources)
  {
    Node source_node = node.new_child("source");
    if (scene_file_name != "")
    {
      // ignore "id" -> ugly quick hack!
    }
    else
    {
      source_node.new_attribute("id", apf::str::A2S(source.id));
    }
    source_node.new_attribute("name", apf::str::A2S(source.name));
    source_node.new_attribute("model", apf::str::A2S(source.model));

    if (scene_file_name == "" || source.audio_file_channel > 0)
    {
      if (source.audio_file_name != "")
      {
        _add_audio_file_name(source_node
            , posixpathtools::make_path_relative_to_file(source.audio_file_name
              , scene_file_name), source.audio_file_channel);
      }
    }

    if (scene_file_name == "")
    {
      if (source.port_name != "")
      {
        source_node.new_child("port", source.port_name);
      }
    }
    else if (source.audio_file_channel == 0 && source.audio_file_name != "")
    {
      source_node.new_child("port", source.audio_file_name);
    }

    _add_position(source_node, source.position, source.fixed_position);
    _add_orientation(source_node, source.orientation);
    if (scene_file_name != "") // ugly quick hack
    {
      // don't add port name!
    }
    else
    {
      source_node.new_attribute("length", apf::str::A2S(source.file_length));
    }
    source_node.new_attribute("mute", apf::str::A2S(source.mute));
    // save volume in dB!
    source_node.new_attribute("volume", apf::str::A2S(apf::math::linear2dB(source.gain)));
    // TODO: information about mirror sources

    if (source.properties_file != "")
    {
      source_node.new_attribute("properties_file"
          , posixpathtools::make_path_relative_to_file(source.properties_file
            , scene_file_name));
    }

    // TODO: save doppler effect setting (source.doppler_effect)
  }
}

template<typename Renderer>
void
Controller<Renderer>::_add_audio_file_name(Node& node, const std::string& name
    , int channel) const
{
  Node file_node = node.new_child("file", name);
  if (channel != 1)
  {
    file_node.new_attribute("channel", apf::str::A2S(channel));
  }
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
