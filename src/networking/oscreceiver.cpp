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
  if (!_handler.mode().compare("server"))
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
  else if (!_handler.mode().compare("client"))
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
  _handler.server().add_method("/update/source/position_fixed", NULL, [](lo_arg **argv,
        int, lo::Message message)
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
  _handler.server().add_method("/update/source/orientation", "if", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", orientation: "<< argv[1]->f << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/orientation.");

  // update on source volume: "/update/source/volume, if, id, volume"
  _handler.server().add_method("/update/source/volume", "if", [](lo_arg **argv, int,
        lo::Message message)
    {
      lo::Address client(message.source());
      VERBOSE3("Update: Client '" << client.hostname() << "', source id = " <<
          argv[0]->i << ", volume: "<< argv[1]->f << ".");
    }
  );
  VERBOSE("OscReceiver: Added method for /update/source/volume.");

  // update on source position mute: "/update/source/mute, i{T,F},
  // id, {true,false}"
  _handler.server().add_method("/update/source/mute", NULL, [](lo_arg **argv, int,
        lo::Message message)
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
        set_server_for_client(_handler, from);
        from.send_from(_handler.server(), "/subscribe", "T");
        VERBOSE2("OscReceiver: Got /poll from server " << from.hostname() <<
            ":" << from.port() << ". Subscribing...");
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
  _handler.server().add_method("/source/position", "iff", [this](lo_arg **argv, int)
    {
      _controller.set_source_position(argv[0]->i, Position(argv[1]->f,
            argv[2]->f));
      VERBOSE2("set source position: id = " << argv[0]->i << ", " <<
        Position(argv[1]->f, argv[2]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/position.");

  // set source fixed: "source/position_fixed, iT, id, true"
  _handler.server().add_method("/source/position_fixed", "iT", [this](lo_arg **argv,
        int)
    {
      _controller.set_source_position_fixed(argv[0]->i, true);
      VERBOSE2("set source position fixed: id = " << argv[0]->i <<
          ", fixed = true");
    }
  );
  VERBOSE("OscReceiver: Added method for /source/position_fixed true.");

  // set source fixed: "source/position_fixed, iF, id, false"
  _handler.server().add_method("/source/position_fixed", "iF", [this](lo_arg **argv, int)
    {
      _controller.set_source_position_fixed(argv[0]->i, false);
      VERBOSE2("set source position fixed: id = " << argv[0]->i <<
          ", fixed = false");
    }
  );
  VERBOSE("OscReceiver: Added method for /source/position_fixed false.");

  // set source orientation: "source/orientation, if, id, azimuth"
  _handler.server().add_method("/source/orientation", "if", [this](lo_arg **argv, int)
    {
      _controller.set_source_orientation(argv[0]->i, Orientation(argv[1]->f));
      VERBOSE2("set source orientation: id = " << argv[0]->i << ", " <<
          Orientation(argv[1]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/orientation.");

  // set source volume: "source/volume, if, id, volume"
  _handler.server().add_method("/source/volume", "if", [this](lo_arg **argv, int)
    {
      _controller.set_source_gain(argv[0]->i,
          apf::math::dB2linear(argv[1]->f));
      VERBOSE2("set source volume: id = " << argv[0]->i << ", volume = " <<
          apf::math::dB2linear(argv[1]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/volume.");

  // set source mute: "source/mute, iT, id, true"
  _handler.server().add_method("/source/mute", "iT", [this](lo_arg **argv, int)
    {
      _controller.set_source_mute(argv[0]->i, true);
      VERBOSE2("set source mute: id = " << argv[0]->i << ", mute = true");
    }
  );
  VERBOSE("OscReceiver: Added method for /source/mute true.");

  // set source mute: "source/mute, iF, id, false"
  _handler.server().add_method("/source/mute", "iF", [this](lo_arg **argv, int)
    {
      _controller.set_source_mute(argv[0]->i, false);
      VERBOSE2("set source mute: id = " << argv[0]->i << ", mute = false");
    }
  );
  VERBOSE("OscReceiver: Added method for /source/mute false.");

  // set source name: "source/name, is, id, name"
  _handler.server().add_method("/source/name", "is", [this](lo_arg **argv, int)
    {
      _controller.set_source_name(argv[0]->i, apf::str::A2S(argv[1]->s));
      VERBOSE2("set source name: id = " << argv[0]->i << ", name = " <<
          apf::str::A2S(argv[1]->s));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/name.");

  // set source file: "/source/properties_file, is, id, properties_file"
  _handler.server().add_method("/source/properties_file", "is", [this](lo_arg **argv,
        int)
    {
      _controller.set_source_properties_file(argv[0]->i,
          apf::str::A2S(argv[1]->s));
      VERBOSE2("set source properties file name: id = " << argv[0]->i <<
          ", file = " << apf::str::A2S(argv[1]->s));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/properties_file.");

  // set source model: "/source/model, is, id, model"
  _handler.server().add_method("/source/model", "is", [this](lo_arg **argv, int)
    {
      Source::model_t model = Source::model_t();
      if (!apf::str::S2A(apf::str::A2S(argv[1]->s), model))
      {
        model = Source::point;
      }
      _controller.set_source_model(argv[0]->i, model);
      VERBOSE2("set source model: id = " << argv[0]->i << ", model = " <<
          apf::str::A2S(argv[1]->s));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/model.");

  // set source port name: "/source/port_name, is, id, port_name"
  _handler.server().add_method("/source/port_name", "is", [this](lo_arg **argv, int)
    {
      _controller.set_source_port_name(argv[0]->i, apf::str::A2S(argv[1]->s));
      VERBOSE2("set source port name: id = " << argv[0]->i << ", port = " <<
          apf::str::A2S(argv[1]->s));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/port_name.");

  // create new source: "/source/new, sssffff{T,F}{T,F}{T,F}, name, model,
  // file_name_or_port_number, x, y, orientation, volume, position_fixed,
  // orientation_fixed, muted"
  // create new source: "/source/new, sssffffis{T,F}{T,F}{T,F}, name, model,
  // file_name_or_port_number, x, y, orientation, volume, channel,
  // properties_file, position_fixed, orientation_fixed, muted"
  _handler.server().add_method("/source/new", NULL, [this](lo_arg **argv, int,
        lo::Message message)
    {
      VERBOSE3("OscReceiver: [/source/new, " << message.types() << "].");
      std::string name(&(argv[0]->s));
      std::string file_name_or_port_number(&(argv[2]->s));
      std::string types(message.types());
      float x(argv[3]->f);
      float y(argv[4]->f);
      float volume(argv[6]->f);
      int channel = 0;
      std::string properties_file = "";
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
        position_fixed = true;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffisTTF") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        position_fixed = true;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffisTFF") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        position_fixed = true;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffisFFF") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        position_fixed = false;
        orientation_fixed = false;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffisTFT") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        position_fixed = true;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffisFTF") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        position_fixed = false;
        orientation_fixed = true;
        muted = false;
        setup = true;
      }
      if (types.compare("sssffffisFTT") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        position_fixed = false;
        orientation_fixed = true;
        muted = true;
        setup = true;
      }
      if (types.compare("sssffffisFFT") == 0)
      {
        channel = argv[7]->i;
        properties_file = &(argv[8]->s);
        position_fixed = false;
        orientation_fixed = false;
        muted = true;
        setup = true;
      }
      if (setup)
      {
        _controller.new_source(name, model, file_name_or_port_number, channel,
            position, position_fixed, orientation, orientation_fixed,
            volume, muted, properties_file);
        VERBOSE2("Creating source with following properties:"
            "\nname: " << name <<
            "\nmodel: " << model <<
            "\nfile_name_or_port_number: " << file_name_or_port_number <<
            "\nchannel: " << channel <<
            "\nposition: " << position <<
            "\nposition_fixed: " << position_fixed <<
            "\norientation: " << orientation <<
            "\norientation_fixed: " << orientation_fixed <<
            "\nvolume (linear): " << volume <<
            "\nmuted: " << muted <<
            "\nproperties_file: " << properties_file);
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /source/new.");

  // delete source: "/source/delete, i, id"
  // special case: i == 0 deletes all sources!
  _handler.server().add_method("/source/delete", "i", [this](lo_arg **argv, int)
    {
      if (argv[0]->i == 0)
      {
        _controller.delete_all_sources();
        VERBOSE2("delete all sources");
      }
      else
      {
        _controller.delete_source(argv[0]->i);
        VERBOSE2("delete source with id = " << argv[0]->i);
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /source/delete.");
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
  _handler.server().add_method("/reference/position", "ff", [this](lo_arg **argv, int)
    {
      _controller.set_reference_position(Position(argv[0]->f, argv[1]->f));
      VERBOSE2("set reference position: " << Position(argv[0]->f, argv[1]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /source/position.");

  // set reference orientation: "/reference/orientation, f, azimuth"
  _handler.server().add_method("/reference/orientation", "f", [this](lo_arg **argv, int)
    {
      _controller.set_reference_orientation(Orientation(argv[0]->f));
      VERBOSE2("set reference orientation: " << Orientation(argv[0]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /reference/orientation.");

  // set reference offset position: "/reference_offset/position, ff, x, y"
  _handler.server().add_method("/reference_offset/position", "ff" , [this](lo_arg
        **argv, int)
    {
      _controller.set_reference_offset_position(Position(argv[0]->f,
            argv[1]->f));
      VERBOSE2("set reference offset position: " << Position(argv[0]->f,
          argv[1]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /reference_offset/position.");

  // set reference offset orientation: "/reference_offset/orientation, f,
  // azimuth"
  _handler.server().add_method("/reference_offset/orientation", "f" , [this](lo_arg
        **argv, int)
    {
      _controller.set_reference_offset_orientation(Orientation(argv[0]->f));
      VERBOSE2("set reference offset orientation: " <<
          Orientation(argv[0]->f));
    }
  );
  VERBOSE("OscReceiver: Added method for /reference_offset/orientation.");
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
  _handler.server().add_method("/scene/save", "s" , [this](lo_arg **argv, int)
    {
      _controller.save_scene_as_XML(std::string(&(argv[0]->s)));
      VERBOSE2("saving scene as: " << std::string(&(argv[0]->s)));
    }
  );
  VERBOSE("OscReceiver: Added method for /scene/save.");

  // load scene from file: "/scene/load, s, file"
  _handler.server().add_method("/scene/load", "s" , [this](lo_arg **argv, int)
    {
      _controller.load_scene(std::string(&(argv[0]->s)));
      VERBOSE2("loading scene: " << std::string(&(argv[0]->s)));
    }
  );
  VERBOSE("OscReceiver: Added method for /scene/load.");

  // set master volume: "/scene/volume, f, volume"
  _handler.server().add_method("/scene/volume", "f" , [this](lo_arg **argv, int)
    {
      _controller.set_master_volume(apf::math::dB2linear(argv[0]->f));
      VERBOSE2("set master volume: " << apf::math::dB2linear(argv[0]->f) <<
          " dB");
    }
  );
  VERBOSE("OscReceiver: Added method for /scene/volume.");

  // clear scene: "/scene/clear"
  _handler.server().add_method("/scene/clear", NULL , [this](lo_arg **argv, int)
    {
      (void) argv;
      _controller.delete_all_sources();
      VERBOSE2("clearing scene.");
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
  _handler.server().add_method("/processing/state", "T" , [this](lo_arg **argv, int)
    {
      (void) argv;
      _controller.start_processing();
      VERBOSE2("start processing.");
    }
  );
  VERBOSE("OscReceiver: Added method for /processing/state true.");

  // set processing state: "/processing/state, F, false"
  _handler.server().add_method("/processing/state", "F" , [this](lo_arg **argv, int)
    {
      (void) argv;
      _controller.stop_processing();
      VERBOSE2("stop processing.");
    }
  );
  VERBOSE("OscReceiver: Added method for /processing/state false.");
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
  _handler.server().add_method("/transport/state", "T" , [this](lo_arg **argv, int)
    {
      (void) argv;
      _controller.transport_start();
      VERBOSE2("start transport.");
    }
  );
  VERBOSE("OscReceiver: Added method for /transport/state true.");

  // set transport state: "/transport/state, F, false"
  _handler.server().add_method("/transport/state", "F" , [this](lo_arg **argv, int)
    {
      (void) argv;
      _controller.transport_stop();
      VERBOSE2("stop transport.");
    }
  );
  VERBOSE("OscReceiver: Added method for /transport/state false.");

  // rewind transport state: "/transport/rewind"
  _handler.server().add_method("/transport/rewind", NULL , [this](lo_arg **argv, int)
    {
      (void) argv;
      _controller.transport_locate(0);
      VERBOSE2("rewind transport.");
    }
  );
  VERBOSE("OscReceiver: Added method for /transport/rewind.");

  // seek transport state: "/transport/seek, s, time"
  _handler.server().add_method("/transport/seek", "s" , [this](lo_arg **argv, int)
    {
      float time;
      if(apf::str::string2time(apf::str::A2S(argv[0]->s), time))
      {
        _controller.transport_locate(time);
        VERBOSE2("Seek transport to: " << time);
      }
      else
      {
        ERROR("Couldn't get the time out of the \"seek\" attribute (\""
            << argv[0]->s << "\").");
      }
    }
  );
  VERBOSE("OscReceiver: Added method for /transport/seek.");
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
  _handler.server().add_method("/tracker/reset", NULL , [this](lo_arg **argv, int)
    {
      (void) argv;
      _controller.calibrate_client();
      VERBOSE2("calibrate tracker.");
    }
  );
  VERBOSE("OscReceiver: Added method for /tracker/reset.");
}

