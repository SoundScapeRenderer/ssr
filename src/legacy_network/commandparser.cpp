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

#include "ssr_global.h" // for SSR_ERROR()
#include "commandparser.h"
#include "api.h"
#include "xmlparser.h"
#include "legacy_position.h"
#include "legacy_orientation.h"

#include "apf/stringtools.h"
#include "apf/math.h"  // for dB2linear()

using namespace apf::str;
using ssr::legacy_network::CommandParser;

/** ctor.
 * @param publisher
 **/
CommandParser::CommandParser(api::Publisher& publisher)
  : _publisher(publisher)
{}

/** Parse a XML string and map to Controller.
 * @param cmd XML string.
 **/
void
CommandParser::parse_cmd(const std::string& cmd)
{
  XMLParser xp;
  XMLParser::doc_t doc1 = xp.load_string(cmd);
  if (!doc1)
  {
    SSR_ERROR("Unable to load string! (\"" << cmd << "\")");
    return;
  }

  XMLParser::xpath_t result1 = doc1->eval_xpath("/request");

  if (!result1)
  {
    SSR_ERROR("XPath: no result!");
    return;
  }
  XMLParser::Node node1 = result1->node(); // get first (and only?) node.

  auto control = _publisher.take_control();

  for (XMLParser::Node i = node1.child(); !!i; ++i)
  {
    if (i == "source")
    {
      bool new_source = false;
      if (!S2A (i.get_attribute("new"), new_source))
      {
        new_source = false;
      }

      unsigned int source_number = 0;
      if (!new_source)
      {
        if (!S2A(i.get_attribute("id"), source_number))
        {
          SSR_ERROR("No source ID specified!");
          return;
        }
      }
      auto id = control->get_source_id(source_number);
      if (id == "" && !new_source)
      {
        SSR_ERROR("Invalid source number: " << source_number);
        return;
      }

      Position position;
      Orientation orientation;
      bool fixed = false;

      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "position")
        {
          if (S2A(inner_loop.get_attribute("x"), position.x)
              && S2A(inner_loop.get_attribute("y"), position.y))
          {
            if (!new_source)
            {
              control->source_position(id, position);
              SSR_VERBOSE2("set source position: id = " << id << ", " << position);
            }
            else
            {
              // position is used later for control->new_source(...)
            }
          }
          else
          {
            position = Position();
          }

          if (S2A(inner_loop.get_attribute("fixed"), fixed))
          {
            if (!new_source)
            {
              control->source_fixed(id, fixed);
              SSR_VERBOSE2("set source position fixed: id = " << id << ", fixed = "
                  << A2S(fixed));
            }
          }
          else
          {
            fixed = false;
          }
        }
        else if (inner_loop == "orientation")
        {
          if (S2A(inner_loop.get_attribute("azimuth"), orientation.azimuth))
          {
            if (!new_source)
            {
              control->source_rotation(id, orientation);
              SSR_VERBOSE2("set source orientation: id = " << id << ", "
                  << orientation);
            }
            else
            {
              // orientation is used later for control->new_source(...)
            }
          }
          else
          {
            orientation = Orientation();
          }
        }
      }//for (XMLParser:...

      float volume;
      if (S2A(i.get_attribute("volume"),volume))
      {
        // volume is given in dB in the network messages
        volume = apf::math::dB2linear(volume);
        if (!new_source)
        {
          control->source_volume(id, volume);
          SSR_VERBOSE2("set source volume: id = " << id
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
          control->source_mute(id, muted);
          SSR_VERBOSE2("set source mute mode: id = " << id
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
        control->source_name(id,name);
        SSR_VERBOSE2("set source name: id = " << id << ", name = " << name);
      }

      std::string properties_file = i.get_attribute("properties-file");
      if (!properties_file.empty() && !new_source)
      {
        SSR_ERROR("Cannot set properties-file! This works only for new sources.");
      }

      std::string model = i.get_attribute("model");
      if (!new_source && model != "")
      {
        control->source_model(id,model);
        SSR_VERBOSE2("set source model: id = " << id << ", model = " << model);
      }
      else
      {
        model = "point";
      }

      std::string port_name = i.get_attribute("port");
      if (!port_name.empty() && !new_source)
      {
        SSR_ERROR("Cannot set port name! This works only for new sources.");
      }

      std::string file_or_port_name = port_name;

      std::string file_name = i.get_attribute("file");
      if (!file_name.empty() && !new_source)
      {
        SSR_ERROR("Cannot set file name! This works only for new sources.");
      }
      if (!file_name.empty() && !port_name.empty())
      {
        SSR_ERROR("Either file name or port name can be set, not both!");
      }
      if (new_source && file_name.empty() && port_name.empty())
      {
        SSR_ERROR("Either file name or port name must be specified!");
      }

      int channel = 0;
      if (S2A(i.get_attribute("channel"), channel))
      {
        assert(channel >= 0);

        if (file_name.empty())
        {
          SSR_ERROR("'channel' is only allowed if a file name is also specified!");
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
          SSR_ERROR("Either file name or port name must be specified!");
        }
        else
        {
          SSR_VERBOSE2("Creating source with following properties:"
              "\nname: " << name <<
              "\nmodel: " << model <<
              "\nfile_or_port_name: " << file_or_port_name <<
              "\nchannel: " << channel <<
              "\nposition: " << position <<
              "\norientation: " << orientation <<
              "\nfixed: " << A2S(fixed) <<
              "\nvolume (linear): " << volume <<
              "\nmuted: " << A2S(muted) <<
              "\nproperties_file: " << properties_file <<
              "\n");

          control->new_source(id, name, model, file_or_port_name, channel,
              position, orientation, fixed,
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
            control->reference_position(Position(x,y));
            SSR_VERBOSE2("set reference position: " << Position(x,y));
          }
          else SSR_ERROR("Invalid reference position!");
        }
        else if (inner_loop == "orientation")
        {
          float azimuth;
          if (S2A(inner_loop.get_attribute("azimuth"), azimuth))
          {
            control->reference_rotation(Orientation(azimuth));
            SSR_VERBOSE2("set reference orientation: " << Orientation(azimuth));
          }
          else SSR_ERROR("Invalid reference orientation!");
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
            control->reference_position_offset(Position(x,y));
            SSR_VERBOSE2("set reference offset position: " << Position(x,y));
          }
          else SSR_ERROR("Invalid reference offset position!");
        }
        else if (inner_loop == "orientation")
        {
          float azimuth;
          if (S2A(inner_loop.get_attribute("azimuth"), azimuth))
          {
            control->reference_rotation_offset(Orientation(azimuth));
            SSR_VERBOSE2("set reference offset orientation: " << Orientation(azimuth));
          }
          else SSR_ERROR("Invalid reference offset orientation!");
        }
      }
    } // if (reference_offset)
    else if (i == "delete")
    {
      for (XMLParser::Node inner_loop = i.child(); !!inner_loop; ++inner_loop)
      {
        if (inner_loop == "source")
        {
          unsigned int source_number;
          if (S2A(inner_loop.get_attribute("id"), source_number))
          {
            std::string id = control->get_source_id(source_number);
            if (id != "")
            {
              control->delete_source(id);
            }
            else
            {
              SSR_ERROR("Source ID doesn't exist: " << id);
            }
          }
          else SSR_ERROR("Cannot read source ID!");
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
        control->save_scene(save_scene);
      }

      // load scene

      std::string load_scene = i.get_attribute("load");
      if (load_scene != "")
      {
        control->load_scene(load_scene);
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
        control->master_volume(apf::math::dB2linear(volume));
        SSR_VERBOSE2("set master volume: " << volume << " dB");
      }
      else SSR_ERROR("Invalid Volume Setting! (\"" << volume_str << "\")");

      bool clear_scene;
      if (S2A(i.get_attribute("clear"), clear_scene) && clear_scene)
      {
        control->delete_all_sources();
      }
    }
    else if (i == "state")
    {
      // start/stop audio processing

      std::string processing= i.get_attribute("processing");
      if (processing == "start")
      {
        control->processing(true);
      }
      else if (processing == "stop")
      {
        control->processing(false);
      }
      else if (processing != "")
      {
        SSR_ERROR("Invalid value for \"processing\": " << processing);
      }

      // start/stop/rewind transport

      std::string transport = i.get_attribute("transport");
      if (transport == "start")
      {
        control->transport_start();
      }
      else if (transport == "stop")
      {
        control->transport_stop();
      }
      else if (transport == "rewind")
      {
        control->transport_locate_frames(0);
      }
      else if (transport != "")
      {
        SSR_ERROR("Invalid value for \"transport\": " << transport);
      }

      // jump to a certain time

      std::string seek = i.get_attribute("seek");

      // seek can be in format h:mm:ss.x or "xx.x h|min|s|ms" just in seconds
      // decimals are optional
      // multiple whitespace is allowed before and after.

      float time;
      if (string2time(seek, time))
      {
        control->transport_locate_seconds(time);
      }
      else if (seek != "")
      {
        SSR_ERROR("Couldn't get the time out of the \"seek\" attribute (\""
            << seek << "\").");
      }

      // reset tracker

      std::string tracker = i.get_attribute("tracker");
      if (tracker == "reset")
      {
        control->reset_tracker();
      }
      else if (tracker != "")
      {
        SSR_ERROR("Invalid value for \"tracker\": " << tracker);
      }
    }
  }
}

