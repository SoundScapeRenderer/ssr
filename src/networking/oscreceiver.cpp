/**
 * Implementation of oscreceiver.h
 * @file oscreceiver.cpp
 */

#include "oscreiver.h"
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
ssr::OscReceiver::OscReceiver(Publisher& controller, int port_in)
  : _controller(controller)
  , _receiver(port_in)
{}

/**
 * Destructor
 * Stops the lo::ServerThread, used for listening for OSC messages
 */
ssr::OscReceiver::~OscReceiver()
{
  _receiver.stop();
}

/**
 * Starts the OscReceiver, by adding client|server callback functions and
 * starting the lo::ServerThread used for listening to OSC messages.
 */
void ssr::OscReceiver::start()
{
  // add method handlers for received messages
  if (_mode == "server")
  {
    add_client_to_server_methods();
  }
  else if (_mode == "client")
  {
    add_server_to_client_methods();
  }
  // start server thread
  _receiver.start();
}

/**
 * Adds callback handlers for OSC messages received from clients.
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_client_to_server_methods()
{
//TODO: implement!
}

/**
 * Adds callback handlers for OSC messages received from a server.
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void ssr::OscReceiver::add_server_to_client_methods()
{
  // set _server_address for OscSender through OscHandler, depending on, if
  // polled from given server before
  _receiver.add_method("poll", NULL, [](lo_arg **argv, int, lo::Message
        message)
    {
      lo::Address server = server_address(&_handler);
      lo::Address from(message.source());
      if(server.hostname() != from.hostname() && server.port() != from.port())
      {
        set_server_address(from);
        // TODO: send reply to subscribed server
      }
    }
  ):

  // set source position: "source/position, iff, id, x, y"
  _receiver.add_method("source/position", "iff", [](lo_arg **argv, int)
    {
      _controller.set_source_position(argv[0]->i, Position(argv[1]->f,
            argv[2]->f);
      VERBOSE2("set source position: id = " << argv[0]->i << ", " <<
        Position(argv[1]->f, argv[2]->f));
    }
  );

  // set source fixed: "source/position_fixed, iT, id, true"
  _receiver.add_method("source/position_fixed", "iT", [](lo_arg **argv, int)
    {
      _controller.set_source_position_fixed(argv[0]->i, true);
      VERBOSE2("set source position fixed: id = " << argv[0]->i << ", fixed =
          true");
    }
  );

  // set source fixed: "source/position_fixed, iF, id, false"
  _receiver.add_method("source/position_fixed", "iF", [](lo_arg **argv, int)
    {
      _controller.set_source_position_fixed(argv[0]->i, false);
      VERBOSE2("set source position fixed: id = " << argv[0]->i << ", fixed =
          false");
    }
  );

  // set source orientation: "source/orientation, if, id, azimuth"
  _receiver.add_method("source/orientation", "if", [](lo_arg **argv, int)
    {
      _controller.set_source_orientation(argv[0]->i, Orientation(argv[1]->f));
      VERBOSE2("set source orientation: id = " << argv[0]->i << ", "
          << Orientation(argv[1]->f));
    }
  );

  // set source volume: "source/volume, if, id, volume"
  _receiver.add_method("source/volume", "if", [](lo_arg **argv, int)
    {
      _controller.set_source_gain(argv[0]->i, dB2linear(argv[1]->f));
      VERBOSE2("set source volume: id = " << argv[0]->i << ", volume = " <<
          dB2linear(argv[1]->f));
    }
  );

  // set source mute: "source/mute, iT, id, true"
  _receiver.add_method("source/mute", "iT", [](lo_arg **argv, int)
    {
      _controller.set_source_mute(argv[0]->i, true);
      VERBOSE2("set source mute: id = " << argv[0]->i << ", mute = true");
    }
  );

  // set source mute: "source/mute, iF, id, false"
  _receiver.add_method("source/mute", "iF", [](lo_arg **argv, int)
    {
      _controller.set_source_mute(argv[0]->i, false);
      VERBOSE2("set source mute: id = " << argv[0]->i << ", mute = false");
    }
  );

  // set source name: "source/name, is, id, name"
  _receiver.add_method("source/name", "is", [](lo_arg **argv, int)
    {
      _controller.set_source_name(argv[0]->i, argv[1]->s);
      VERBOSE2("set source name: id = " << argv[0]->i << ", name = " <<
          argv[1]->s);
    }
  );

  // set source file: "source/file, is, id, file"
  _receiver.add_method("source/file", "is", [](lo_arg **argv, int)
    {
      _controller.set_source_properties_file(argv[0]->i, argv[1]->s);
      VERBOSE2("set source properties file name: id = " << argv[0]->i << ",
          file = " << argv[1]->s);
    }
  );

  // set source model: "source/model, is, id, model"
  _receiver.add_method("source/model", "is", [](lo_arg **argv, int)
    {
      _controller.set_source_model(argv[0]->i, argv[1]->s);
      VERBOSE2("set source model: id = " << argv[0]->i << ",
          model = " << argv[1]->s);
    }
  );

  // set source port name: "source/port_name, is, id, port_name"
  _receiver.add_method("source/port_name", "is", [](lo_arg **argv, int)
    {
      _controller.set_source_port_name(argv[0]->i, argv[1]->s);
      VERBOSE2("set source port name: id = " << argv[0]->i << ",
          port = " << argv[1]->s);
    }
  );

  // create new source: "source/new, sssffff{T,F}{T,F}{T,F}, name, model,
  // port_name, x, y, orientation, volume, position_fixed, orientation_fixed,
  // muted"
  // create new source: "source/new, sssffffis{T,F}{T,F}{T,F}, name, model,
  // port_name, x, y, orientation, volume, channel, properties_file,
  // position_fixed, orientation_fixed, muted"
  _receiver.add_method("source/new", NULL, [](lo_arg **argv, int,
        lo::Message message)
    {
      std::string name(argv[0]->s);
      std::string model(argv[1]->s);
      std::string file_or_portname(argv[2]->s);
      float x(argv[3]->f);
      float y(argv[4]->f);
      float orientation(argv[5]->f);
      float volume(argv[6]->f);
      int channel = 0;
      std::string properties_file = "";
      bool position_fixed;
      bool orientation_fixed;
      bool muted;
      bool setup = false;
      switch (message.types())
      {
      case "sssffffTTT":
        position_fixed = true;
        orientation_fixed = true;
        muted = true;
        setup = true;
      break;
      case "sssffffTTF":
        position_fixed = true;
        orientation_fixed = true;
        muted = false;
        setup = true;
      break;
      case "sssffffTFF":
        position_fixed = true;
        orientation_fixed = false;
        muted = false;
        setup = true;
      break;
      case "sssffffFFF":
        position_fixed = false;
        orientation_fixed = false;
        muted = false;
        setup = true;
      break;
      case "sssffffTFT":
        position_fixed = true;
        orientation_fixed = false;
        muted = true;
        setup = true;
      break;
      case "sssffffFTF":
        position_fixed = false;
        orientation_fixed = true;
        muted = false;
        setup = true;
      break;
      case "sssffffFTT":
        position_fixed = false;
        orientation_fixed = true;
        muted = true;
        setup = true;
      break;
      case "sssffffFFT":
        position_fixed = false;
        orientation_fixed = false;
        muted = true;
        setup = true;
      break;
      case "sssffffisTTT":
        channel = argv[7]->i;
        properties_file = argv[8]->s;
        position_fixed = true;
        orientation_fixed = true;
        muted = true;
        setup = true;
      break;
      case "sssffffisTTF":
        channel = argv[7]->i;
        properties_file = argv[8]->s;
        position_fixed = true;
        orientation_fixed = true;
        muted = false;
        setup = true;
      break;
      case "sssffffisTFF":
        channel = argv[7]->i;
        properties_file = argv[8]->s;
        position_fixed = true;
        orientation_fixed = false;
        muted = false;
        setup = true;
      break;
      case "sssffffisFFF":
        channel = argv[7]->i;
        properties_file = argv[8]->s;
        position_fixed = false;
        orientation_fixed = false;
        muted = false;
        setup = true;
      break;
      case "sssffffisTFT":
        channel = argv[7]->i;
        properties_file = argv[8]->s;
        position_fixed = true;
        orientation_fixed = false;
        muted = true;
        setup = true;
      break;
      case "sssffffisFTF":
        channel = argv[7]->i;
        properties_file = argv[8]->s;
        position_fixed = false;
        orientation_fixed = true;
        muted = false;
        setup = true;
      break;
      case "sssffffisFTT":
        channel = argv[7]->i;
        properties_file = argv[8]->s;
        position_fixed = false;
        orientation_fixed = true;
        muted = true;
        setup = true;
      break;
      case "sssffffisFFT":
        channel = argv[7]->i;
        properties_file = argv[8]->s;
        position_fixed = false;
        orientation_fixed = false;
        muted = true;
        setup = true;
      break;
      }
      if (setup)
      {
        _controller.new_source(name, model, file_or_port_name, channel,
            Position(x, y), position_fixed, orientation, orientation_fixed,
            volume, muted, properties_file);
        VERBOSE2("Creating source with following properties:"
            "\nname: " << name <<
            "\nmodel: " << model <<
            "\nfile_or_port_name: " << file_or_port_name <<
            "\nchannel: " << channel <<
            "\nposition: " << Position(x, y) <<
            "\nposition_fixed: " << position_fixed <<
            "\norientation: " << orientation <<
            "\norientation_fixed: " << orientation_fixed <<
            "\nvolume (linear): " << volume <<
            "\nmuted: " << muted <<
            "\nproperties_file: " << properties_file <<
            "\n");
      }
    }
  );

  // delete source: "source/delete, i, id"
  // special case: i == 0 deletes all sources!
  _receiver.add_method("source/delete", "i", [](lo_arg **argv, int)
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

  // set reference position: "reference/position, ff, x, y"
  _receiver.add_method("reference/position", "ff", [](lo_arg **argv, int)
    {
      _controller.set_reference_position(Position(argv[0]->f, argv[1]->f);
      VERBOSE2("set reference position: " << Position(argv[0]->f, argv[1]->f));
    }
  );

  // set reference orientation: "reference/orientation, f, azimuth"
  _receiver.add_method("reference/position", "f", [](lo_arg **argv, int)
    {
      _controller.set_reference_orientation(Orientation(argv[0]->f));
      VERBOSE2("set reference orientation: " << Orientation(argv[0]->f));
    }
  );

  // set reference offset position: "reference_offset/position, ff, x, y"
  _receiver.add_method("reference_offset/position", "ff"
    , [](lo_arg **argv, int)
    {
      _controller.set_reference_offset_position(Position(argv[0]->f,
            argv[1]->f);
      VERBOSE2("set reference offset position: " << Position(argv[0]->f,
          argv[1]->f));
    }
  );

  // set reference offset orientation: "reference_offset/orientation, f,
  // azimuth"
  _receiver.add_method("reference_offset/position", "f"
    , [](lo_arg **argv, int)
    {
      _controller.set_reference_offset_orientation(Orientation(argv[0]->f));
      VERBOSE2("set reference offset orientation: " <<
          Orientation(argv[0]->f));
    }
  );

  // save scene to file: "scene/save, s, file"
  _receiver.add_method("scene/save", "s"
    , [](lo_arg **argv, int)
    {
      _controller.save_scene_as_XML(argv[0]->s);
      VERBOSE2("saving theme as: " << argv[0]->s);
    }
  );

  // load scene from file: "scene/load, s, file"
  _receiver.add_method("scene/load", "s"
    , [](lo_arg **argv, int)
    {
      _controller.load_scene(argv[0]->s);
      VERBOSE2("loading scene: " << argv[0]->s);
    }
  );

  // set master volume: "scene/volume, f, volume"
  _receiver.add_method("scene/volume", "f"
    , [](lo_arg **argv, int)
    {
      _controller.set_master_volume(dB2linear(argv[0]->f));
      VERBOSE2("set master volume: " << dB2linear(argv[0]->f) << " dB"); }
  );

  // clear scene: "scene/clear"
  _receiver.add_method("scene/load", NULL
    , [](lo_arg **argv, int)
    {
      _controller.delete_all_sources();
      VERBOSE2("clearing scene.");
    }
  );

  // set processing state: "state/processing, T, true"
  _receiver.add_method("state/processing", "T"
    , [](lo_arg **argv, int)
    {
      _controller.start_processing();
      VERBOSE2("start processing.");
    }
  );

  // set processing state: "state/processing, F, false"
  _receiver.add_method("state/processing", "F"
    , [](lo_arg **argv, int)
    {
      _controller.stop_processing();
      VERBOSE2("stop processing.");
    }
  );

  // set transport state: "state/transport, T, true"
  _receiver.add_method("state/transport", "T"
    , [](lo_arg **argv, int)
    {
      _controller.transport_start();
      VERBOSE2("start transport.");
    }
  );

  // set transport state: "state/transport, F, false"
  _receiver.add_method("state/transport", "F"
    , [](lo_arg **argv, int)
    {
      _controller.transport_stop();
      VERBOSE2("stop transport.");
    }
  );

  // rewind transport state: "state/transport/rewind"
  _receiver.add_method("state/transport/rewind", NULL
    , [](lo_arg **argv, int)
    {
      _controller.transport_locate(0);
      VERBOSE2("rewind transport.");
    }
  );

  // seek transport state: "state/transport/seek, s, time"
  _receiver.add_method("state/transport/seek", "s"
    , [](lo_arg **argv, int)
    {
      float time;
      if(string2time(argv[0]->s, time))
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

  // reset tracker: "tracker/reset"
  _receiver.add_method("tracker/reset", NULL
    , [](lo_arg **argv, int)
    {
      _controller.calibrate_client();
      VERBOSE2("calibrate tracker.");
    }
  );


}
