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
/// CommandParser class (implementation).

#include <cassert>

#include "ssr_global.h" // for ERROR()
#include "commandparser.h"
#include "publisher.h"

#include "apf/stringtools.h"
#include "apf/math.h"  // for dB2linear()

using namespace apf::str;

/** ctor.
 * @param controller 
 **/
ssr::CommandParser::CommandParser(Publisher& controller)
  : _controller(controller)
{}

/// dtor.
ssr::CommandParser::~CommandParser() {}

/** Parse a XML string and map to Controller.
 * @param cmd XML string. 
 **/
void
ssr::CommandParser::parse_cmd(std::string& cmd)
{
  XMLParser xp;
  XMLParser::doc_t doc1 = xp.load_string(cmd);
  if (!doc1)
  {
    ERROR("Unable to load string! (\"" << cmd << "\")");
    return;
  }

  XMLParser::xpath_t result1 = doc1->eval_xpath("/request");

  if (!result1)
  {
    ERROR("XPath: no result!");
    return;
  }
  XMLParser::Node node1 = result1->node(); // get first (and only?) node.

  for (XMLParser::Node i = node1.child(); !!i; ++i)
  {
    if (i == "source")
    {
      bool new_source = false;
      if (!S2A (i.get_attribute("new"), new_source))
      {
        new_source = false;
      }

      id_t id = 0; // ugly ...
      if (!new_source)
      {
        if (!S2A(i.get_attribute("id"),id))
        {
          ERROR("No source ID specified!");
          return;
        }
      }

      Position position;
      Orientation orientation;
      bool position_fixed = false, orientation_fixed = false;

      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "position")
        {
          if (S2A(inner_loop.get_attribute("x"), position.x)
              && S2A(inner_loop.get_attribute("y"), position.y))
          {
            if (!new_source)
            {
              _controller.set_source_position(id, position);
              VERBOSE2("set source position: id = " << id << ", " << position);
            }
            else
            {
              // position is used later for _controller.new_source(...)
            }
          }
          else
          {
            position = Position();
          }

          if (S2A(inner_loop.get_attribute("fixed"), position_fixed))
          {
            if (!new_source)
            {
              _controller.set_source_position_fixed(id, position_fixed);
              VERBOSE2("set source position fixed: id = " << id << ", fixed = "
                  << A2S(position_fixed));
            }
          }
          else
          {
            position_fixed = false;
          }
        }
        else if (inner_loop == "orientation")
        {
          if (S2A(inner_loop.get_attribute("azimuth"), orientation.azimuth))
          {
            if (!new_source)
            {
              _controller.set_source_orientation(id, orientation);
              VERBOSE2("set source orientation: id = " << id << ", "
                  << orientation);
            }
            else
            {
              // orientation is used later for _controller.new_source(...)
            }
          }
          else
          {
            orientation = Orientation();
          }
          // source orientation_fixed is not yet implemented!
          /*
          if (S2A(inner_loop.get_attribute("fixed"),
          orientation_fixed))
          {
          if (!new_source)
          {
          _controller.set_source_orientation_fix(id, orientation_fixed);
          VERBOSE2("set source orientation fixed");
          }
          }
          else
          {
          orientation_fixed = false;
          }
          */
        }
      }//for (XMLParser:...

      float volume;
      if (S2A(i.get_attribute("volume"),volume))
      {
        // volume is given in dB in the network messages
        volume = apf::math::dB2linear(volume);
        if (!new_source)
        {
          _controller.set_source_gain(id, volume);
          VERBOSE2("set source volume: id = " << id
              << ", volume (linear) = " << volume);
        }
      }
      else
      {
        volume = 1.0; // linear
      }

      bool muted;
      if (S2A(i.get_attribute("mute"), muted))
      {
        if (!new_source)
        {
          _controller.set_source_mute(id, muted);
          VERBOSE2("set source mute mode: id = " << id
              << ", mute = " << A2S(muted));
        }
      }
      else
      {
        muted = false;
      }
      std::string name = i.get_attribute("name");
      if (!name.empty() && !new_source)
      {
        _controller.set_source_name(id,name);
        VERBOSE2("set source name: id = " << id << ", name = " << name);
      }

      std::string properties_file = i.get_attribute("properties_file");
      if (!properties_file.empty() && !new_source)
      {
        _controller.set_source_properties_file(id,properties_file);
        VERBOSE2("set source properties file name: id = " << id
            << ", file = " << properties_file);
      }

      Source::model_t model = Source::model_t();
      if (S2A(i.get_attribute("model"),model))
      {
        if (!new_source)
        {
          _controller.set_source_model(id,model);
          VERBOSE2("set source model: id = " << id << ", model = " << model);
        }
      }
      else
      {
        model = Source::point;
      }

      std::string port_name = i.get_attribute("port");
      if (!port_name.empty() && !new_source)
      {
        _controller.set_source_port_name(id,port_name);
        VERBOSE2("set source port  name: id = " << id
            << ", port = " << port_name);
      }

      std::string file_or_port_name = port_name;

      std::string file_name = i.get_attribute("file");
      if (!file_name.empty() && !new_source)
      {
        ERROR("Cannot set file name! This works only for new sources.");
      }
      if (!file_name.empty() && !port_name.empty())
      {
        ERROR("Either file name or port name can be set, not both!");
      }
      if (new_source && file_name.empty() && port_name.empty())
      {
        ERROR("Either file name or port name must be specified!");
      }

      int channel = 0;
      if (S2A(i.get_attribute("channel"), channel))
      {
        assert(channel >= 0);

        if (file_name.empty())
        {
          ERROR("'channel' is only allowed if a file name is also specified!");
          channel = 0;
        }
        else
        {
          file_or_port_name = file_name;
        }
      }
      else if (!file_name.empty())
      {
        file_or_port_name = file_name;
        channel = 1;
      }

      if (new_source)
      {
        if (file_or_port_name == "")
        {
          ERROR("Either file name or port name must be specified!");
        }
        else
        {
          VERBOSE2("Creating source with following properties:"
              "\nname: " << name <<
              "\nmodel: " << model <<
              "\nfile_or_port_name: " << file_or_port_name <<
              "\nchannel: " << channel <<
              "\nposition: " << position <<
              "\nposition_fixed: " << A2S(position_fixed) <<
              "\norientation: " << orientation <<
              "\norientation_fixed: " << A2S(orientation_fixed) <<
              "\nvolume (linear): " << volume <<
              "\nmuted: " << A2S(muted) <<
              "\nproperties_file: " << properties_file <<
              "\n");

          _controller.new_source(name, model, file_or_port_name, channel,
              position, position_fixed, orientation, orientation_fixed,
              volume, muted, properties_file);
        }
      }

    } // if (source)
    else if (i == "reference")
    {
      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "position")
        {
          float x, y;
          if (S2A(inner_loop.get_attribute("x"), x)
              && S2A(inner_loop.get_attribute("y"), y))
          {
            _controller.set_reference_position(Position(x,y));
            VERBOSE2("set reference position: " << Position(x,y));
          }
          else ERROR("Invalid reference position!");
        }
        else if (inner_loop == "orientation")
        {
          float azimuth;
          if (S2A(inner_loop.get_attribute("azimuth"), azimuth))
          {
            _controller.set_reference_orientation(Orientation(azimuth));
            VERBOSE2("set reference orientation: " << Orientation(azimuth));
          }
          else ERROR("Invalid reference orientation!");
        }
      }
    } // if (reference)
    else if (i == "reference_offset")
    {
      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "position")
        {
          float x, y;
          if (S2A(inner_loop.get_attribute("x"), x)
              && S2A(inner_loop.get_attribute("y"), y))
          {
            _controller.set_reference_offset_position(Position(x,y));
            VERBOSE2("set reference offset position: " << Position(x,y));
          }
          else ERROR("Invalid reference offset position!");
        }
        else if (inner_loop == "orientation")
        {
          float azimuth;
          if (S2A(inner_loop.get_attribute("azimuth"), azimuth))
          {
            _controller.set_reference_offset_orientation(Orientation(azimuth));
            VERBOSE2("set reference offset orientation: " << Orientation(azimuth));
          }
          else ERROR("Invalid reference offset orientation!");
        }
      }
    } // if (reference_offset)
    else if (i == "delete")
    {
      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "source")
        {
          id_t id;
          if (S2A(inner_loop.get_attribute("id"),id))
          {
            // if (id == 0) {} //_controller.delete_all_sources();
            // else {}//_controller.delete_source(id);
            _controller.delete_source(id);
          }
          else ERROR("Cannot read source ID!");
        }
        else {}
      }// for ()
    }// else if (delete)
    else if (i== "scene")
    {
      // if both save and load are requested, first save, then load.
      // TODO: or make save and load exclusive? That's maybe better ...

      // save scene

      std::string save_scene = i.get_attribute("save");
      if (save_scene != "")
      {
        _controller.save_scene_as_XML(save_scene);
      }

      // load scene

      std::string load_scene = i.get_attribute("load");
      if (load_scene != "")
      {
        _controller.load_scene(load_scene);
      }

      std::string volume_str = i.get_attribute("volume");
      float volume;
      if (volume_str == "")
      {
        // do nothing
      }
      else if (S2A(volume_str, volume))
      {
        // volume is given in dB in the network messages
        _controller.set_master_volume(apf::math::dB2linear(volume));
        VERBOSE2("set master volume: " << volume << " dB");
      }
      else ERROR("Invalid Volume Setting! (\"" << volume_str << "\")");

      bool clear_scene;
      if (S2A(i.get_attribute("clear"), clear_scene) && clear_scene)
      {
        _controller.delete_all_sources();
      }
    }
    else if (i == "state")
    {
      // start/stop audio processing

      std::string processing= i.get_attribute("processing");
      if (processing == "start")
      {
        _controller.start_processing();
      }
      else if (processing == "stop")
      {
        _controller.stop_processing();
      }
      else if (processing != "")
      {
        ERROR("Invalid value for \"processing\": " << processing);
      }

      // start/stop/rewind transport

      std::string transport = i.get_attribute("transport");
      if (transport == "start")
      {
        _controller.transport_start();
      }
      else if (transport == "stop")
      {
        _controller.transport_stop();
      }
      else if (transport == "rewind")
      {
        _controller.transport_locate(0);
      }
      else if (transport != "")
      {
        ERROR("Invalid value for \"transport\": " << transport);
      }

      // jump to a certain time

      std::string seek = i.get_attribute("seek");

      // seek can be in format h:mm:ss.x or "xx.x h|min|s|ms" just in seconds
      // decimals are optional
      // multiple whitespace is allowed before and after.

      float time;
      if (string2time(seek, time))
      {
        _controller.transport_locate(time);
      }
      else if (seek != "")
      {
        ERROR("Couldn't get the time out of the \"seek\" attribute (\""
            << seek << "\").");
      }

      // reset tracker

      std::string tracker = i.get_attribute("tracker");
      if (tracker == "reset")
      {
        _controller.calibrate_client();
      }
      else if (tracker != "")
      {
        ERROR("Invalid value for \"tracker\": " << tracker);
      }
    }
  }
}

#if 0
ssr::CommandParser::parse_string2(std::string cmd)
{
  XMLParser xp;
  XMLParser::doc_t doc1 = xp.load_string(data);
  if (!doc1)
  {
    std::cerr << "client_parser_thread: Unable to load string!" << std::endl;
    return;
  }
  XMLParser::xpath_t result1 = doc1->eval_xpath("/update");

  if (!result1)
  {
    std::cerr << "XPath: no result!" << std::endl;
    continue ;
  }
  XMLParser::Node node1 = result1->node(); // get first (and only?) node.

  // all loudspeaker information is written to this temporary container and
  // transmitted in the end of the function
  Loudspeaker::container_t loudspeakers;

  for (XMLParser::Node i = node1.child(); !!i; ++i)
  {
    if (i == "source")
    {
      id_t id;
      if (!S2A (i.get_attribute("id"),id))
      {
        ERROR("No source ID given!");
        return;
      }
      _scene.new_source(id);

      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "position")
        {
          float x, y;
          if (S2A(inner_loop.get_attribute("x"), x)
              && S2A(inner_loop.get_attribute("y"), y))
          {
            _scene.set_source_position(id, Position(x,y));
            VERBOSE2("set source position");
          }
          else
          {
            std::cerr << "Invalid position!" << std::endl;
          }
        }
        else if (inner_loop == "orientation")
        {
          float azimuth;
          if (S2A(inner_loop.get_attribute("azimuth"), azimuth))
          {
            _scene.set_source_orientation(id,Orientation(azimuth));
            VERBOSE2("set source orientation");
          }
          else
          {
            std::cerr << "Invalid source orientation!" << std::endl;
          }
        }
      }//eof for

      float volume;
      if (S2A(i.get_attribute("volume"),volume))
      {
        // volume is given in dB in the network messages
        _scene.set_source_gain(id, apf::math::dB2linear(volume));
        VERBOSE2("set source volume");
      }

      bool muted;
      if (S2A(i.get_attribute("mute"), muted))
      {
        _scene.set_source_mute(id, muted);
        VERBOSE2("set source mute mode");
      }

      std::string name = i.get_attribute("name");
      if (name.empty() == false)
      {
        _scene.set_source_name(id,name);
        VERBOSE2("set source name");
      }

      std::string properties_file = i.get_attribute("brirs");
      if (properties_file.empty() == false)
      {
        _scene.set_source_properties_file(id,properties_file);
        VERBOSE2("set source BRIR file name");
      }

      Source::model_t model;
      if (S2A(i.get_attribute("model"),model))
      {

        _scene.set_source_model(id,model);
        VERBOSE2("set source model");
      }

      std::string port_name = i.get_attribute("port_name");
      if (!port_name.empty())
      {
        _scene.set_source_port_name(id,port_name);
        VERBOSE2("Set source port name");
      }

      float level;
      if (S2A(i.get_attribute("level"),level))
      {
        _scene.set_source_signal_level(id,level);
      }

    }//eof if (source)
    else if (i == "reference")
    {
      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "position")
        {
          float x, y;
          if (S2A(inner_loop.get_attribute("x"), x)
              && S2A(inner_loop.get_attribute("y"), y))
          {
            _scene.set_reference_position(Position(x,y));
            VERBOSE2("Set Reference Position");
          }
          else
          {
            std::cerr << "Invalid reference position!" << std::endl;
          }
        }
        else if (inner_loop == "orientation")
        {
          float azimuth;
          if (S2A(inner_loop.get_attribute("azimuth"), azimuth))
          {
            _scene.set_reference_orientation(Orientation(azimuth));
            VERBOSE2("Set Reference Orientation");
          }
          else
            std::cerr << "Invalid reference orientation!" << std::endl;
        }
      }// for
    }// if (reference)
    else if (i == "loudspeaker")
    {
      Loudspeaker loudspeaker;
      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "position")
        {
          float x, y;
          if (S2A(inner_loop.get_attribute("x"), x)
              && S2A(inner_loop.get_attribute("y"), y))
            loudspeaker.position = Position(x,y);
          else
            std::cerr << "Invalid loudspeaker position!" << std::endl;
        }
        else if (inner_loop == "orientation")
        {
          float azimuth;
          if (S2A(inner_loop.get_attribute("azimuth"), azimuth))
            loudspeaker.orientation = Orientation(azimuth);
          else
            std::cerr << "Invalid loudspeaker orientation!" << std::endl;
        }
      }//for
      Loudspeaker::model_t model;
      if (S2A(i.get_attribute("model"), model))
      {
        loudspeaker.model = model;
      }
      bool mute = false;
      if (S2A(i.get_attribute("mute"), mute))
        loudspeaker.mute = mute;
      loudspeakers.push_back(loudspeaker);
    }
    else if (i == "volume")
    {
      float volume;
      if (S2A(get_content(i),volume))
      {
        // volume is given in dB in the network messages
        _scene.set_master_volume(apf::math::dB2linear(volume));
        VERBOSE2("Set Master Volume");
      }
      else
        std::cerr << "Invalid Volume Setting!" << std::endl;
    } //if (volume)
    else if (i == "active")
    {
      Loudspeaker::container_t::size_type loudspeaker_id;
      if (S2A(i.get_attribute("loudspeaker"),loudspeaker_id))
      {
        std::set<id_t> active_source_ids;
        id_t temp_id;
        std::istringstream iss (get_content(i));
        while (iss >> temp_id)
        {
          active_source_ids.insert(temp_id);
        }
        if (!active_source_ids.empty())
          _scene.set_active_sources(loudspeaker_id, active_source_ids);
      }
    }
    else if (i == "delete")
    {
      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "source")
        {
          id_t id;
          if (S2A(inner_loop.get_attribute("id"),id))
          {
            if (id == 0)
            {
              _scene.delete_all_sources();
              VERBOSE2("delete all sources");
            }
            else {
              _scene.delete_source(id);
              VERBOSE2("delete source with id " << A2S(id));
            }
          }
          else
            std::cerr << "Cannot read source ID!" << std::endl;
        }
        else {}
      }
    }//eof if (delete)
    else if (i == "cpu")
    {
      float load;
      if (S2A(i.get_attribute("load"),load))
        _scene.set_cpu_load(load);
    } //cpu
    else if (i == "master")
    {
      float level;
      if (S2A(i.get_attribute("level"),level))
        _scene.set_master_signal_level(level);
    }
  }
  // if there were any loudspeakers given, save them
  if (!loudspeakers.empty())
  {
    _scene.set_loudspeakers(loudspeakers);
  }
}

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
