#include "oscreiver.h"
#include "oschandler.h"
#include "publisher.h"
#include "apf/stringtools.h"
#include "apf/math.h"

using namespace apf::str;

ssr::OscReceiver::OscReceiver(Publisher& controller, int port)
  : _controller(controller)
    //TODO: add error handler here
  , _receiver(port)
{}

ssr::OscReceiver::~OscReceiver()
{
  _receiver.stop();
}


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

//  _controller.subscribe(&_subscriber);

}

lo::Address server_address(OscHandler& handler)
{
  return handler.server_address(handler->_osc_sender);
}

/** Add callback handlers for OSC messages received from clients.
 *
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void add_client_to_server_methods()
{
//TODO: implement!
}

/** Add callback handlers for OSC messages received from a server.
 *
 * This function uses C++11 lambda functions to define the behavior for every
 * callback, that interface with the Publisher's functionality.
 */
void add_server_to_client_methods()
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
      _controller.set_source_position_fixed(argv[0]->i, argv[1]->T);
      VERBOSE2("set source position fixed: id = " << argv[0]->i << ", fixed = "
          << argv[1]->T);
    }
  );

  // set source fixed: "source/position_fixed, iF, id, false"
  _receiver.add_method("source/position_fixed", "iF", [](lo_arg **argv, int)
    {
      _controller.set_source_position_fixed(argv[0]->i, argv[1]->F);
      VERBOSE2("set source position fixed: id = " << argv[0]->i << ", fixed = "
          << argv[1]->F);
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
      _controller.set_source_mute(argv[0]->i, argv[1]->T);
      VERBOSE2("set source mute: id = " << argv[0]->i << ", mute = " <<
          dB2linear(argv[1]->T));
    }
  );

  // set source mute: "source/mute, iF, id, false"
  _receiver.add_method("source/mute", "iF", [](lo_arg **argv, int)
    {
      _controller.set_source_mute(argv[0]->i, argv[1]->F);
      VERBOSE2("set source mute: id = " << argv[0]->i << ", mute = " <<
          dB2linear(argv[1]->F));
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

  //FIXME: spezialize with TF types

  // create new source: "source/new, sssffbfbfb, name, model, port_name, x, y,
  // position_fixed, orientation, orientation_fixed, volume, muted"
  _receiver.add_method("source/new", "sssffbfbfb", [](lo_arg **argv, int)
    {
      _controller.new_source(argv[0]->s, argv[1]->s, argv[2]->s, 0,
          Position(argv[3]->f, argv[4]->f), argv[5]->b, argv[6]->f, argv[7]->b,
          argv[8]->f, argv[9]->b, "");
      VERBOSE2("Creating source with following properties:"
          "\nname: " << argv[0]->s <<
          "\nmodel: " << argv[1]->s <<
          "\nfile_or_port_name: " << argv[2]->s <<
          "\nchannel: 0" <<
          "\nposition: " << Position(argv[3]->f, argv[4]->f) <<
          "\nposition_fixed: " << argv[5]->b <<
          "\norientation: " << argv[6]->f <<
          "\norientation_fixed: " << argv[7]->b <<
          "\nvolume (linear): " << argv[8]->f <<
          "\nmuted: " << argv[9]->b <<
          "\nproperties_file: " <<
          "\n");
    }
  );

  // create new source: "source/new, sssiffbfbfbs, name, model, file, channel,
  // x, y, position_fixed, orientation, orientation_fixed, volume, muted,
  // properties_file"
  _receiver.add_method("source/new", "sssiffbfbfbs", [](lo_arg **argv, int)
    {
      _controller.new_source(argv[0]->s, argv[1]->s, argv[2]->s, 0,
          Position(argv[3]->f, argv[4]->f), argv[5]->b, argv[6]->f, argv[7]->b,
          argv[8]->f, argv[9]->b, "");
      VERBOSE2("Creating source with following properties:"
          "\nname: " << argv[0]->s <<
          "\nmodel: " << argv[1]->s <<
          "\nfile_or_port_name: " << argv[2]->s <<
          "\nchannel: " << argv[3]->i <<
          "\nposition: " << Position(argv[4]->f, argv[5]->f) <<
          "\nposition_fixed: " << argv[6]->b <<
          "\norientation: " << argv[7]->f <<
          "\norientation_fixed: " << argv[8]->b <<
          "\nvolume (linear): " << argv[9]->f <<
          "\nmuted: " << argv[10]->b <<
          "\nproperties_file: " << argv[11]->s <<
          "\n");
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
