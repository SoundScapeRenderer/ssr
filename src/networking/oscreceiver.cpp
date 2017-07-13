/**
 * Implementation of oscreceiver.h
 * @file oscreceiver.cpp
 */

#include "oscreceiver.h"
#include "oschandler.h"
#include "publisher.h"
#include "apf/stringtools.h"
#include "apf/math.h"

using namespace apf::str;

/**
 * Constructor
 * @param controller reference to a Publisher object
 * @param port_in integer representing a port to used for incoming OSC messages
 */
ssr::OscReceiver::OscReceiver(Publisher& controller, OscHandler& handler)
  : _controller(controller)
  , _handler(handler)
{
  VERBOSE("OscReceiver: Initialized.");
}

/**
 * Destructor
 */
ssr::OscReceiver::~OscReceiver()
{
  VERBOSE("OscReceiver: Destructing.");
}

/**
 * Starts the OscReceiver, by adding client/ server callback functions.
 */
void ssr::OscReceiver::start()
{
  VERBOSE("OscReceiver: Starting.");
  // add method handlers for received messages
  if (_handler.is_server())
  {
    add_client_to_server_methods();
    add_processing_methods();
    add_reference_methods();
    add_scene_methods();
    add_source_methods();
    add_tracker_methods();
    add_transport_methods();
    add_update_notification_methods();
  }
  else if (_handler.is_client())
  {
    add_server_to_client_methods();
    add_processing_methods();
    add_reference_methods();
    add_scene_methods();
    add_source_methods();
    add_tracker_methods();
    add_transport_methods();
  }
}

/**
 * Stops the OscReceiver
 */
void ssr::OscReceiver::stop()
{
  VERBOSE("OscReceiver: Stopping.");
}

/**
 * Adds callback handlers (for server) for alive, subscribe and message level
 * messages received from clients.
 * This function uses C++11 lambda functions to define the behavior for every
 * callback.
 */
void ssr::OscReceiver::add_client_to_server_methods()
{
  // incrementing alive_counter of subscribed client: "/alive"
  _handler.server().add_method("/alive", NULL, [this](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/alive] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      increment_client_alive_counter(_handler, message.source().hostname(),
          message.source().port());
    }
  );
  VERBOSE("OscReceiver: Added callback for /alive.");

  // setting MessageLevel of subscribed client: "/message_level, {i,ssi}"
  _handler.server().add_method("/message_level", NULL, [this](lo_arg **argv,
        int, lo::Message message)
    {
      (void) argv;
      if(!message.types().compare("i"))
      {
        VERBOSE2("OscReceiver: Got [/message_level, " << argv[0]->i <<
            "] from client '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        set_client_message_level(_handler, message.source().hostname(),
            message.source().port(), get_sane_message_level(argv[0]->i));
      }
      else if(!message.types().compare("ssi"))
      {
        std::string hostname(&argv[0]->s);
        std::string port(&argv[1]->s);
        VERBOSE2("OscReceiver: Got [/message_level, " << hostname << ", " <<
            port << ", " << argv[2]->i << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        set_client_message_level(_handler, hostname, port,
            get_sane_message_level(argv[2]->i));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /message_level {i,ssi}.");

  // subscribing and unsubscribing clients
  _handler.server().add_method("/subscribe", NULL, [this](lo_arg **argv, int,
        lo::Message message)
    {
      if(!message.types().compare("T"))
      {
      // subscribing client: "/subscribe, T"
        VERBOSE2("OscReceiver: Got [/subscribe, " <<
            _handler.bool_to_string(true) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        add_client(_handler, message.source().hostname(),
            message.source().port(), ssr::MessageLevel::CLIENT);
      }
      // unsubscribing client: "/subscribe, F"
      else if(!message.types().compare("F"))
      {
        VERBOSE2("OscReceiver: Got [/subscribe, " <<
            _handler.bool_to_string(false) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        deactivate_client(_handler, message.source().hostname(),
            message.source().port());
      }
      // unsubscribing client: "/subscribe, Fss, hostname, port"
      else if(!message.types().compare("Fss"))
      {
        std::string hostname(&argv[1]->s);
        std::string port(&argv[2]->s);
        VERBOSE2("OscReceiver: Got [/subscribe, " <<
            _handler.bool_to_string(false) << ", " << hostname << ", " << port
            <<  "] from client '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        deactivate_client(_handler, hostname, port);
      }
      // subscribing client: "/subscribe, Tssi, hostname, port, message_level"
      else if(!message.types().compare("Tssi"))
      {
        std::string hostname(&argv[1]->s);
        std::string port(&argv[2]->s);
        VERBOSE2("OscReceiver: Got [/subscribe, " <<
            _handler.bool_to_string(true) << ", " << hostname << ", " << port
            << ", " << argv[3]->i << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        add_client(_handler, hostname, port,
            get_sane_message_level(argv[3]->i));
      }
      // subscribing client: "/subscribe, Ti, message_level"
      else if(!message.types().compare("Ti"))
      {
        VERBOSE2("OscReceiver: Got [/subscribe, " <<
            _handler.bool_to_string(true) << ", " << argv[1]->i <<
            "] from client '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        add_client(_handler, message.source().hostname(),
            message.source().port(), get_sane_message_level(argv[1]->i));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /subscribe {F,Fss,T,Ti,Tssi}.");
}

/**
 * Adds callback handlers (for server) for update messages received from
 * clients.
 * This function uses C++11 lambda functions to define the behavior for every
 * callback.
 */
void ssr::OscReceiver::add_update_notification_methods()
{
  // update on cpu_load: "/update/cpu_load, f, load"
  _handler.server().add_method("/update/cpu_load", "f", [](lo_arg **argv, int,
        lo::Message message)
    {
      VERBOSE3("OscReceiver: Got [/update/cpu_load, " << argv[0]->f <<
          "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/cpu_load f.");

  // update on state processing: "/update/processing/state, {F,T},
  // false|true"
  _handler.server().add_method("/update/processing/state", NULL, [this](lo_arg
        **argv, int, lo::Message message)
    {
      bool state;
      (void) argv;
      if(!message.types().compare("T"))
      {
        state = true;
      }
      else if(!message.types().compare("F"))
      {
        state = false;
      }
      VERBOSE3("OscReceiver: Got [/update/processing/state, " <<
          _handler.bool_to_string(state) << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/processing/state {F,T}.");

  // update on reference orientation: "/update/reference/orientation, f,
  // azimuth"
  _handler.server().add_method("/update/reference/orientation", "f", [](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/reference/orientation, " <<
          argv[0]->f << "] from client '" << message.source().hostname() << ":"
          << message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/reference/orientation f.");

  // update on reference position: "/update/reference/position, ff, x, y"
  _handler.server().add_method("/update/reference/position", "ff", [](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/reference/position, " << argv[0]->f
          << ", " << argv[1]->f << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/reference/position ff.");

  // update on reference offset orientation:
  // "/update/reference_offset/orientation, f, azimuth"
  _handler.server().add_method("/update/reference_offset/orientation", "f",
      [](lo_arg **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/reference_offset/orientation, " <<
          argv[0]->f << "] from client '" << message.source().hostname() << ":"
          <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for \
/update/reference_offset/orientation f.");

  // update on reference offset position: "/update/reference_offset/position,
  // ff, x, y"
  _handler.server().add_method("/update/reference_offset/position", "ff",
      [](lo_arg **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/reference_offset/position, " <<
          argv[0]->f << ", " << argv[1]->f << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/reference_offset/position \
ff.");

  // update on scene amplitude reference distance:
  // "update/scene/amplitude_reference_distance, f,
  // amplitude_reference_distance"
  _handler.server().add_method("/update/scene/amplitude_reference_distance",
      "f", [](lo_arg **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/scene/amplitude_reference_distance, "
          << argv[0]->f << "] from client '" << message.source().hostname() <<
          ":" << message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for \
/update/scene/amplitude_reference_distance f.");

  // update on scene source auto rotation: "/update/scene/auto_rotate_sources,
  // {F,T}, false|true"
  _handler.server().add_method("/update/scene/auto_rotate_sources", NULL,
      [this](lo_arg **argv, int, lo::Message message)
    {
      bool state;
      (void) argv;
      if(!message.types().compare("T"))
      {
        state = true;
      }
      else if(!message.types().compare("F"))
      {
        state = false;
      }
      VERBOSE2("OscReceiver: Got [/update/scene/auto_rotate_sources, " <<
          _handler.bool_to_string(state) << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/scene/auto_rotate_sources \
{F,T}.");

  // update on scene decay exponent: "/update/scene/decay_exponent, f,
  // decay_exponent"
  _handler.server().add_method("/update/scene/decay_exponent", "f", [](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/scene/decay_exponent, " << argv[0]->f
          << "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/scene/decay_exponent f.");

  // update on scene master signal level: "/update/scene/master_signal_level,
  // f, master_signal_level"
  _handler.server().add_method("/update/scene/master_signal_level", "f",
      [](lo_arg **argv, int, lo::Message message)
    {
      VERBOSE3("OscReceiver: Got [/update/scene/master_signal_level, " <<
          argv[0]->f << "] from client '" << message.source().hostname() << ":"
          << message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/scene/master_signal_level \
f.");

  // update on scene sample rate: "/update/scene/sample_rate, i, sample_rate"
  _handler.server().add_method("/update/scene/sample_rate", "i", [](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/scene/sample_rate, " << argv[0]->i <<
          "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/scene/sample_rate i.");

  // update on scene volume: "/update/scene/volume, f, volume"
  _handler.server().add_method("/update/scene/volume", "f", [](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/scene/volume, " << argv[0]->f <<
          "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/scene/volume f.");

  // update on deleted source: "/update/source/delete, i, id"
  _handler.server().add_method("/update/source/delete", "i", [](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/source/delete, " << argv[0]->i
          << "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/delete i.");

  // update on source file_channel: "/update/source/file_channel, ii, id,
  // file_channel"
  _handler.server().add_method("/update/source/file_channel", "ii", [](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/source/file_channel, " << argv[0]->i
          << ", " << argv[1]->i << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/file_channel ii.");

  // update on source file_name_or_port_number:
  // "/update/source/file_name_or_port_number, is, id, file_name_or_port_number"
  _handler.server().add_method("/update/source/file_name_or_port_number", "is",
      [](lo_arg **argv, int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/update/source/file_name_or_port_number, " <<
          argv[0]->i << ", " << name << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for \
/update/source/file_name_or_port_number is.");

  // update on source gain: "/update/source/gain, if, id, gain"
  _handler.server().add_method("/update/source/gain", "if", [](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/source/gain, " << argv[0]->i
          << ", " << argv[1]->f << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/gain if.");

  // update on source file length: "/update/source/length, ii, id, length"
  _handler.server().add_method("/update/source/length", "ii", [](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/source/length, " << argv[0]->i
          << ", " << argv[1]->i << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/length ii.");

  // update on source signal level: "/update/source/level, if, id, level"
  _handler.server().add_method("/update/source/level", "if", [](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE3("OscReceiver: Got [/update/source/level, " << argv[0]->i <<
          ", "<< argv[1]->f << "] from client '" << message.source().hostname()
          << ":" << message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/level if.");

  // update on source model: "/update/source/model, is, id, model"
  _handler.server().add_method("/update/source/model", "is", [](lo_arg **argv,
        int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/update/source/model, " << argv[0]->i << ", "
          << name << "] from client '" << message.source().hostname() << ":" <<
          message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/model is.");

  // update on source mute: "/update/source/mute, i{F,T}, id, false|true"
  _handler.server().add_method("/update/source/mute", NULL, [this](lo_arg
        **argv, int, lo::Message message)
    {
      if(!message.types().compare("iT"))
      {
        VERBOSE2("OscReceiver: Got [/update/source/mute, " << argv[0]->i <<
            ", " << _handler.bool_to_string(true) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
      }
      else if(!message.types().compare("iF"))
      {
        VERBOSE2("OscReceiver: Got [/update/source/mute, " << argv[0]->i <<
            ", " << _handler.bool_to_string(false) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/mute i{F,T}.");

  // update on source name: "/update/source/name, is, id, name"
  _handler.server().add_method("/update/source/name", "is", [](lo_arg **argv,
        int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/update/source/name, " << argv[0]->i << ", "
          << name << "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/name is.");

  // update on source orientation: "/update/source/orientation, if, id,
  // azimuth"
  _handler.server().add_method("/update/source/orientation", "if", [](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/source/orientation, " << argv[0]->i
          << ", " << argv[1]->f << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/orientation if.");

  // update on new source: "/update/source/new, i, id"
  _handler.server().add_method("/update/source/new", "i", [](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/source/new, " << argv[0]->i
          << "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/new i.");

  // update on source port_name: "/update/source/port_name, is, id, port_name"
  _handler.server().add_method("/update/source/port_name", "is", [](lo_arg
        **argv, int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/update/source/port_name, " <<
          argv[0]->i << ", " << name << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/port_name is.");

  // update on source position: "/update/source/position, iff, id, x, y"
  _handler.server().add_method("/update/source/position", "iff", [](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/update/source/position, " << argv[0]->i <<
          ", " << argv[1]->f << ", " << argv[2]->f << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/position iff.");

  // update on source position fixation: "/update/source/position_fixed,
  // i{F,T}, id, false|true"
  _handler.server().add_method("/update/source/position_fixed", NULL,
      [this](lo_arg **argv, int, lo::Message message)
    {
      if(!message.types().compare("iT"))
      {
        VERBOSE2("OscReceiver: Got [/update/source/position_fixed, " <<
            argv[0]->i << ", " << _handler.bool_to_string(true) <<
            "] from client '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
      }
      else if(!message.types().compare("iF"))
      {
        VERBOSE2("OscReceiver: Got [/update/source/position_fixed, " <<
            argv[0]->i << ", " << _handler.bool_to_string(false) <<
            "] from client '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/position_fixed \
i{F,T}.");

  // update on source properties_file: "/update/source/properties_file, is, id,
  // properties_file"
  _handler.server().add_method("/update/source/properties_file", "is",
      [](lo_arg **argv, int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/update/source/properties_file, " <<
          argv[0]->i << ", " << name << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/source/properties_file is.");

  // update on transport seek: "/update/transport/seek, s, time"
  _handler.server().add_method("/update/transport/seek", "s", [](lo_arg **argv,
        int, lo::Message message)
    {
      std::string seek = &argv[0]->s;
      VERBOSE3("OscReceiver: Got [/update/transport/seek, " << seek <<
          "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/transport/seek s.");

  // update on transport state: "/update/transport/state, {F,T}, false|true"
  _handler.server().add_method("/update/transport/state", NULL, [this](lo_arg
        **argv, int, lo::Message message)
    {
      bool state;
      (void) argv;
      if(!message.types().compare("T"))
      {
        state = true;
      }
      else if(!message.types().compare("F"))
      {
        state = false;
      }
      VERBOSE3("OscReceiver: Got [/update/transport/state, " <<
          _handler.bool_to_string(state) << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /update/transport/state {F,T}.");
}

/**
 * Adds callback handlers (for clients) for /poll and /message_level messages
 * received from a server.
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_server_to_client_methods()
{
  // set cpu_load: "/cpu_load, f, load"
  _handler.server().add_method("/cpu_load", "f", [](lo_arg **argv, int,
        lo::Message message)
    {
      VERBOSE3("OscReceiver: Got [/cpu_load, " << argv[0]->f <<
          "] from server '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /cpu_load f.");

  // set OscSender's _server _message_level: /message_level, i
  _handler.server().add_method("/message_level", "i", [this](lo_arg **argv,
        int, lo::Message message)
    {
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      VERBOSE2("OscReceiver: Got [/message_level, " << argv[0]->i <<
          "] from server '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      if(is_server(_handler, hostname, port))
        set_server_message_level(_handler, get_sane_message_level(argv[0]->i));
    }
  );
  VERBOSE("OscReceiver: Added callback for /message_level i.");

  // set _server_address for OscSender through OscHandler, depending on, if
  // polled from given server before
  _handler.server().add_method("/poll", NULL, [this](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address from(message.source());
      std::string hostname(from.hostname());
      std::string port(from.port());
      (void) argv;
      if(!is_server(_handler, hostname, port))
      {
        VERBOSE2("OscReceiver: Got [/poll] from server " << from.hostname() <<
            ":" << from.port() << ". Subscribing...");
        set_server_address(_handler, hostname, port);
        from.send_from(_handler.server(), "/subscribe", "T");
      }
      else
      {
        VERBOSE2("OscReceiver: Got [/poll] from server " << from.hostname() <<
            ":" << from.port() << ".");
        from.send_from(_handler.server(), "/alive", "");
        VERBOSE2("OscReceiver: Sent [/alive] to server " << from.hostname() <<
            ":" << from.port() << ".");
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /poll.");
}

/**
 * Adds callback handlers (for clients and server) for source related messages
 * received from a server or a client with MessageLevel::SERVER (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_source_methods()
{
  // delete source: "/source/delete, i, id"
  // special case: i == 0 deletes all sources!
  _handler.server().add_method("/source/delete", "i", [this](lo_arg **argv,
        int, lo::Message message)
    {
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      VERBOSE2("OscReceiver: Got [/source/delete, " << argv[0]->i <<
          "] from " << _handler.from_is()  << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i == 0)
        {
          _controller.delete_all_sources();
        }
        else if (argv[0]->i > 0)
        {
          _controller.delete_source(argv[0]->i);
        }
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/delete i.");

  // set source file_channel: "/source/file_channel, ii, id, file_channel"
  _handler.server().add_method("/source/file_channel", "ii", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/source/file_channel, " << argv[0]->i << ", "
          << argv[1]->i << "] from " << _handler.from_is()  << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_file_channel(argv[0]->i, argv[1]->i);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/file_channel ii.");

  // set source file_name_or_port_number: "/source/file_name_or_port_number,
  // is, id, file_name_or_port_number"
  _handler.server().add_method("/source/file_name_or_port_number", "is",
      [this](lo_arg **argv, int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/source/file_name_or_port_number, " <<
          argv[0]->i << ", " << name << "] from " << _handler.from_is()  <<
          " '" << message.source().hostname() << ":" << message.source().port()
          << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_file_name(argv[0]->i, name);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/file_name_or_port_number "
      "is.");

  // set source gain: "/source/gain, if, id, gain"
  _handler.server().add_method("/source/gain", "if", [this](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/source/gain, " << argv[0]->i << ", " <<
          argv[1]->f << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_gain(argv[0]->i,
              apf::math::dB2linear(argv[1]->f));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/gain if.");

  // set source signal level (GUI_CLIENT only): "/source/level, if, id, level"
  _handler.server().add_method("/source/level", "if", [this](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE3("OscReceiver: Got [/source/level, " << argv[0]->i << ", "<<
          argv[1]->f << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/level if.");

  // set source model: "/source/model, is, id, model"
  _handler.server().add_method("/source/model", "is", [this](lo_arg **argv,
        int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      Source::model_t model = Source::model_t();
      if (!apf::str::S2A(name, model))
      {
        model = Source::point;
      }
      VERBOSE2("OscReceiver: Got [/source/model, " << argv[0]->i << ", " <<
          name << "] from " << _handler.from_is()  << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_model(argv[0]->i, model);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/model is.");

  // set source mute: "/source/mute, i{F,T}, id, true|false"
  _handler.server().add_method("/source/mute", NULL, [this](lo_arg **argv, int,
        lo::Message message)
    {
      if(!message.types().compare("iT"))
      {
        VERBOSE2("OscReceiver: Got [/source/mute, " << argv[0]->i <<
            ", true] from " << _handler.from_is() << " '"<<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          if (argv[0]->i > 0)
            _controller.set_source_mute(argv[0]->i, true);
        }
      }
      else if(!message.types().compare("iF"))
      {
        VERBOSE2("OscReceiver: Got [/source/mute, " << argv[0]->i <<
            ", false] from " << _handler.from_is() << " '"<<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          if (argv[0]->i > 0)
            _controller.set_source_mute(argv[0]->i, false);
        }
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/mute i{F,T}.");

  // set source name: "/source/name, is, id, name"
  _handler.server().add_method("/source/name", "is", [this](lo_arg **argv, int,
        lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/source/name, " << argv[0]->i << ", " << name
          << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_name(argv[0]->i, name);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/name is.");

  // create new source: "/source/new, sssffff{F,T}{F,T}{F,T}, name, model,
  // file_name_or_port_number, x, y, orientation, gain, position_fixed,
  // orientation_fixed, muted"
  // create new source: "/source/new, sssffffis{F,T}{F,T}{F,T}, name, model,
  // file_name_or_port_number, x, y, orientation, gain, file_channel,
  // properties_file, position_fixed, orientation_fixed, muted"
  _handler.server().add_method("/source/new", NULL, [this](lo_arg **argv, int,
        lo::Message message)
    {
      int file_channel = 0;
      std::string properties_file = "";
      std::string channel_and_properties = "";
      bool position_fixed;
      bool orientation_fixed;
      bool muted;
      bool setup = false;
      if (!message.types().compare("sssffffTTT"))
      {
        position_fixed = true;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (!message.types().compare("sssffffTTF"))
      {
        position_fixed = true;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (!message.types().compare("sssffffTFF"))
      {
        position_fixed = true;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (!message.types().compare("sssffffFFF"))
      {
        position_fixed = false;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (!message.types().compare("sssffffTFT"))
      {
        position_fixed = true;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (!message.types().compare("sssffffFTF"))
      {
        position_fixed = false;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (!message.types().compare("sssffffFTT"))
      {
        position_fixed = false;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (!message.types().compare("sssffffFFT"))
      {
        position_fixed = false;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (!message.types().compare("sssffffisTTT"))
      {
        file_channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (file_channel+", "+properties_file);
        position_fixed = true;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (!message.types().compare("sssffffisTTF"))
      {
        file_channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (file_channel+", "+properties_file);
        position_fixed = true;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (!message.types().compare("sssffffisTFF"))
      {
        file_channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (file_channel+", "+properties_file);
        position_fixed = true;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (!message.types().compare("sssffffisFFF"))
      {
        file_channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (file_channel+", "+properties_file);
        position_fixed = false;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (!message.types().compare("sssffffisTFT"))
      {
        file_channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (file_channel+", "+properties_file);
        position_fixed = true;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (!message.types().compare("sssffffisFTF"))
      {
        file_channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (file_channel+", "+properties_file);
        position_fixed = false;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (!message.types().compare("sssffffisFTT"))
      {
        file_channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (file_channel+", "+properties_file);
        position_fixed = false;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (!message.types().compare("sssffffisFFT"))
      {
        file_channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (file_channel+", "+properties_file);
        position_fixed = false;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (setup)
      {
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          std::string name(&(argv[0]->s));
          std::string file_name_or_port_number(&(argv[2]->s));
          float x(argv[3]->f);
          float y(argv[4]->f);
          float gain(apf::math::dB2linear(argv[6]->f));
          Source::model_t model = Source::model_t();
          if (!apf::str::S2A(apf::str::A2S(argv[1]->s), model))
          {
            model = Source::point;
          }
          Position position(x, y);
          Orientation orientation(argv[5]->f);
          VERBOSE2("OscReceiver: Got [/source/new, " << name << ", " << model
              << ", " << file_name_or_port_number << ", " << x << ", " << y <<
              ", " << orientation.azimuth << ", " << gain <<
              channel_and_properties << ", " <<
              _handler.bool_to_string(position_fixed) << ", " <<
              _handler.bool_to_string(orientation_fixed) << ", " <<
              _handler.bool_to_string(muted) <<  "] from " <<
              _handler.from_is() << " '" << message.source().hostname() << ":"
              << message.source().port() << "'.");
          _controller.new_source(name, model, file_name_or_port_number,
              file_channel, position, position_fixed, orientation,
              orientation_fixed, gain, muted, properties_file);
          VERBOSE2("OscReceiver: Created source with following properties:"
              "\nname: " << name <<
              "\nmodel: " << model <<
              "\nfile_name_or_port_number: " << file_name_or_port_number <<
              "\nfile_channel: " << file_channel <<
              "\nposition: " << position <<
              "\nposition_fixed: " << position_fixed <<
              "\norientation: " << orientation <<
              "\norientation_fixed: " << orientation_fixed <<
              "\ngain (linear): " << gain <<
              "\nmuted: " << muted <<
              "\nproperties_file: " << properties_file);
        }
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/new \
{sssffff,sssffffis}{F,T}{F,T}{F,T}.");

  // set source orientation: "/source/orientation, if, id, azimuth"
  _handler.server().add_method("/source/orientation", "if", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/source/orientation, " << argv[0]->i << ", "
          << argv[1]->f << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_orientation(argv[0]->i,
              Orientation(argv[1]->f));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/orientation if.");

  // set source port name: "/source/port_name, is, id, port_name"
  _handler.server().add_method("/source/port_name", "is", [this](lo_arg **argv,
        int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/source/port_name, " << argv[0]->i << ", " <<
          name << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_port_name(argv[0]->i, name);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/port_name is.");

  // set source position: "/source/position, iff, id, x, y"
  _handler.server().add_method("/source/position", "iff", [this](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/source/position, " << argv[0]->i << ", " <<
        argv[1]->f << ", " <<  argv[2]->f << "] from " << _handler.from_is() <<
        " '" << message.source().hostname() << ":" << message.source().port()
        << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_position(argv[0]->i, Position(argv[1]->f,
                argv[2]->f));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/position iff.");

  // set source fixed: "/source/position_fixed, i{F,T}, id, true|false"
  _handler.server().add_method("/source/position_fixed", NULL, [this](lo_arg
        **argv, int, lo::Message message)
    {
      if (!message.types().compare("iT"))
      {
        VERBOSE2("OscReceiver: Got [/source/position_fixed, " << argv[0]->i <<
            ", true] from " << _handler.from_is() << " '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          if (argv[0]->i > 0)
            _controller.set_source_position_fixed(argv[0]->i, true);
        }
      }
      else if (!message.types().compare("iF"))
      {
        VERBOSE2("OscReceiver: Got [/source/position_fixed, " << argv[0]->i <<
            ", false] from " << _handler.from_is() << " '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          if (argv[0]->i > 0)
            _controller.set_source_position_fixed(argv[0]->i, false);
        }
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/position_fixed i{F,T}.");

  // set source file: "/source/properties_file, is, id, properties_file"
  _handler.server().add_method("/source/properties_file", "is", [this](lo_arg
        **argv, int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/source/properties_file, " << argv[0]->i <<
          ", " << name << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        if (argv[0]->i > 0)
          _controller.set_source_properties_file(argv[0]->i, name);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /source/properties_file is.");

}

/**
 * Adds callback handlers (for clients and server) for reference related
 * messages received from a server or a client with MessageLevel::SERVER
 * (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_reference_methods()
{
  // set reference orientation: "/reference/orientation, f, azimuth"
  _handler.server().add_method("/reference/orientation", "f", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/reference/orientation, " << argv[0]->f <<
          "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.set_reference_orientation(Orientation(argv[0]->f));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /reference/orientation f.");

  // set reference position: "/reference/position, ff, x, y"
  _handler.server().add_method("/reference/position", "ff", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/reference/position, " << argv[0]->f << ", "
          << argv[1]->f << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" <<  message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.set_reference_position(Position(argv[0]->f, argv[1]->f));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /reference/position ff.");

  // set reference offset orientation: "/reference_offset/orientation, f,
  // azimuth"
  _handler.server().add_method("/reference_offset/orientation", "f" ,
      [this](lo_arg **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/reference_offset/orientation, " <<
          argv[0]->f << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.set_reference_offset_orientation(Orientation(argv[0]->f));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /reference_offset/orientation f.");

  // set reference offset position: "/reference_offset/position, ff, x, y"
  _handler.server().add_method("/reference_offset/position", "ff" ,
      [this](lo_arg **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/reference_offset/position, " << argv[0]->f
          << ", " << argv[1]->f << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.set_reference_offset_position(Position(argv[0]->f,
              argv[1]->f));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /reference_offset/position ff.");
}

/**
 * Adds callback handlers (for clients and server) for scene related messages
 * received from a server or a client with MessageLevel::SERVER (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_scene_methods()
{
  //TODO: add /scene/transfer ss, host, port (_controller.get_scene_as_XML())
  //_add_master_volume(node);
  //_add_transport_state(node);
  //_add_reference(node);
  //_add_loudspeakers(node);
  //_add_sources(node);

  // set scene's amplitude reference distance:
  // "/scene/amplitude_reference_distance, f, amplitude_reference_distance"
  _handler.server().add_method("/scene/amplitude_reference_distance", "f",
      [this](lo_arg **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/scene/amplitude_reference_distance, " <<
          argv[0]->f << "] from client '" << message.source().hostname() <<
          ":" << message.source().port() << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.set_amplitude_reference_distance(argv[0]->f);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for \
/scene/amplitude_reference_distance f.");

  // set scene source auto rotation: "/scene/auto_rotate_sources, {F,T},
  // false|true"
  _handler.server().add_method("/scene/auto_rotate_sources", NULL,
      [this](lo_arg **argv, int, lo::Message message)
    {
      (void) argv;
      if(!message.types().compare("T"))
      {
      VERBOSE2("OscReceiver: Got [/scene/auto_rotate_sources, " <<
          _handler.bool_to_string(true) << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          _controller.set_auto_rotation(true);
        }
      }
      else if(!message.types().compare("F"))
      {
        VERBOSE2("OscReceiver: Got [/scene/auto_rotate_sources, " <<
            _handler.bool_to_string(false) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          _controller.set_auto_rotation(false);
        }
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /scene/auto_rotate_sources {F,T}.");

  // clear scene: "/scene/clear"
  _handler.server().add_method("/scene/clear", NULL , [this](lo_arg **argv,
        int, lo::Message message)
    {
      (void) argv;
      VERBOSE2("OscReceiver: Got [/scene/clear] from " << _handler.from_is() <<
          " '" << message.source().hostname() << ":" << message.source().port()
          << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.delete_all_sources();
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /scene/clear.");

  // set scene decay exponent: "/scene/decay_exponent, f, decay_exponent"
  _handler.server().add_method("/scene/decay_exponent", "f", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/scene/decay_exponent, " << argv[0]->f
          << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.set_decay_exponent(argv[0]->f);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /scene/decay_exponent f.");

  // load scene from file: "/scene/load, s, file"
  _handler.server().add_method("/scene/load", "s" , [this](lo_arg **argv, int,
        lo::Message message)
    {
      std::string name(&argv[0]->s);
      VERBOSE2("OscReceiver: Got [/scene/load, " << name << "] from " <<
          _handler.from_is() << " '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.load_scene(name);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /scene/load s.");

  // save scene to file: "/scene/save, s, file"
  _handler.server().add_method("/scene/save", "s" , [this](lo_arg **argv, int,
        lo::Message message)
    {
      std::string name(&argv[0]->s);
      VERBOSE2("OscReceiver: Got [/scene/save, " << name << "] from " <<
          _handler.from_is() << " '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
      {
        _controller.save_scene_as_XML(name);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /scene/save s.");

  // set scene master signal level: "/scene/master_signal_level, f,
  // master_signal_level"
  _handler.server().add_method("/scene/master_signal_level", "f", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE3("OscReceiver: Got [/scene/master_signal_level, " << argv[0]->f
          << "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
    }
  );
  VERBOSE("OscReceiver: Added callback for /scene/master_signal_level f.");

  // set master volume: "/scene/volume, f, volume"
  _handler.server().add_method("/scene/volume", "f" , [this](lo_arg **argv,
      int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/scene/volume, " << argv[0]->f << "] from "
          << _handler.from_is() << " '" << message.source().hostname() << ":"
          << message.source().port() << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.set_master_volume(apf::math::dB2linear(argv[0]->f));
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /scene/volume f.");
}

/**
 * Adds callback handlers (for clients and server) for processing related
 * messages received from a server or a client with MessageLevel::SERVER
 * (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_processing_methods()
{
  // set processing state: "/processing/state, T, true"
  _handler.server().add_method("/processing/state", NULL, [this](lo_arg **argv,
        int, lo::Message message)
    {
      (void) argv;
      if(!message.types().compare("T"))
      {
        VERBOSE2("OscReceiver: Got [/processing/state, " <<
            _handler.bool_to_string(true) << "] from " << _handler.from_is() <<
            " '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          _controller.start_processing();
        }
      }
      else if(!message.types().compare("F"))
      {
        VERBOSE2("OscReceiver: Got [/processing/state, " <<
            _handler.bool_to_string(false) << "] from " << _handler.from_is()
            << " '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          _controller.stop_processing();
        }
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /processing/state {F,T}.");

}

/**
 * Adds callback handlers (for clients and server) for transport related
 * messages received from a server or a client with MessageLevel::SERVER
 * (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_transport_methods()
{
  // rewind transport state: "/transport/rewind"
  _handler.server().add_method("/transport/rewind", NULL , [this](lo_arg
        **argv, int, lo::Message message)
    {
      (void) argv;
      VERBOSE2("OscReceiver: Got [/transport/rewind] from " <<
          _handler.from_is() << " '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.transport_locate(0);
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /transport/rewind.");

  // seek transport state: "/transport/seek, s, time"
  _handler.server().add_method("/transport/seek", "s" , [this](lo_arg **argv,
        int, lo::Message message)
    {
      std::string message_time(&argv[0]->s);
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      VERBOSE3("OscReceiver: Got [/transport/seek, " << message_time <<
          "] from " << _handler.from_is() << " '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        float time;
        if(apf::str::string2time(message_time, time))
        {
          _controller.transport_locate(time);
        }
        else
        {
          ERROR("Couldn't get the time out of the \"seek\" attribute (\""
              << message_time << "\").");
        }
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /transport/seek s.");

  // set transport state: "transport/state, T, true"
  _handler.server().add_method("/transport/state", "T" , [this](lo_arg **argv,
        int, lo::Message message)
    {
      (void) argv;
      if(!message.types().compare("T"))
      {
        VERBOSE3("OscReceiver: Got [/transport/state, " <<
            _handler.bool_to_string(true) << "] from " << _handler.from_is() <<
            " '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          _controller.transport_start();
        }
      }
      if(!message.types().compare("F"))
      {
        VERBOSE3("OscReceiver: Got [/transport/state, " <<
            _handler.bool_to_string(false) << "] from " << _handler.from_is()
            << " '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        std::string hostname(message.source().hostname());
        std::string port(message.source().port());
        if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
            (_handler.is_server() && client_has_message_level(_handler,
                                                              hostname, port,
                                                              MessageLevel::SERVER)))
        {
          _controller.transport_stop();
        }
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /transport/state {F,T}.");
}

/**
 * Adds callback handlers (for clients and server) for tracker related messages
 * received from a server or a client with MessageLevel::SERVER (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_tracker_methods()
{
  // reset tracker: "/tracker/reset"
  _handler.server().add_method("/tracker/reset", NULL , [this](lo_arg **argv,
        int, lo::Message message)
    {
      (void) argv;
      VERBOSE2("OscReceiver: Got [/tracker/reset] from " << _handler.from_is()
          << " '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      std::string hostname(message.source().hostname());
      std::string port(message.source().port());
      if ((_handler.is_client() && is_server(_handler, hostname, port)) ||
          (_handler.is_server() && client_has_message_level(_handler, hostname,
                                                            port,
                                                            MessageLevel::SERVER)))
      {
        _controller.calibrate_client();
      }
    }
  );
  VERBOSE("OscReceiver: Added callback for /tracker/reset.");
}

/**
 * Creates a sane MessageLevel from an int32_t
 * @param message_level An int32_t
 * @return a MessageLevel
 */
ssr::MessageLevel ssr::OscReceiver::get_sane_message_level(int32_t message_level)
{
  if(message_level <= 0){
    return ssr::MessageLevel::CLIENT;
  }
  else if(ssr::MessageLevel::MAX_VALUE <
      static_cast<ssr::MessageLevel>(message_level))
  {
    return ssr::MessageLevel::GUI_SERVER;
  }
  else
  {
    return static_cast<ssr::MessageLevel>(message_level);
  }
}
