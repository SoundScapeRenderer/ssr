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

#include <functional>  // for std::function
#include <type_traits>  // for std::is_same_v
#include <regex>
#include <unordered_set>

#include "apf/jack_policy.h"
#include "apf/cxx_thread_policy.h"

#define SSR_QUERY_POLICY apf::enable_queries

#include <libxml/xmlsave.h> // temporary hack!

#include "ssr_global.h"
#include "api.h"

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
#include "legacy_network/server.h"
#include "legacy_xmlsceneprovider.h"  // for LegacyXmlSceneProvider
#endif
#ifdef ENABLE_WEBSOCKET_INTERFACE
#include "websocket/server.h"  // for ws::Server
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

#include "legacy_scene.h"  // for LegacyScene
#include "scene.h"  // for Scene
#include "rendersubscriber.h"  // for RenderSubscriber

#include "geometry.h"  // for look_at()
#include "posixpathtools.h"
#include "apf/math.h"
#include "apf/stringtools.h"

namespace ssr
{

using Node = XMLParser::Node; ///< a node of the DOM tree

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


/// %Controller class.  Implements the Publisher interface.
/// The Controller can be either a "leader" or a "follower".
template<typename Renderer>
class Controller : public api::Publisher
#ifdef ENABLE_IP_INTERFACE
                 , public LegacyXmlSceneProvider
#endif
{
  public:
    /// ctor
    Controller(int argc, char *argv[]);
    virtual ~Controller(); ///< dtor

    bool run();

  private:
    using output_list_t
      = typename Renderer::template rtlist_proxy<typename Renderer::Output>;

    template<typename X = api::SceneControlEvents> class CommonInterface;
    template<typename X> class ControlInterface;
    class FollowerInterface;

    class query_state;

    // Ordered list, keeps subscription order
    template<typename Events> using Subscribers = std::vector<Events*>;

    class SubscribeHelper;

    std::unique_ptr<api::SubscribeHelper> subscribe() override
    {
      return std::make_unique<SubscribeHelper>(*this);
    }

    class Subscription : public api::Subscription
    {
    public:
      explicit Subscription(std::function<void()> f) : _f(std::move(f)) {}
      ~Subscription() { _f(); }

    private:
      std::function<void()> _f;
    };

    std::unique_ptr<api::Subscription> attach_leader(api::Controller*) override;

    std::unique_ptr<api::Controller> take_control(api::SceneControlEvents*) override;
    std::unique_ptr<api::Controller> take_control() override;
    std::unique_ptr<api::Follower> update_follower() override;

    unsigned int get_source_number(id_t source_id) const override;

#ifdef ENABLE_IP_INTERFACE
    std::string get_scene_as_XML() const override;
#endif

    bool _load_scene(const std::string& filename);
    bool _save_scene(const std::string& filename) const;

    void _new_source(const std::string& id, const std::string& name
      , const std::string& model, const std::string& file_name_or_port_number
      , int channel, const Pos& position, const Rot& rotation, bool fixed
      , float volume, bool mute, const std::string& properties_file);

    void _delete_all_sources();

    void _orient_source_toward_reference(id_t id);
    void _orient_all_sources_toward_reference();

    void _calibrate_client();

    void _transport_locate_frames(uint32_t time);

    template<typename Events>
    bool _subscribe(Events* subscriber)
    {
      assert(subscriber);
      auto& list = std::get<Subscribers<Events>>(_subscribers);
      if (std::find(list.begin(), list.end(), subscriber) == list.end())
      {
        list.push_back(subscriber);
        return true;
      }
      return false;
    }

    template<typename Events>
    void _unsubscribe(Events* subscriber)
    {
      assert(subscriber);
      auto& list = std::get<Subscribers<Events>>(_subscribers);
      auto iter = std::find(list.begin(), list.end(), subscriber);
      if (iter != list.end())
      {
        list.erase(iter);
      }
    }

    template<typename PTMF, typename... Args>
    void _call_leader(PTMF member_function, Args&&... args)
    {
      if (_leader)
      {
        try { (_leader->*member_function)(std::forward<Args>(args)...); }
        catch (std::exception& e) { SSR_ERROR(e.what()); }
      }
      else
      {
        SSR_WARNING("Instance is configured as \"follower\", "
            "but no \"leader\" is connected");
      }
    }

    /// Dummy class to select relevant _publish() overload.
    struct ToLeaderTag {};

    /// Low-level publishing function, overload for SceneControlEvents.
    /// The first argument is the initiator of the request (or nullptr).
    /// The second argument is a pointer to a member function of
    /// SceneControlEvents, the rest are arguments to said member function.
    // NB: Args must be convertible to FuncArgs, but they don't necessarily have
    //     to be the same types.
    template<typename R, typename... FuncArgs, typename... Args>
    void _publish(api::SceneControlEvents* initiator
        , R (api::SceneControlEvents::*f)(FuncArgs...), Args&&... args)
    {
      // TODO: check if sender is allowed to move source?

      for (api::SceneControlEvents* subscriber:
           std::get<Subscribers<api::SceneControlEvents>>(_subscribers))
      {
        // NB: If initiator is nullptr this is always true:
        if (subscriber != initiator)
        {
          try { (subscriber->*f)(std::forward<Args>(args)...); }
          catch (std::exception& e) { SSR_ERROR(e.what()); }
        }
      }
    }

    /// Overload for SceneControlEvents to be sent to the "leader".
    /// The first argument is a dummy argument for selecting this overload.
    template<typename R, typename... FuncArgs, typename... Args>
    void _publish(ToLeaderTag*
        , R (api::SceneControlEvents::*f)(FuncArgs...), Args&&... args)
    {
      assert(_conf.follow);
      try { _call_leader(f, std::forward<Args>(args)...); }
      catch (std::exception& e) { SSR_ERROR(e.what()); }
    }

    /// Overload for all events without options to suppress own messages.
    /// Those are never sent to the "leader", therefore it is not allowed to use
    /// this for SceneControlEvents on a "follower".
    template<typename R, typename C, typename... FuncArgs, typename... Args>
    void _publish(R (C::*f)(FuncArgs...), Args&&... args)
    {
      // NB: Nothing is sent to the "leader"

      if constexpr (std::is_same_v<C, api::SceneControlEvents>)
      {
        // In a "follower", those must use separate overload from above
        assert(!_conf.follow);
      }

      for (C* subscriber: std::get<Subscribers<C>>(_subscribers))
      {
        try { (subscriber->*f)(std::forward<Args>(args)...); }
        catch (std::exception& e) { SSR_ERROR(e.what()); }
      }
    }

    std::vector<Loudspeaker> _get_loudspeakers() const
    {
      std::vector<LegacyLoudspeaker> loudspeakers;
      _renderer.get_loudspeakers(loudspeakers);
      return {loudspeakers.begin(), loudspeakers.end()};
    }

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
    LegacyScene _legacy_scene;

    std::tuple<
      Subscribers<api::BundleEvents>,
      Subscribers<api::SceneControlEvents>,
      Subscribers<api::SceneInformationEvents>,
      Subscribers<api::TransportFrameEvents>,
      Subscribers<api::RendererControlEvents>,
      Subscribers<api::RendererInformationEvents>,
      Subscribers<api::SourceMetering>,
      Subscribers<api::MasterMetering>,
      Subscribers<api::OutputActivity>,
      Subscribers<api::CpuLoad>
    > _subscribers;
    api::Controller* _leader = nullptr;
#ifdef ENABLE_GUI
    std::unique_ptr<QGUI> _gui;
#endif

    Renderer _renderer;
    RenderSubscriber<Renderer> _rendersubscriber;

    query_state _query_state;
#ifdef ENABLE_ECASOUND
    AudioRecorder::ptr_t    _audio_recorder; ///< pointer to audio recorder
    AudioPlayer::ptr_t      _audio_player;   ///< pointer to audio player
#endif
#ifdef ENABLE_IP_INTERFACE
    std::unique_ptr<legacy_network::Server> _network_interface;
#endif
#ifdef ENABLE_WEBSOCKET_INTERFACE
    std::unique_ptr<ws::Server> _websocket_interface;
#endif
    std::unique_ptr<Tracker> _tracker;

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

    std::unique_ptr<typename Renderer::QueryThread> _query_thread;

    std::mutex _m;

    // "non-colonized name": https://www.w3.org/TR/xml-names11/#NT-NCName
    std::regex _re_ncname{
      // NCNameStartChar
      "["
      "A-Z_a-z"
      "\u00C0-\u00D6\u00D8-\u00F6\u00F8-\u02FF\u0370-\u037D"
      "\u037F-\u1FFF\u200C-\u200D\u2070-\u218F\u2C00-\u2FEF"
      "\u3001-\uD7FF\uF900-\uFDCF\uFDF0-\uFFFD\U00010000-\U000EFFFF"
      "]"
      // NCNameChar*
      "["
      "-.0-9\u00B7\u0300-\u036F\u203F-\u2040"
      "A-Z_a-z"
      "\u00C0-\u00D6\u00D8-\u00F6\u00F8-\u02FF\u0370-\u037D"
      "\u037F-\u1FFF\u200C-\u200D\u2070-\u218F\u2C00-\u2FEF"
      "\u3001-\uD7FF\uF900-\uFDCF\uFDF0-\uFFFD\U00010000-\U000EFFFF"
      "]*"};
};

template<typename Renderer>
Controller<Renderer>::Controller(int argc, char* argv[])
  : _argc(argc)
  , _argv(argv)
  , _conf(configuration(_argc, _argv))
  , _renderer(_conf.renderer_params)
  , _rendersubscriber(_renderer)
  , _query_state(query_state(*this, _renderer))
  , _loop(_conf.loop)  // temporary solution
{
  // TODO: signal handling?

  internal::print_about_message();

#ifndef ENABLE_IP_INTERFACE
  if (_conf.ip_server)
  {
    throw std::logic_error(_conf.exec_name
        + " was compiled without IP-server support!\n"
        "Use the --no-ip-server option to disable the IP-server.\n"
        "Type '" + _conf.exec_name + " --help' for more information.");
  }
#endif

#ifndef ENABLE_WEBSOCKET_INTERFACE
  if (_conf.websocket_server)
  {
    throw std::logic_error(_conf.exec_name
        + " was compiled without WebSocket server support!\n"
        "Use --no-websocket-server to disable the WebSocket server.\n"
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

  if ((_conf.ip_server || _conf.websocket_server) && _conf.freewheeling)
  {
    SSR_WARNING("Freewheel mode cannot be used together with "
        "--ip-server or --websocket-server. Ignored.\n"
        "Type '" + _conf.exec_name + " --help' for more information.");
    _conf.freewheeling = false;
  }

  if (_conf.freewheeling && _conf.gui)
  {
    SSR_WARNING("In 'freewheeling' mode the GUI cannot be used! Disabled.\n"
        "Type '" + _conf.exec_name + " --help' for more information.");
    _conf.gui = false;
  }

  // NB: we don't need a lock here because nothing is publishing yet

  _subscribe<api::SceneControlEvents>(&_scene);
  _subscribe<api::SceneInformationEvents>(&_scene);

  _subscribe<api::SceneControlEvents>(&_legacy_scene);
  _subscribe<api::SceneInformationEvents>(&_legacy_scene);
  _subscribe<api::RendererControlEvents>(&_legacy_scene);
  _subscribe<api::RendererInformationEvents>(&_legacy_scene);
  _subscribe<api::TransportFrameEvents>(&_legacy_scene);
  _subscribe<api::SourceMetering>(&_legacy_scene);
  _subscribe<api::MasterMetering>(&_legacy_scene);
  _subscribe<api::OutputActivity>(&_legacy_scene);
  _subscribe<api::CpuLoad>(&_legacy_scene);

  _subscribe<api::BundleEvents>(&_rendersubscriber);
  _subscribe<api::SceneControlEvents>(&_rendersubscriber);
  _subscribe<api::RendererControlEvents>(&_rendersubscriber);
  _subscribe<api::RendererInformationEvents>(&_rendersubscriber);

  // End of initial subscriptions, start publishing ...

  _publish(&api::RendererInformationEvents::renderer_name
      , _renderer.name());

  _renderer.load_reproduction_setup();

  _publish(&api::RendererInformationEvents::loudspeakers, _get_loudspeakers());

#ifdef ENABLE_ECASOUND
  _load_audio_recorder(_conf.audio_recorder_file_name);
#endif

  if (!_conf.follow)
  {
    _publish(&api::SceneControlEvents::auto_rotate_sources
        , _conf.auto_rotate_sources);
    _publish(&api::SceneInformationEvents::sample_rate
        , _renderer.sample_rate());
    bool is_rolling;
    std::tie(is_rolling, std::ignore) = _renderer.get_transport_state();
    // This is may not be necessary since the renderer continuously sends this?
    _publish(&api::SceneInformationEvents::transport_rolling, is_rolling);
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

    SSR_VERBOSE("Starting IP Server with port " << _conf.server_port
        << " and with end-of-message character with ASCII code " <<
        _conf.end_of_message_character << ".");

    _network_interface = std::make_unique<legacy_network::Server>(*this, *this
        , _conf.server_port, static_cast<char>(_conf.end_of_message_character));
    _network_interface->start();
  }
#endif // ENABLE_IP_INTERFACE

#ifdef ENABLE_WEBSOCKET_INTERFACE
  if (_conf.websocket_server)
  {
    SSR_VERBOSE("Starting WebSocket server with port " << _conf.websocket_port);
    _websocket_interface = std::make_unique<ws::Server>(*this
        , _conf.websocket_port, _conf.websocket_resource_directory);
  }
#endif // ENABLE_WEBSOCKET_INTERFACE

  if (_conf.follow && _conf.scene_file_name != "")
  {
    // TODO: if "follower": connect to "leader" before loading scene
    throw std::logic_error(
        "Loading a scene as \"follower\" is not yet supported");
  }

  auto control = this->take_control();  // Start a bundle for scene loading

  if (_conf.scene_file_name != "" && !_load_scene(_conf.scene_file_name))
  {
    throw std::runtime_error("Couldn't load scene!");
  }
}

template<typename Renderer>
class Controller<Renderer>::query_state
{
  public:
    query_state(Controller& controller, Renderer& renderer)
      : _controller(controller)
      , _renderer(renderer)
    {}

    // NB: This is executed in the audio thread
    void query()
    {
      if (!_controller._conf.follow)
      {
        _state = _renderer.get_transport_state();
      }
      _cpu_load = _renderer.get_cpu_load();

      auto output_list = output_list_t(_renderer.get_output_list());

      _master_level = {};
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

          assert(levels->outputs.size() == _renderer.get_output_list().size());
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

    // NB: This is executed in the control thread
    void update()
    {
      auto control = _controller.take_control();  // Scoped bundle

      if (!_controller._conf.follow)
      {
        _controller._publish(&api::TransportFrameEvents::transport_frame
            , _state.second);
        bool rolling{_state.first};
        if (rolling != _controller._scene.transport_is_rolling())
        {
          _controller._publish(&api::SceneInformationEvents::transport_rolling
              , rolling);
        }
      }
      _controller._publish(&api::CpuLoad::cpu_load, _cpu_load);
      _controller._publish(&api::MasterMetering::master_level, _master_level);

      if (!_discard_source_levels)
      {
        for (auto& item: _source_levels)
        {
          _controller._publish(&api::SourceMetering::source_level
              , item.source_id, item.source_level);

          // TODO: make this a compile-time decision:
          if (item.outputs_available)
          {
            assert(item.outputs.size() == _renderer.get_output_list().size());
            _controller._publish(&api::OutputActivity::output_activity
                , item.source_id, &*item.outputs.begin(), &*item.outputs.end());
          }
        }
      }
      else
      {
        // NB: The output list should only be accessed by the audio thread.
        //     Since the number of outputs is never changed, that's OK here.
        _source_levels.resize(_new_size
          , SourceLevel(_renderer.get_output_list().size()));
      }
    }

  private:
    struct SourceLevel
    {
      explicit SourceLevel(size_t number_of_outputs)
        : outputs(number_of_outputs)
      {}

      typename Renderer::sample_type source_level;

      std::string source_id;

      bool outputs_available{false};
      // this must never be resized:
      std::vector<typename Renderer::sample_type> outputs;
    };

    using source_levels_t = std::vector<SourceLevel>;

    Controller& _controller;
    const Renderer& _renderer;
    std::pair<bool, uint32_t> _state;
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
  _query_thread = _renderer.make_query_thread(10 * 1000);

  _start_tracker(_conf.tracker, _conf.tracker_ports);

  if (!_renderer.activate())
  {
    return false;
  }

  // CAUTION: this must be called after activate()!
  // If not, an infinite recursion happens!

  _renderer.new_query(_query_state);

  {
    auto control = this->take_control();
    control->processing(true);
    if (!_conf.follow)
    {
      control->transport_locate_frames(0);
    }
  }

  if (_conf.gui)
  {
#ifdef ENABLE_GUI
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
    this->take_control()->transport_start();
    bool keep_running = true;
    while (keep_running)
    {
      switch(fgetc(stdin))
      {
        case 'c':
          std::cout << "Calibrating client.\n";
          this->take_control()->reset_tracker();
          break;
        case 'p':
          this->take_control()->transport_start();
          break;
        case 'q':
          keep_running = false;
          break;
        case 'r':
          this->take_control()->transport_locate_frames(0);
          break;
        case 's':
          this->take_control()->transport_stop();
          break;
      }
    }
  }
  return true;
}

template<typename Renderer>
Controller<Renderer>::~Controller()
{
  if (!_conf.follow)
  {
    auto control = this->take_control();

    // TODO: check if transport needs to be stopped
    control->transport_stop();

    // NB: Scene is save while holding the lock
    if (!_save_scene("ssr_scene_autosave.asd"))
    {
      SSR_ERROR("Couldn't write XML scene! (It's an ugly hack anyway ...");
    }
  }
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
        SSR_ERROR("Invalid position!");
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
        SSR_ERROR("Invalid orientation!");
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
get_attribute_of_node(const Node& node, const std::string& attribute
    , const T default_value)
{
  if (!node) return default_value;
  return apf::str::S2RV(node.get_attribute(attribute), default_value);
}

/// Overload for const char* default value
inline std::string
get_attribute_of_node(const Node& node, const std::string& attribute
    , const char* default_value)
{
  std::string temp{default_value};
  return get_attribute_of_node(node, attribute, temp);
}

/** check for file/port
 * @param node parent node
 * @param file_name_or_port_number a string where the obtained file name or
 * port number is stored.
 * @return channel number, 0 if port was given.
 * @note on error, @a file_name_or_port_number is set to the empty string ""
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

} // end of namespace "internal"


template<typename Renderer>
template<typename X>
class Controller<Renderer>::CommonInterface
                                        : virtual public api::SceneControlEvents
{
public:
  explicit CommonInterface(X* initiator, Controller<Renderer>& controller)
    : _initiator(initiator)
    , _controller(controller)
    , _lock(_controller._m)
  {
    _controller._publish(&api::BundleEvents::bundle_start);
  }

  ~CommonInterface()
  {
    _controller._publish(&api::BundleEvents::bundle_stop);
  }

  // SceneControlEvents

  void auto_rotate_sources(bool auto_rotate) override
  {
    _controller._publish(_initiator,
        &api::SceneControlEvents::auto_rotate_sources, auto_rotate);

    if constexpr (_is_leader)
    {
      if (auto_rotate)
      {
        _controller._orient_all_sources_toward_reference();
        SSR_VERBOSE("Auto-rotation of sound sources is enabled.");
      }
      else
      {
        SSR_VERBOSE("Auto-rotation of sound sources is disabled.");
      }
    }
  }

  void delete_source(id_t id) override
  {
    // TODO: check if source actually exists?
    _controller._publish(_initiator,
        &api::SceneControlEvents::delete_source, id);

    // TODO: stop AudioPlayer if not needed anymore?
  }

  void source_position(id_t id, const Pos& position) override
  {
    if constexpr (_is_leader)
    {
      auto* src = _controller._scene.get_source(id);
      if (src == nullptr)
      {
        SSR_WARNING("Source \"" << id << "\" does not exist.");
        return;
      }
      else if (src->fixed)
      {
        SSR_WARNING("Source \"" << id << "\" cannot be moved because it is fixed.");
        return;
      }
    }
    _controller._publish(_initiator,
        &api::SceneControlEvents::source_position, id, position);
    if constexpr (_is_leader)
    {
      if (_controller._scene.get_auto_rotation())
      {
        _controller._orient_source_toward_reference(id);
      }
    }
  }

  void source_rotation(id_t id, const Rot& rotation) override
  {
    if constexpr (_is_leader)
    {
      if (_controller._scene.get_auto_rotation())
      {
        SSR_VERBOSE2("Ignoring update of source rotation."
            << " Auto-rotation is enabled.");
        return;
      }
      auto* src = _controller._scene.get_source(id);
      if (src == nullptr)
      {
        SSR_WARNING("Source \"" << id << "\" does not exist.");
        return;
      }
      if (src->fixed)
      {
        SSR_WARNING("Source \"" << id
            << "\" cannot be rotated because it is fixed.");
        return;
      }
    }
    _controller._publish(_initiator,
        &api::SceneControlEvents::source_rotation, id, rotation);
  }

  void source_volume(id_t id, float volume) override
  {
    _controller._publish(_initiator
        , &api::SceneControlEvents::source_volume, id, volume);
  }

  void source_mute(id_t id, bool mute) override
  {
    _controller._publish(_initiator
        , &api::SceneControlEvents::source_mute, id, mute);
  }

  void source_name(id_t id, const std::string& name) override
  {
    _controller._publish(_initiator
        , &api::SceneControlEvents::source_name, id, name);
  }

  void source_model(id_t id, const std::string& model) override
  {
    _controller._publish(_initiator
        , &api::SceneControlEvents::source_model, id, model);
  }

  void source_fixed(id_t id, bool fixed) override
  {
    _controller._publish(_initiator
        , &api::SceneControlEvents::source_fixed, id, fixed);
  }

  void reference_position(const Pos& position) override
  {
    _controller._publish(_initiator,
        &api::SceneControlEvents::reference_position, position);
    if constexpr (_is_leader)
    {
      if (_controller._scene.get_auto_rotation())
      {
        _controller._orient_all_sources_toward_reference();
      }
    }
  }

  void reference_rotation(const Rot& rotation) override
  {
    _controller._publish(_initiator,
        &api::SceneControlEvents::reference_rotation, rotation);
  }

  void master_volume(float volume) override
  {
    _controller._publish(_initiator
        , &api::SceneControlEvents::master_volume, volume);
  }

  void decay_exponent(float exponent) override
  {
    _controller._publish(_initiator
        , &api::SceneControlEvents::decay_exponent, exponent);
  }

  void amplitude_reference_distance(float dist) override
  {
    if (dist > 1.0f)
    {
      _controller._publish(_initiator,
          &api::SceneControlEvents::amplitude_reference_distance, dist);
    }
    else
    {
      SSR_ERROR("Amplitude reference distance cannot be smaller than 1.");
    }
  }

protected:
  static constexpr bool _is_leader = !std::is_same_v<X, ToLeaderTag>;
  X* _initiator;
  Controller<Renderer>& _controller;
  std::lock_guard<std::mutex> _lock;
};


template<typename Renderer>
template<typename X>
class Controller<Renderer>::ControlInterface : public CommonInterface<X>
                                             , public api::Controller
{
public:
  using CommonInterface<X>::CommonInterface;

  void load_scene(const std::string& filename) override
  {
    if constexpr (_is_leader)
    {
      if (!_controller._load_scene(filename))
      {
        SSR_WARNING("Loading scene \"" << filename << "\" failed");
      }
    }
    else
    {
      _controller._call_leader(&api::Controller::load_scene, filename);
    }
  }

  void save_scene(const std::string& filename) const override
  {
    if constexpr (_is_leader)
    {
      if (!_controller._save_scene(filename))
      {
        SSR_WARNING("Saving scene \"" << filename << "\" failed");
      }
    }
    else
    {
      _controller._call_leader(&api::Controller::save_scene, filename);
    }
  }

  void new_source(const std::string& id, const std::string& name
      , const std::string& model, const std::string& file_name_or_port_number
      , int channel, const Pos& position, const Rot& rotation, bool fixed
      , float volume, bool mute, const std::string& properties_file) override
  {
    if constexpr (_is_leader)
    {
      _controller._new_source(id, name, model, file_name_or_port_number
          , channel, position, rotation, fixed, volume, mute, properties_file);
    }
    else
    {
      _controller._call_leader(&api::Controller::new_source, id, name,  model
          , file_name_or_port_number, channel, position, rotation, fixed
          , volume, mute, properties_file);
    }
  }

  void delete_all_sources() override
  {
    if constexpr (_is_leader)
    {
      _controller._delete_all_sources();
    }
    else
    {
      _controller._call_leader(&api::Controller::delete_all_sources);
    }
  }

  void transport_start() override
  {
    if constexpr (_is_leader)
    {
      _controller._renderer.transport_start();
    }
    else
    {
      _controller._call_leader(&api::Controller::transport_start);
    }
  }

  void transport_stop() override
  {
    if constexpr (_is_leader)
    {
      _controller._renderer.transport_stop();
    }
    else
    {
      _controller._call_leader(&api::Controller::transport_stop);
    }
  }

  void transport_locate_frames(uint32_t time) override
  {
    if constexpr (_is_leader)
    {
      _controller._transport_locate_frames(time);
    }
    else
    {
      _controller._call_leader(&api::Controller::transport_locate_frames, time);
    }
  }

  void transport_locate_seconds(float time) override
  {
    if constexpr (_is_leader)
    {
      auto frames = static_cast<uint32_t>(
          time * _controller._renderer.sample_rate());
      _controller._transport_locate_frames(frames);
    }
    else
    {
      _controller._call_leader(&api::Controller::transport_locate_seconds
          , time);
    }
  }

  void reset_tracker() override
  {
    // NB: This is not sent to the leader
    _controller._calibrate_client();
  }

  std::string get_source_id(unsigned source_number) const override
  {
    return _controller._scene.get_source_id(source_number);
  }

  // RendererControlEvents

  void processing(bool state) override
  {
    _controller._publish(&api::RendererControlEvents::processing, state);
  }

  void reference_position_offset(const Pos& position) override
  {
    _controller._publish(&api::RendererControlEvents::reference_position_offset
        , position);
  }

  void reference_rotation_offset(const Rot& rotation) override
  {
    _controller._publish(&api::RendererControlEvents::reference_rotation_offset
        , rotation);
  }

private:
  using CommonInterface<X>::_is_leader;
  using CommonInterface<X>::_controller;
};


template<typename Renderer>
class Controller<Renderer>::FollowerInterface : public CommonInterface<>
                                              , public api::Follower
{
private:
  using CommonInterface<>::_controller;

public:
  explicit FollowerInterface(Controller<Renderer>& controller)
    : CommonInterface<>(nullptr, controller)
  {}

  // SceneControlEvents are inherited from CommonInterface

  // SceneInformationEvents

  void sample_rate(int rate) override
  {
    _controller._publish(&api::SceneInformationEvents::sample_rate, rate);
  }

  void new_source(id_t id) override
  {
    _controller._publish(&api::SceneInformationEvents::new_source, id);
  }

  void source_property(id_t id, const std::string& key
                              , const std::string& value) override
  {
    _controller._publish(&api::SceneInformationEvents::source_property
        , id, key, value);
  }

  void transport_rolling(bool rolling) override
  {
    _controller._publish(&api::SceneInformationEvents::transport_rolling
        , rolling);
  }

  // TransportFrameEvents

  void transport_frame(uint32_t frame) override
  {
    _controller._publish(&api::TransportFrameEvents::transport_frame, frame);
  }

  // SourceMetering

  void source_level(id_t id, float level) override
  {
    _controller._publish(&api::SourceMetering::source_level, id, level);
  }
};


template<typename Renderer>
class Controller<Renderer>::SubscribeHelper : public api::SubscribeHelper
{
public:
  explicit SubscribeHelper(Controller<Renderer>& controller)
    : _controller(controller)
    , _lock(_controller._m)
  {
    _controller._publish(&api::BundleEvents::bundle_start);
  }

  ~SubscribeHelper()
  {
    _controller._publish(&api::BundleEvents::bundle_stop);
  }

private:
  struct _noop { void operator()() {} };

  template<typename Events, typename F = _noop>
  std::unique_ptr<api::Subscription>
  _subscribe_helper(Events* subscriber, F&& init_func = F{})
  {
    if (_controller._subscribe<Events>(subscriber))
    {
      std::forward<F>(init_func)();
      auto* ctrl_ptr = &_controller;
      // NB: Capturing pointer by value, because SubscribeHelper will die soon:
      return std::make_unique<Subscription>([ctrl_ptr, subscriber]() {
          assert(ctrl_ptr);
          assert(subscriber);
          std::lock_guard lock{ctrl_ptr->_m};
          ctrl_ptr->template _unsubscribe<Events>(subscriber);
      });
    }
    throw std::runtime_error("Repeated subscriptions are not allowed");
  }

  std::unique_ptr<api::Subscription>
  bundle(api::BundleEvents* subscriber) override
  {
    return _subscribe_helper(subscriber, [this, subscriber]() {
        subscriber->bundle_start();
    });
  }

  std::unique_ptr<api::Subscription>
  scene_control(api::SceneControlEvents* subscriber) override
  {
    return _subscribe_helper(subscriber, [this, subscriber]() {
        _controller._scene.get_data(subscriber);
    });
  }

  std::unique_ptr<api::Subscription>
  scene_information(api::SceneInformationEvents* subscriber) override
  {
    return _subscribe_helper(subscriber, [this, subscriber]() {
        _controller._scene.get_data(subscriber);
    });
  }

  std::unique_ptr<api::Subscription>
  renderer_control(api::RendererControlEvents* subscriber) override
  {
    return _subscribe_helper(subscriber, [this, subscriber]() {
        _controller._rendersubscriber.get_data(subscriber);
    });
  }

  std::unique_ptr<api::Subscription>
  renderer_information(api::RendererInformationEvents* subscriber) override
  {
    return _subscribe_helper(subscriber, [this, subscriber]() {
        _controller._rendersubscriber.get_data(subscriber);
    });
  }

  std::unique_ptr<api::Subscription>
  transport_frame(api::TransportFrameEvents* subscriber) override
  {
    return _subscribe_helper(subscriber);
  }

  std::unique_ptr<api::Subscription>
  source_metering(api::SourceMetering* subscriber) override
  {
    return _subscribe_helper(subscriber);
  }

  std::unique_ptr<api::Subscription>
  master_metering(api::MasterMetering* subscriber) override
  {
    return _subscribe_helper(subscriber);
  }

  std::unique_ptr<api::Subscription>
  output_activity(api::OutputActivity* subscriber) override
  {
    return _subscribe_helper(subscriber);
  }

  std::unique_ptr<api::Subscription>
  cpu_load(api::CpuLoad* subscriber) override
  {
    return _subscribe_helper(subscriber);
  }

  Controller<Renderer>& _controller;
  std::lock_guard<std::mutex> _lock;
};


template<typename Renderer>
std::unique_ptr<api::Subscription>
Controller<Renderer>::attach_leader(api::Controller* leader)
{
  if (!_conf.follow)
  {
    throw std::logic_error(
        "\"attach_leader()\" can only be called on \"follower\"");
  }
  std::lock_guard lock{_m};
  if (_leader != nullptr)
  {
    throw std::runtime_error("A \"leader\" is already subscribed");
  }
  _leader = leader;
  return std::make_unique<Subscription>([this]() {
    std::lock_guard lock{_m};
    _leader = nullptr;
  });
}


template<typename Renderer>
std::unique_ptr<api::Controller>
Controller<Renderer>::take_control(api::SceneControlEvents* suppress_own)
{
  if (_conf.follow)
  {
    if (suppress_own != nullptr)
    {
      throw std::logic_error(
          "\"suppress_own\" can only be used directly on \"leader\"");
    }
    return std::make_unique<ControlInterface<ToLeaderTag>>(
        nullptr, *this);
  }
  else
  {
    return std::make_unique<ControlInterface<api::SceneControlEvents>>(
        suppress_own, *this);
  }
}


template<typename Renderer>
std::unique_ptr<api::Controller>
Controller<Renderer>::take_control()
{
  return this->take_control(nullptr);
}


template<typename Renderer>
std::unique_ptr<api::Follower>
Controller<Renderer>::update_follower()
{
  return std::make_unique<FollowerInterface>(*this);
}


template<typename Renderer>
unsigned int
Controller<Renderer>::get_source_number(id_t source_id) const
{
  return _scene.get_source_number(source_id);
}


template<typename Renderer>
bool
Controller<Renderer>::_load_scene(const std::string& scene_file_name)
{
  assert(!_conf.follow);

  // remove all existing sources (if any)
  _delete_all_sources();

#ifdef ENABLE_ECASOUND
  _audio_player.reset();  // shut down audio player
#endif

  if (scene_file_name == "")
  {
    SSR_VERBOSE("No scene file specified. Opening empty scene ...");
    return true;
  }

  std::string file_extension = posixpathtools::get_file_extension(scene_file_name);

  if (file_extension == "")
  {
    SSR_ERROR("File name '" << scene_file_name << "' does not have an extension.");
    return false;
  }
  else if (file_extension == "asd")
  {
    XMLParser xp; // load XML parser
    auto scene_file = xp.load_file(scene_file_name);
    if (!scene_file)
    {
      SSR_ERROR("Unable to load scene setup file '" << scene_file_name << "'!");
      return false;
    }

    if (_conf.xml_schema == "")
    {
      SSR_ERROR("No schema file specified!");
      // TODO: return true and continue anyway?
      return false;
    }
    else if (scene_file->validate(_conf.xml_schema))
    {
      SSR_VERBOSE("Valid scene setup (" << scene_file_name << ").");
    }
    else
    {
      SSR_ERROR("Error validating '" << scene_file_name << "' with schema '"
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
      SSR_WARNING("Invalid master volume specified in scene!");
      master_volume = 0.0f;
    }

    SSR_VERBOSE("Setting master volume to " << master_volume << " dB.");
    _publish(&api::SceneControlEvents::master_volume
        , apf::math::dB2linear(master_volume));

    // GET DECAY EXPONENT
    auto exponent = _conf.renderer_params.get<float>("decay_exponent");

    xpath_result = scene_file->eval_xpath("//scene_setup/decay_exponent");
    if (xpath_result)
    {
      if (!apf::str::S2A(get_content(xpath_result->node()), exponent))
      {
        SSR_WARNING("Invalid amplitude decay exponent!");
      }
    }

    // always use default value when nothing is specified
    SSR_VERBOSE("Setting amplitude decay exponent to " << exponent << ".");
    _publish(&api::SceneControlEvents::decay_exponent, exponent);

    // GET AMPLITUDE REFERENCE DISTANCE

    auto ref_dist = _conf.renderer_params.get<float>(
        "amplitude_reference_distance");  // throws on error!

    xpath_result
      = scene_file->eval_xpath("//scene_setup/amplitude_reference_distance");
    if (xpath_result)
    {
      if (!apf::str::S2A(get_content(xpath_result->node()), ref_dist))
      {
        SSR_WARNING("Invalid amplitude reference distance!");
      }
    }

    // always use default value when nothing is specified
    SSR_VERBOSE("Setting amplitude reference distance to "
        << ref_dist << " meters.");
    _publish(&api::SceneControlEvents::amplitude_reference_distance, ref_dist);

    // LOAD REFERENCE

    std::unique_ptr<internal::PositionPlusBool> pos_ptr;
    std::unique_ptr<Orientation>      dir_ptr;

    xpath_result = scene_file->eval_xpath("//scene_setup/reference");
    if (xpath_result)
    {
      // there should be only one result:
      if (xpath_result->size() != 1)
      {
        SSR_ERROR("More than one reference found in scene setup! Aborting.");
        return false;
      }
      pos_ptr = internal::get_position   (xpath_result->node());
      dir_ptr = internal::get_orientation(xpath_result->node());
    }
    else
    {
      SSR_VERBOSE("No reference point given in XML file. "
          "Using standard (= origin).");
    }
    if (!pos_ptr) pos_ptr.reset(new internal::PositionPlusBool());
    if (!dir_ptr) dir_ptr.reset(new Orientation(90));

    _publish(&api::SceneControlEvents::reference_position, *pos_ptr);
    _publish(&api::SceneControlEvents::reference_rotation, *dir_ptr);

    // LOAD SOURCES

    xpath_result = scene_file->eval_xpath("//scene_setup/source");
    if (xpath_result)
    {
      for (Node node; (node = xpath_result->node()); ++(*xpath_result))
      {
        std::string name  = node.get_attribute("name");
        std::string id    = node.get_attribute("id");
        std::string properties_file = node.get_attribute("properties-file");

        properties_file = posixpathtools::make_path_relative_to_current_dir(
            properties_file, scene_file_name);

        pos_ptr.reset(); dir_ptr.reset();
        pos_ptr = internal::get_position   (node);
        dir_ptr = internal::get_orientation(node);

        // just for the error message:
        std::string id_str;   if (id != "") id_str = " id: \"" + id + "\"";
        std::string name_str; if (name != "") name_str = " name: \""
          + name + "\"";

        std::string model = internal::get_attribute_of_node(node, "model", "");

        if (model == "")
        {
          SSR_VERBOSE("Source model not defined!" << id_str << name_str
              << " Using default (= point source).");
          model = "point";
        }

        if ((model == "point") && !dir_ptr)
        {
          // orientation is optional for point sources, required for plane waves
          dir_ptr.reset(new Orientation);
        }

        if (!pos_ptr || (!dir_ptr && !_scene.get_auto_rotation()))
        {
          SSR_ERROR("Both position and orientation have to be specified for source"
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

        // NB: If ID is not the empty string and not unique, this will fail:
        _new_source(id, name, model, file_name_or_port_number
            , channel, Pos{pos_ptr->x, pos_ptr->y}
            , *dir_ptr, pos_ptr->fixed
            , apf::math::dB2linear(gain_dB), muted, properties_file);
      }
    }
    else
    {
      SSR_WARNING("No sources found in \"" << scene_file_name << "\"!");
    }
  }
  else // file_extension != "asd" -> try to open file as audio file
  {
    SSR_WARNING("Trying to open specified file as audio file.");
    if (!_create_spontaneous_scene(scene_file_name))
    {
      SSR_ERROR("\"" << scene_file_name << "\" could not be loaded as audio file!");
      return false;
    }
  }
  _transport_locate_frames(0);  // go to beginning of audio files
  return true;
}

template<typename Renderer>
bool
Controller<Renderer>::_create_spontaneous_scene(
    const std::string& audio_file_name)
{
  assert(!_conf.follow);

#ifndef ENABLE_ECASOUND
  SSR_ERROR("Couldn't create scene from file \"" << audio_file_name
        << "\"! Ecasound was disabled at compile time.");
  return false;
#else
  size_t no_of_audio_channels;
  AudioPlayer::Soundfile::get_format(audio_file_name, no_of_audio_channels);

  if (no_of_audio_channels == 0)
  {
    SSR_WARNING("No audio channels found in file \"" << audio_file_name << "\"!");
    return false;
  }

  SSR_WARNING("Creating spontaneous scene from the audio file \""
      << audio_file_name << "\".");

  if (_renderer.params.get("name", "") == "brs")
  {
    SSR_WARNING("I don't have information on the BRIRs. I'll use default HRIRs. "
            "Everything will sound in front.");
  }

  // extract pure file name
  const std::string source_name
    = audio_file_name.substr(audio_file_name.rfind('/') + 1);

  // set master volume
  _publish(&api::SceneControlEvents::master_volume, apf::math::dB2linear(0.0f));
  _publish(&api::SceneControlEvents::decay_exponent,
      // throws on error!
      _conf.renderer_params.get<float>("decay_exponent"));
  _publish(&api::SceneControlEvents::amplitude_reference_distance,
      // throws on error!
      _conf.renderer_params.get<float>("amplitude_reference_distance"));
  // set reference
  _publish(&api::SceneControlEvents::reference_position, Pos{});
  _publish(&api::SceneControlEvents::reference_rotation, Rot{});

  const float default_source_distance = 2.5f; // for mono and stereo files

  switch (no_of_audio_channels)
  {
    case 1: // mono file
      {
        Pos pos{0, default_source_distance};
        SSR_VERBOSE("Creating point source at x = " << pos.x
            << " mtrs, y = " << pos.y << " mtrs.");
        assert(pos.z == 0);
        _new_source("", source_name, "point", audio_file_name, 1
            , pos, Rot{}, false, 1.0f, false, "");
      }
      break;

    case 2: // stereo file
      {
        constexpr auto pi = apf::math::pi<float>();

        const float pos_x = default_source_distance * std::cos(pi/3.0f);
        const float pos_y = default_source_distance * std::sin(pi/3.0f);

        Pos pos{-pos_x, pos_y};
        SSR_VERBOSE("Creating plane wave at x = " << pos.x
            << " mtrs, y = " << pos.y << " mtrs.");
        assert(pos.z == 0);

        _new_source("", source_name + " left", "plane"
            , audio_file_name, 1, pos, Orientation(-60)
            , false, 1.0f, false, "");

        pos = Pos{pos_x, pos_y};
        SSR_VERBOSE("Creating plane wave at x = " << pos.x
            << " mtrs, y = " << pos.y << " mtrs.");
        assert(pos.z == 0);

        _new_source("", source_name + " right", "plane"
            , audio_file_name, 2, pos, Orientation(-120)
            , false, 1.0f, false, "");
      }
      break;

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

        Pos pos{pos_x, pos_y};

        SSR_VERBOSE("Creating point source at x = " << pos.x
            << " mtrs, y = " << pos.y << " mtrs.");
        assert(pos.z == 0);

        _new_source("", source_name + " " + apf::str::A2S(n + 1)
            , "point", audio_file_name, n + 1, pos, Rot{}
            , false, 1.0f, false, "");
      }
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

  _gui.reset(new QGUI(*this, _legacy_scene, _argc, _argv,
    path_to_gui_images, path_to_scene_menu));

  // check if anti-aliasing is possible
  if (!_gui->format().sampleBuffers())
  {
    SSR_WARNING("This system does not provide sample buffer support.\n"
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
    SSR_ERROR("The SSR was compiled without InterSense tracker support!");
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
    SSR_ERROR("The SSR was compiled without Polhemus tracker support!");
    (void)ports;  // avoid "unused parameter" warning
    return;
#endif
  }
  else if (type == "vrpn")
   {
 #if defined(ENABLE_VRPN)
     _tracker = TrackerVrpn::create(*this, ports);
 #else
     SSR_ERROR("The SSR was compiled without VRPN tracker support!");
     (void)ports;  // avoid "unused parameter" warning
     return;
 #endif
   }
  else if (type == "razor")
  {
#if defined(ENABLE_RAZOR)
    _tracker = TrackerRazor::create(*this, ports);
#else
    SSR_ERROR("The SSR was compiled without Razor AHRS tracker support!");
    (void)ports;  // avoid "unused parameter" warning
    return;
#endif
  }
  else
  {
    SSR_ERROR("Unknown tracker type \"" << type << "\"!");
    return;
  }

  if (!_tracker)
  {
    SSR_WARNING("Cannot find tracker. "
            "Make sure that you have the appropriate access rights "
            "to read from the port. I continue without tracker.");
  }
}

/// This is temporary!!!!
template<typename Renderer>
void
Controller<Renderer>::_calibrate_client()
{
#if defined(ENABLE_INTERSENSE) || defined(ENABLE_POLHEMUS) || defined(ENABLE_VRPN) || defined(ENABLE_RAZOR)
  if (_tracker)
  {
    _tracker->calibrate();
  }
  else
  {
    SSR_WARNING("No tracker there to calibrate.");
  }
#endif
}


template<typename Renderer>
void
Controller<Renderer>::_transport_locate_frames(uint32_t time)
{
  _renderer.transport_locate(time);
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

/** Create a new source.
 * @param model Source model
 * @param file_name_or_port_number File name or port number (as string)
 * @param channel Channel of soundfile. If 0, a JACK portname is expected.
 **/
template<typename Renderer>
void
Controller<Renderer>::_new_source(id_t requested_id, const std::string& name
      , const std::string& model, const std::string& file_name_or_port_number
      , int channel, const Pos& position, const Rot& rotation, bool fixed
      , float volume, bool mute, const std::string& properties_file)
{

  // TODO: similar function for follower? just using a JACK port, no audio file

  assert(!_conf.follow);

  std::string id = requested_id;

  if (id != "" && !std::regex_match(id, _re_ncname))
  {
    SSR_ERROR("Invalid source ID: " << id);
    return;
  }

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
    SSR_ERROR("Couldn't open audio file \"" << file_name_or_port_number
        << "\"! Ecasound was disabled at compile time.");
    return;
#endif
  }
  else  // no audio file
  {
    assert(channel == 0);

    if (file_name_or_port_number != "")
    {
      port_name = _conf.input_port_prefix + file_name_or_port_number;
    }
  }

  if (port_name == "")
  {
    SSR_VERBOSE("No audio file or port specified for source");
  }

  apf::parameter_map p;
  p.set("connect-to", port_name);
  p.set("properties-file", properties_file);
  try
  {
    id = _renderer.add_source(id, p);
  }
  catch (std::exception& e)
  {
    SSR_ERROR(e.what());
    return;
  }
  assert(requested_id.size() == 0 || requested_id == id);

  _publish(&api::SceneInformationEvents::new_source, id);
  _publish(&api::SceneInformationEvents::source_property
      , id, "port-name", port_name);

  if (file_name_or_port_number != "")
  {
    _publish(&api::SceneInformationEvents::source_property
        , id, "audio-file", file_name_or_port_number);
    _publish(&api::SceneInformationEvents::source_property
        , id, "audio-file-channel", apf::str::A2S(channel));
    _publish(&api::SceneInformationEvents::source_property
        , id, "audio-file-length", apf::str::A2S(file_length));
  }
  _publish(&api::SceneInformationEvents::source_property
      , id, "properties-file", properties_file);

  _publish(&api::SceneControlEvents::source_name, id, name);
  _publish(&api::SceneControlEvents::source_model, id, model);
  _publish(&api::SceneControlEvents::source_position, id, position);
  _publish(&api::SceneControlEvents::source_rotation, id, rotation);
  _publish(&api::SceneControlEvents::source_fixed, id, fixed);
  _publish(&api::SceneControlEvents::source_volume, id, volume);
  _publish(&api::SceneControlEvents::source_mute, id, mute);
}

template<typename Renderer>
void
Controller<Renderer>::_delete_all_sources()
{
  assert(!_conf.follow);

  std::string id;
  while (!(id = _scene.get_source_id(1)).empty())
  {
    _publish(&api::SceneControlEvents::delete_source, id);
    // NB: This assumes that _scene is subscribed!
  }
}

template<typename Renderer>
void
Controller<Renderer>::_orient_source_toward_reference(id_t id)
{
  assert(!_conf.follow);

  auto* src = _scene.get_source(id);
  if (src)
  {
    // NB: Reference offset is not taken into account
    //     (because it is a property of the renderer, not the scene)
    auto ref_pos = _scene.get_reference_position();
    _publish(&api::SceneControlEvents::source_rotation
        , id, look_at(src->position, ref_pos));
  }
  else
  {
    SSR_WARNING("Auto-rotation: Source \"" << id << "\" doesn't exist");
  }
}

template<typename Renderer>
void
Controller<Renderer>::_orient_all_sources_toward_reference()
{
  _scene.for_each_source([this](auto id, const auto& source){
    if (!source.fixed)
    {
      _orient_source_toward_reference(id);
    }
  });
}

#ifdef ENABLE_IP_INTERFACE
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
#endif

template<typename Renderer>
bool
Controller<Renderer>::_save_scene(const std::string& filename) const
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
  float volume = _legacy_scene.get_master_volume();
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
  DirectionalPoint reference = _legacy_scene.get_reference();
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
  LegacyLoudspeaker::container_t loudspeakers;
  _legacy_scene.get_loudspeakers(loudspeakers, false); // get relative positions
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
  _legacy_scene.for_each_source([&](
        unsigned int id, const LegacySource& source) {
    Node source_node = node.new_child("source");
    if (scene_file_name != "")
    {
      // ignore "id" -> ugly quick hack!
    }
    else
    {
      source_node.new_attribute("id", apf::str::A2S(id));
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

    if (source.properties_file != "")
    {
      source_node.new_attribute("properties-file"
          , posixpathtools::make_path_relative_to_file(source.properties_file
            , scene_file_name));
    }
  });
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
