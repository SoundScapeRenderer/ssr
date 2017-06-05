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
 * @todo add error handler for ServerThread
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
    add_update_notification_methods();
    add_source_methods();
    add_reference_methods();
    add_scene_methods();
    add_processing_methods();
    add_transport_methods();
    add_tracker_methods();
  }
  else if (_handler.is_client())
  {
    add_poll_methods();
    add_source_methods();
    add_reference_methods();
    add_scene_methods();
    add_processing_methods();
    add_transport_methods();
    add_tracker_methods();
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
 * Adds callback handlers (for server) for subscribe and message level messages
 * received from clients.
 * This function uses C++11 lambda functions to define the behavior for every
 * callback.
 */
void ssr::OscReceiver::add_client_to_server_methods()
{
  // adding new subscribing client: "/subscribe, {T,Ti,F}"
  _handler.server().add_method("/subscribe", NULL, [this](lo_arg **argv, int, lo::Message
        message)
    {
      lo::Address client(message.source());
      if(!message.types().compare("T"))
      {
        VERBOSE2("OscReceiver: Got subscribe request from '" <<
            client.hostname() << ":" << client.port() << "'.");
        add_client(_handler, client, ssr::MessageLevel::CLIENT);
      }
      else if(!message.types().compare("F"))
      {
        VERBOSE2("OscReceiver: Got unsubscribe request from '" <<
            client.hostname() << ":" << client.port() << "'.");
        deactivate_client(_handler, client);
      }
      else if(!message.types().compare("Ti"))
      {
        VERBOSE2("OscReceiver: Got subscribe request from '" <<
            client.hostname() << ":" << client.port() <<
            "' for message level: " << argv[1]->i);
        add_client(_handler, client, static_cast<ssr::MessageLevel>(argv[1]->i));
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /subscribe {T,F,Ti}.");

  // adding new subscribing client: "/message_level, i"
  _handler.server().add_method("/message_level", "i", [this](lo_arg **argv, int, lo::Message
        message)
    {
      lo::Address client(message.source());
      VERBOSE2("OscReceiver: Got request to set message level for client '" <<
          client.hostname() << ":" << client.port() << "' to: " << argv[0]->i);
      set_message_level(_handler, client,
          static_cast<ssr::MessageLevel>(argv[0]->i));
    }
  );
  VERBOSE("OscReceiver: Added method for /message_level i.");
}

/**
 * Adds callback handlers (for server) for update messages received from
 * clients.
 * This function uses C++11 lambda functions to define the behavior for every
 * callback.
 */
void ssr::OscReceiver::add_update_notification_methods()
{
  // update on new source: "/update/source/new, i, id"
  _handler.server().add_method("/update/source/new", "i", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << " created.");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/new.");

  // update on deleted source: "/update/source/delete, i, id"
  _handler.server().add_method("/update/source/delete", "i", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      if(argv[0]->i == 0)
      {
        VERBOSE3("Update: Client '" << client.hostname() <<
            "', all sources deleted.");
      }
      else
      {
        VERBOSE3("Update: Client '" << client.hostname() << "', source id = "
            << argv[0]->i << " deleted.");
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/delete.");

  // update on source position: "/update/source/position, iff, id, x, y"
  _handler.server().add_method("/update/source/position", "iff", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", position: "<< argv[1]->f << "/" << argv[2]->f <<
          ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/position.");

  // update on source position fixation: "/update/source/position_fixed, i{T,F},
  // id, {true,false}"
  _handler.server().add_method("/update/source/position_fixed", NULL, [](lo_arg
        **argv, int, lo::Message message)
    {
      lo::Address client(message.source());
      std::string position_fixed;
      if(message.types() == "iT")
      {
        position_fixed = "true";
      }
      else if(message.types() == "iF")
      {
        position_fixed = "false";
      }
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", position_fixed: "<< position_fixed << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/position_fixed.");

  // update on source orientation: "/update/source/orientation, if, id, azimuth"
  _handler.server().add_method("/update/source/orientation", "if", [](lo_arg
        **argv, int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", orientation: "<< argv[1]->f << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/orientation.");

  // update on source gain: "/update/source/gain, if, id, gain"
  _handler.server().add_method("/update/source/gain", "if", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", gain: "<< argv[1]->f << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/gain.");

  // update on source mute: "/update/source/mute, i{T,F}, id, {true,false}"
  _handler.server().add_method("/update/source/mute", NULL, [](lo_arg **argv,
        int, lo::Message message)
    {
      lo::Address client(message.source());
      std::string state;
      if(message.types() == "iT")
      {
        state = "true";
      }
      else if(message.types() == "iF")
      {
        state = "false";
      }
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          apf::str::A2S(argv[0]->i) << ", mute: "<< state  << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/mute.");

  // update on source name: "/update/source/name, is, id, name"
  _handler.server().add_method("/update/source/name", "is", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", name: "<< argv[1]->s << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/name.");

  // update on source properties_file: "/update/source/properties_file, is, id,
  // properties_file"
  _handler.server().add_method("/update/source/properties_file", "is", [](lo_arg **argv,
        int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", properties_file: "<< argv[1]->s << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/properties_file.");

  // update on scene decay exponent: "/update/scene/decay_exponent, f,
  // decay_exponent"
  _handler.server().add_method("/update/scene/decay_exponent", "f", [](lo_arg **argv,
        int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', decay_exponent: "<< argv[0]->f << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/scene/decay_exponent.");

  // update on scene amplitude reference distance:
  // "update/scene/amplitude_reference_distance, f,
  // amplitude_reference_distance"
  _handler.server().add_method("/update/scene/amplitude_reference_distance", "f",
      [](lo_arg **argv, int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', amplitude_reference_distance: "<< argv[0]->f << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for \
/update/scene/amplitude_reference_distance.");

  // update on source model: "/update/source/model, is, id, model"
  _handler.server().add_method("/update/source/model", "is", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", model: "<< argv[1]->s << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/model.");

  // update on source port_name: "/update/source/port_name, is, id, port_name"
  _handler.server().add_method("/update/source/port_name", "is", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", port_name: "<< argv[1]->s << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/port_name.");

  // update on source file_name_or_port_number:
  // "/update/source/file_name_or_port_number, is, id, file_name_or_port_number"
  _handler.server().add_method("/update/source/file_name_or_port_number", "is",
      [](lo_arg **argv, int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", file_name_or_port_number: "<< argv[1]->s << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for \
/update/source/file_name_or_port_number.");

  // update on source channel: "/update/source/channel, ii, id, channel"
  _handler.server().add_method("/update/source/channel", "ii", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", channel: "<< argv[1]->i << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/channel.");

  // update on source file length: "/update/source/length, ii, id, length"
  _handler.server().add_method("/update/source/length", "ii", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", length: "<< argv[1]->i << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/length.");

  // update on reference position: "/update/reference/position, ff, x, y"
  _handler.server().add_method("/update/reference/position", "ff", [](lo_arg **argv,
        int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', reference position: "<< argv[0]->f << "/" << argv[1]->f <<
          " (x/y).");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/reference/position.");

  // update on reference orientation: "/update/reference/orientation, f,
  // azimuth"
  _handler.server().add_method("/update/reference/orientation", "f", [](lo_arg **argv,
        int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', reference orientation: "<< argv[0]->f << " (azimuth).");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/reference/orientation.");

  // update on reference offset position: "/update/reference_offset/position,
  // ff, x, y"
  _handler.server().add_method("/update/reference_offset/position", "ff", [](lo_arg
        **argv, int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', reference offset position: "<< argv[0]->f << "/" << argv[1]->f
          << " (x/y).");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/reference_offset/position.");

  // update on reference offset orientation:
  // "/update/reference_offset/orientation, f, azimuth"
  _handler.server().add_method("/update/reference_offset/orientation", "f", [](lo_arg
        **argv, int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', reference offset orientation: "<< argv[0]->f << " (azimuth).");
    }
  );
  VERBOSE("OscReceiver: Added method for \
/update/reference_offset/orientation.");

  // update on scene volume: "/update/scene/volume, f, volume"
  _handler.server().add_method("/update/scene/volume", "f", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', scene volume: "<<
          argv[0]->f << "dB.");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/scene/volume.");

  // update on state processing: "/update/processing/state, {T,F}, {true,false}"
  _handler.server().add_method("/update/processing/state", NULL, [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      std::string state;
      (void) argv;
      if(message.types() == "T")
      {
        state = "true";
      }
      else if(message.types() == "F")
      {
        state = "false";
      }
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', state processing: " << state  << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/processing/state.");

  // update on transport state: "/update/transport/state, {T,F}, {true,false}"
  _handler.server().add_method("/update/transport/state", NULL, [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      std::string state;
      (void) argv;
      if(message.types() == "T")
      {
        state = "true";
      }
      else if(message.types() == "F")
      {
        state = "false";
      }
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', transport state: "<< state  << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/transport/state.");

  // update on transport seek: "/update/transport/seek, s, time"
  _handler.server().add_method("/update/transport/seek", "s", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', transport seek: "<< argv[0]->s << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/transport/seek.");

  // update on scene source auto rotation: "/update/scene/auto_rotate_sources,
  // {T,F}, {true,false}"
  _handler.server().add_method("/update/scene/auto_rotate_sources", NULL, [](lo_arg
        **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      std::string state;
      (void) argv;
      if(message.types() == "T")
      {
        state = "true";
      }
      else if(message.types() == "F")
      {
        state = "false";
      }
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', scene auto_rotate_sources: "<< state  << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/scene/auto_rotate_sources.");

  // update on cpu_load: "/update/cpu_load, f, load"
  _handler.server().add_method("/update/cpu_load", "f", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', cpu_load: "<<
          argv[0]->f << "%.");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/cpu_load.");

  // update on scene sample rate: "/update/scene/sample_rate, i, sample_rate"
  _handler.server().add_method("/update/scene/sample_rate", "i", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', scene sample_rate: "<< argv[0]->i << "Hz.");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/scene/sample_rate.");

  // update on scene master signal level: "/update/scene/master_signal_level, f,
  // master_signal_level"
  _handler.server().add_method("/update/scene/master_signal_level", "f", [](lo_arg
        **argv, int, lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() <<
          "', scene master_signal_level: "<< argv[0]->f << "dB.");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/scene/master_signal_level.");

  // update on source signal level: "/update/source/level, if, id, level"
  _handler.server().add_method("/update/source/level", "if", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", level: "<< argv[1]->f << "dB.");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/level.");
}

/**
 * Adds callback handlers (for clients) for poll messages received from a
 * server.
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_poll_methods()
{
  // set _server_address for OscSender through OscHandler, depending on, if
  // polled from given server before
  _handler.server().add_method("/poll", NULL, [this](lo_arg **argv, int, lo::Message
        message)
    {
      lo::Address server(server_address(_handler));
      lo::Address from(message.source());
      (void) argv;
      if((server.hostname().compare(from.hostname()) != 0) &&
          (server.port().compare(from.port()) != 0) &&
          (from.port().compare("50001") != 0) &&
          (from.hostname().compare("none") != 0)
        )
      {
        VERBOSE2("OscReceiver: Got [/poll] from server " << from.hostname() <<
            ":" << from.port() << ". Subscribing...");
        set_server_for_client(_handler, from);
        from.send_from(_handler.server(), "/subscribe", "T");
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /poll.");
}

/**
 * Adds callback handlers (for clients and server) for source related messages
 * received from a server or a client with MessageLevel::SERVER (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 * @todo add checks for client MessageLevel when this is used for a server
 * (only clients with message_level MessageLevel::SERVER ought to be able to
 * set source related settings).
 */
void ssr::OscReceiver::add_source_methods()
{
  // set source position: "source/position, iff, id, x, y"
  _handler.server().add_method("/source/position", "iff", [this](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/source/position, " << argv[0]->i << ", " <<
        argv[1]->f << ", " <<  argv[2]->f << "] from client '" <<
        message.source().hostname() << ":" << message.source().port() << "'.");
      _controller.set_source_position(argv[0]->i, Position(argv[1]->f,
            argv[2]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/position iff.");

  // set source fixed: "source/position_fixed, i{T,F}, id, true|false"
  _handler.server().add_method("/source/position_fixed", NULL, [this](lo_arg
        **argv, int, lo::Message message)
    {
      if (!message.types().compare("iT"))
      {
        VERBOSE2("OscReceiver: Got [/source/position_fixed, " << argv[0]->i <<
            ", true] from client '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        _controller.set_source_position_fixed(argv[0]->i, true);
      }
      else if (!message.types().compare("iF"))
      {
        VERBOSE2("OscReceiver: Got [/source/position_fixed, " << argv[0]->i <<
            ", false] from client '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        _controller.set_source_position_fixed(argv[0]->i, false);
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /source/position_fixed i{T,F}.");

  // set source orientation: "source/orientation, if, id, azimuth"
  _handler.server().add_method("/source/orientation", "if", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/source/orientation, " << argv[0]->i << ", "
          << argv[1]->f << "] from client '" << message.source().hostname() <<
          ":" << message.source().port() << "'.");
      _controller.set_source_orientation(argv[0]->i, Orientation(argv[1]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/orientation if.");

  // set source gain: "/source/gain, if, id, gain"
  _handler.server().add_method("/source/gain", "if", [this](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/source/gain, " << argv[0]->i << ", " <<
          argv[1]->f << "] from client '" << message.source().hostname() << ":"
          << message.source().port() << "'.");
      _controller.set_source_gain(argv[0]->i,
          apf::math::dB2linear(argv[1]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/gain if.");

  // set source mute: "source/mute, i{T,F}, id, true|false"
  _handler.server().add_method("/source/mute", NULL, [this](lo_arg **argv, int,
        lo::Message message)
    {
      if(!message.types().compare("iT"))
      {
        VERBOSE2("OscReceiver: Got [/source/mute, " << argv[0]->i <<
            ", true] from client '"<< message.source().hostname() << ":" <<
            message.source().port() << "'.");
        _controller.set_source_mute(argv[0]->i, true);
      }
      else if(!message.types().compare("iF"))
      {
        VERBOSE2("OscReceiver: Got [/source/mute, " << argv[0]->i <<
            ", false] from client '"<< message.source().hostname() << ":" <<
            message.source().port() << "'.");
        _controller.set_source_mute(argv[0]->i, false);
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /source/mute i{T,F}.");

  // set source name: "source/name, is, id, name"
  _handler.server().add_method("/source/name", "is", [this](lo_arg **argv, int,
      lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/source/name, " << argv[0]->i << ", " <<
          name << "] from client '" << message.source().hostname() << ":" <<
          message.source().port() <<
          "'.");
      _controller.set_source_name(argv[0]->i, name);
    }
  );
  VERBOSE("OscReceiver: Added method for /source/name is.");

  // set source file: "/source/properties_file, is, id, properties_file"
  _handler.server().add_method("/source/properties_file", "is", [this](lo_arg **argv,
        int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/source/properties_file, " << argv[0]->i <<
          ", " << name << "] from client '" << message.source().hostname() <<
          ":" << message.source().port() << "'.");
      _controller.set_source_properties_file(argv[0]->i, name);
    }
  );
  VERBOSE("OscReceiver: Added method for /source/properties_file is.");

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
          name << "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      _controller.set_source_model(argv[0]->i, model);
    }
  );
  VERBOSE("OscReceiver: Added method for /source/model is.");

  // set source port name: "/source/port_name, is, id, port_name"
  _handler.server().add_method("/source/port_name", "is", [this](lo_arg **argv,
        int, lo::Message message)
    {
      std::string name(&argv[1]->s);
      VERBOSE2("OscReceiver: Got [/source/port_name, " << argv[0]->i << ", " <<
          name << "] from client '" << message.source().hostname() << ":" << 
          message.source().port() << "'.");
      _controller.set_source_port_name(argv[0]->i, name);
    }
  );
  VERBOSE("OscReceiver: Added method for /source/port_name is.");

  // create new source: "/source/new, sssffff{T,F}{T,F}{T,F}, name, model,
  // file_name_or_port_number, x, y, orientation, gain, position_fixed,
  // orientation_fixed, muted"
  // create new source: "/source/new, sssffffis{T,F}{T,F}{T,F}, name, model,
  // file_name_or_port_number, x, y, orientation, gain, channel,
  // properties_file, position_fixed, orientation_fixed, muted"
  _handler.server().add_method("/source/new", NULL, [this](lo_arg **argv, int,
        lo::Message message)
    {
      std::string name(&(argv[0]->s));
      std::string file_name_or_port_number(&(argv[2]->s));
      std::string types(message.types());
      float x(argv[3]->f);
      float y(argv[4]->f);
      float gain(argv[6]->f);
      int channel = 0;
      std::string properties_file = "";
      std::string channel_and_properties = "";
      bool position_fixed;
      bool orientation_fixed;
      bool muted;
      bool setup = false;
      Source::model_t model = Source::model_t();
      if (!apf::str::S2A(apf::str::A2S(argv[1]->s), model))
      {
        model = Source::point;
      }
      Position position(x, y);
      Orientation orientation(argv[5]->f);
      if (types.compare("sssffffTTT") == 0)
      {
        position_fixed = true;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffTTF") == 0)
      {
        position_fixed = true;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffTFF") == 0)
      {
        position_fixed = true;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffFFF") == 0)
      {
        position_fixed = false;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffTFT") == 0)
      {
        position_fixed = true;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffFTF") == 0)
      {
        position_fixed = false;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffFTT") == 0)
      {
        position_fixed = false;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffFFT") == 0)
      {
        position_fixed = false;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffisTTT") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (channel+", "+properties_file);
        position_fixed = true;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffisTTF") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (channel+", "+properties_file);
        position_fixed = true;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffisTFF") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (channel+", "+properties_file);
        position_fixed = true;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffisFFF") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (channel+", "+properties_file);
        position_fixed = false;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffisTFT") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (channel+", "+properties_file);
        position_fixed = true;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffisFTF") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (channel+", "+properties_file);
        position_fixed = false;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffisFTT") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (channel+", "+properties_file);
        position_fixed = false;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffisFFT") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        channel_and_properties = (channel+", "+properties_file);
        position_fixed = false;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (setup)
      {
        VERBOSE3("OscReceiver: Got [/source/new, " << name << ", " << model <<
            ", " << file_name_or_port_number << ", " << x << ", " << y << ", "
            << orientation.azimuth << ", " << gain<< ", " <<
            channel_and_properties << ", " <<
            _handler.bool_to_string(position_fixed) << ", " <<
            _handler.bool_to_string(orientation_fixed) << ", " <<
            _handler.bool_to_string(muted) <<  "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        _controller.new_source(name, model, file_name_or_port_number, channel,
            position, position_fixed, orientation, orientation_fixed, gain,
            muted, properties_file);
        VERBOSE2("OscReceiver: Created source with following properties:"
            "\nname: " << name <<
            "\nmodel: " << model <<
            "\nfile_name_or_port_number: " << file_name_or_port_number <<
            "\nchannel: " << channel <<
            "\nposition: " << position <<
            "\nposition_fixed: " << position_fixed <<
            "\norientation: " << orientation <<
            "\norientation_fixed: " << orientation_fixed <<
            "\ngain (linear): " << gain <<
            "\nmuted: " << muted <<
            "\nproperties_file: " << properties_file);
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /source/new {sssffff,sssffffis}{F,T}{F,T}{F,T}.");

  // delete source: "/source/delete, i, id"
  // special case: i == 0 deletes all sources!
  _handler.server().add_method("/source/delete", "i", [this](lo_arg **argv,
        int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/source/delete, " << argv[0]->i <<
          "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      if (argv[0]->i == 0)
      {
        _controller.delete_all_sources();
      }
      else
      {
        _controller.delete_source(argv[0]->i);
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /source/delete i.");
}

/**
 * Adds callback handlers (for clients and server) for reference related
 * messages received from a server or a client with MessageLevel::SERVER
 * (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 * @todo add checks for client MessageLevel when this is used for a server
 * (only clients with message_level MessageLevel::SERVER ought to be able to
 * interact)
 */
void ssr::OscReceiver::add_reference_methods()
{
  // set reference position: "/reference/position, ff, x, y"
  _handler.server().add_method("/reference/position", "ff", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/reference/position, " << argv[0]->f << ", "
          << argv[1]->f << "] from client '" << message.source().hostname() <<
          ":" <<  message.source().port() << "'.");
      _controller.set_reference_position(Position(argv[0]->f, argv[1]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /reference/position ff.");

  // set reference orientation: "/reference/orientation, f, azimuth"
  _handler.server().add_method("/reference/orientation", "f", [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/reference/orientation, " << argv[0]->f <<
          "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      _controller.set_reference_orientation(Orientation(argv[0]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /reference/orientation f.");

  // set reference offset position: "/reference_offset/position, ff, x, y"
  _handler.server().add_method("/reference_offset/position", "ff" , [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/reference/offset_position, " << argv[0]->f
          << ", " << argv[1]->f << "] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      _controller.set_reference_offset_position(Position(argv[0]->f,
            argv[1]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /reference_offset/position ff.");

  // set reference offset orientation: "/reference_offset/orientation, f,
  // azimuth"
  _handler.server().add_method("/reference_offset/orientation", "f" , [this](lo_arg
        **argv, int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/reference_offset/orientation, " <<
          argv[0]->f << "] from client '" << message.source().hostname() << ":"
          << message.source().port() << "'.");
      _controller.set_reference_offset_orientation(Orientation(argv[0]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /reference_offset/orientation f.");
}

/**
 * Adds callback handlers (for clients and server) for scene related messages
 * received from a server or a client with MessageLevel::SERVER (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 * @todo add checks for client MessageLevel when this is used for a server
 * (only clients with message_level MessageLevel::SERVER ought to be able to
 * interact)
 */
void ssr::OscReceiver::add_scene_methods()
{
  // save scene to file: "/scene/save, s, file"
  _handler.server().add_method("/scene/save", "s" , [this](lo_arg **argv, int,
        lo::Message message)
    {
      std::string name(&argv[0]->s);
      VERBOSE2("OscReceiver: Got [/scene/save, " << name << "] from client '"
          << message.source().hostname() << ":" << message.source().port() <<
          "'.");
      _controller.save_scene_as_XML(name);
    }
  );
  VERBOSE("OscReceiver: Added method for /scene/save s.");

  // load scene from file: "/scene/load, s, file"
  _handler.server().add_method("/scene/load", "s" , [this](lo_arg **argv, int,
        lo::Message message)
    {
      std::string name(&argv[0]->s);
      VERBOSE2("OscReceiver: Got [/scene/load, " << name << "] from client '"
          << message.source().hostname() << ":" << message.source().port() <<
          "'.");
      _controller.load_scene(name);
    }
  );
  VERBOSE("OscReceiver: Added method for /scene/load s.");

  // set master volume: "/scene/volume, f, volume"
  _handler.server().add_method("/scene/volume", "f" , [this](lo_arg **argv,
      int, lo::Message message)
    {
      VERBOSE2("OscReceiver: Got [/scene/volume, " << argv[0]->f <<
          "] from client '" << message.source().hostname() << ":" <<
          message.source().port() << "'.");
      _controller.set_master_volume(apf::math::dB2linear(argv[0]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /scene/volume f.");

  // clear scene: "/scene/clear"
  _handler.server().add_method("/scene/clear", NULL , [this](lo_arg **argv,
        int, lo::Message message)
    {
      (void) argv;
      VERBOSE2("OscReceiver: [/scene/clear] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      _controller.delete_all_sources();
    }
  );
  VERBOSE("OscReceiver: Added method for /scene/clear.");
}

/**
 * Adds callback handlers (for clients and server) for processing related
 * messages received from a server or a client with MessageLevel::SERVER
 * (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 * @todo add checks for client MessageLevel when this is used for a server
 * (only clients with message_level MessageLevel::SERVER ought to be able to
 * interact)
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
            _handler.bool_to_string(true) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        _controller.start_processing();
      }
      else if(!message.types().compare("T"))
      {
        VERBOSE2("OscReceiver: Got [/processing/state, " <<
            _handler.bool_to_string(false) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        _controller.stop_processing();
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /processing/state {T,F}.");

}

/**
 * Adds callback handlers (for clients and server) for transport related
 * messages received from a server or a client with MessageLevel::SERVER
 * (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 * @todo add checks for client MessageLevel when this is used for a server
 * (only clients with message_level MessageLevel::SERVER ought to be able to
 * interact)
 */
void ssr::OscReceiver::add_transport_methods()
{
  // set transport state: "transport/state, T, true"
  _handler.server().add_method("/transport/state", "T" , [this](lo_arg **argv,
        int, lo::Message message)
    {
      (void) argv;
      if(!message.types().compare("T"))
      {
        VERBOSE2("OscReceiver: Got [/transport/state, " <<
            _handler.bool_to_string(true) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        _controller.transport_start();
      }
      if(!message.types().compare("F"))
      {
        VERBOSE2("OscReceiver: Got [/transport/state, " <<
            _handler.bool_to_string(false) << "] from client '" <<
            message.source().hostname() << ":" << message.source().port() <<
            "'.");
        _controller.transport_stop();
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /transport/state {T,F}.");

  // rewind transport state: "/transport/rewind"
  _handler.server().add_method("/transport/rewind", NULL , [this](lo_arg
        **argv, int, lo::Message message)
    {
      (void) argv;
      VERBOSE2("OscReceiver: Got [/transport/rewind] from client '" <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      _controller.transport_locate(0);
    }
  );
  VERBOSE("OscReceiver: Added method for /transport/rewind.");

  // seek transport state: "/transport/seek, s, time"
  _handler.server().add_method("/transport/seek", "s" , [this](lo_arg **argv,
        int, lo::Message message)
    {
      float time;
      std::string message_time(&argv[0]->s);
      if(apf::str::string2time(message_time, time))
      {
        VERBOSE2("OscReceiver: Got [/transport/seek, " << message_time <<
            "] from client '" << message.source().hostname() << ":" <<
            message.source().port() << "'.");
        _controller.transport_locate(time);
      }
      else
      {
        ERROR("Couldn't get the time out of the \"seek\" attribute (\""
            << message_time << "\").");
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /transport/seek s.");
}

/**
 * Adds callback handlers (for clients and server) for tracker related messages
 * received from a server or a client with MessageLevel::SERVER (respectively).
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 * @todo add checks for client MessageLevel when this is used for a server
 * (only clients with message_level MessageLevel::SERVER ought to be able to
 * interact)
 */
void ssr::OscReceiver::add_tracker_methods()
{
  // reset tracker: "/tracker/reset"
  _handler.server().add_method("/tracker/reset", NULL , [this](lo_arg **argv,
        int, lo::Message message)
    {
      (void) argv;
      VERBOSE2("OscReceiver: Got [/tracker/reset] from client " <<
          message.source().hostname() << ":" << message.source().port() <<
          "'.");
      _controller.calibrate_client();
    }
  );
  VERBOSE("OscReceiver: Added method for /tracker/reset.");
}

