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
/// Parent class for loudspeaker-based renderers.

#ifndef SSR_LOUDSPEAKERRENDERER_H
#define SSR_LOUDSPEAKERRENDERER_H

#include "rendererbase.h"
#include "loudspeaker.h"
#include "xmlparser.h"
#include "apf/parameter_map.h"

namespace ssr
{

template<typename Derived>
class LoudspeakerRenderer : public RendererBase<Derived>
{
  private:
    using _base = RendererBase<Derived>;
    using Node = XMLParser::Node;

  public:
    class Output : public _base::Output, public Loudspeaker
    {
      public:
        struct Params : _base::Output::Params, Loudspeaker {};

        // TODO: handle loudspeaker delays?

        Output(const Params& p) : _base::Output(p), Loudspeaker(p) {}
    };

    LoudspeakerRenderer(const apf::parameter_map& p)
      : _base(p)
      , _reproduction_setup(p.get("reproduction_setup", ""))
      , _xml_schema(p.get("xml_schema", ""))
      , _next_loudspeaker_channel(1)
    {
      this->_show_head = false;
    }

    void load_reproduction_setup();

    void get_loudspeakers(std::vector<Loudspeaker>& l) const;

  private:
    void _load_loudspeaker(const Node& node);
    void _load_linear_array(const Node& node);
    void _load_circular_array(const Node& node);
    void _set_connection(apf::parameter_map& p);

    std::unique_ptr<Position> _get_position(const Node& node);
    std::unique_ptr<Orientation> _get_orientation(const Node& node);
    std::unique_ptr<Orientation> _get_angle(const Node& node);

    const std::string _reproduction_setup;
    const std::string _xml_schema;

    int _next_loudspeaker_channel;
};

template<typename Derived>
void
LoudspeakerRenderer<Derived>::get_loudspeakers(std::vector<Loudspeaker>& l)
  const
{
  using out_list_t
    = typename _base::template rtlist_proxy<typename Derived::Output>;

  for (const auto& out: out_list_t(this->get_output_list()))
  {
    l.push_back(Loudspeaker(out));
  }
}

template<typename Derived>
void
LoudspeakerRenderer<Derived>::load_reproduction_setup()
{
  if (_reproduction_setup == "")
  {
    throw std::runtime_error("No reproduction_setup specified!");
  }

  // read the setup from XML file
  XMLParser xp; // load XML parser
  auto setup_file = xp.load_file(_reproduction_setup);
  if (!setup_file)
  {
    throw std::runtime_error("Unable to load reproduction setup file \""
        + _reproduction_setup + "\"!");
  }

  if (_xml_schema != "" && !setup_file->validate(_xml_schema))
  {
    throw std::runtime_error("Error validating \""
        + _reproduction_setup + "\" with schema file \"" + _xml_schema + "\"!");
  }

  auto xpath_result = setup_file->eval_xpath("//reproduction_setup/*");

  // one could use separate xpath expressions like
  // "//reproduction_setup/loudspeaker",
  // "//reproduction_setup/circular_array" and
  // "//reproduction_setup/linear_array",
  // but this would not preserve the order of the elements!

  if (!xpath_result)
  {
    throw std::runtime_error(
        "No <reproduction_setup> section found in XML file!");
  }

  // it is not necessary to use xmlFreeNode on "node", it should be freed in the
  // destructor of xpath_result (hopefully?)
  for (Node node; (node = xpath_result->node()); ++(*xpath_result))
  {
    if (node == "loudspeaker")
    {
      _load_loudspeaker(node);
    }
    else if (node == "linear_array")
    {
      _load_linear_array(node);
    }
    else if (node == "circular_array")
    {
      _load_circular_array(node);
    }
    else if (node == "skip")
    {
      int number = 1;

      if (!apf::str::S2A(node.get_attribute("number"), number) || (number < 1))
      {
        //VERBOSE("Couldn't get \"number\" attribute of \"skip\" element."
        //    " Using default value 1.");
        number = 1;
      }
      for (int i = 0; i < number; ++i)
      {
        ++_next_loudspeaker_channel;
      }
    }
    else
    {
      //WARNING("Ignored \"" << node << "\" element!");
    }
  }
  if (_next_loudspeaker_channel < 2)
  {
    throw std::runtime_error("No loudspeakers found in the file \""
        + _reproduction_setup + "\"!");
  }

  //VERBOSE("Loaded " << l.size() << " loudspeakers from '"
  //    << setup_file_name << "'.");
}

template<typename Derived>
void
LoudspeakerRenderer<Derived>::_set_connection(apf::parameter_map& p)
{
  const std::string prefix = this->params.get("system_output_prefix", "");
  if (prefix != "")
  {
    p.set("connect_to", prefix + apf::str::A2S(_next_loudspeaker_channel));
  }
  ++_next_loudspeaker_channel;
}

template<typename Derived>
void
LoudspeakerRenderer<Derived>::_load_loudspeaker(const Node& node)
{
  if (!node) return;

  std::unique_ptr<Position> position;
  std::unique_ptr<Orientation> orientation;

  position    = _get_position(node);
  orientation = _get_orientation(node);

  if (!position)
  {
    throw std::runtime_error(
        "No position found for the loudspeaker! Not loaded!");
    return;
  }
  else if (!orientation)
  {
    throw std::runtime_error(
        "No orientation found for the loudspeaker! Not loaded!");
    return;
  }

  std::string model_str = node.get_attribute("model");
  Loudspeaker::model_t model;
  if      (model_str == "normal")    model = Loudspeaker::normal;
  else if (model_str == "subwoofer") model = Loudspeaker::subwoofer;
  else                               model = Loudspeaker::normal;

  float delay = !node ? 0.0f
    : apf::str::S2RV(node.get_attribute("delay"), 0.0f);

  // check validity
  if (delay < 0)
  {
    //ERROR("Loudspeaker delay can not be negative. I'll ignore it");
    delay = 0.0f;
  }

  float weight = !node ? 1.0f
    : apf::str::S2RV(node.get_attribute("weight"), 1.0f);

  typename Output::Params params;
  params.position = *position;
  params.orientation = *orientation;
  params.model = model;
  params.weight = weight;
  params.delay = delay;
  _set_connection(params);
  this->add(params);
}

template<typename Derived>
void
LoudspeakerRenderer<Derived>::_load_linear_array(const Node& node)
{
  if (!node) return;

  int number;
  if (!apf::str::S2A(node.get_attribute("number"), number))
  {
    throw std::runtime_error(
        "Invalid number of loudspeakers! Array not loaded.");
    return;
  }

  if (number < 2)
  {
    // This is also checked in the Schema validation.
    throw std::runtime_error(
        "Number of loudspeakers in an array must be at least 2!");
    return;
  }

  std::unique_ptr<Position> first_pos, second_pos, last_pos;
  std::unique_ptr<Orientation> first_dir, second_dir, last_dir;

  // we search the first occurence of each element, but
  // the XML Schema should assure that each element comes only once.
  first_pos  = _get_position   (node.child("first"));
  first_dir  = _get_orientation(node.child("first"));
  second_pos = _get_position   (node.child("second"));
  second_dir = _get_orientation(node.child("second"));
  last_pos   = _get_position   (node.child("last"));
  last_dir   = _get_orientation(node.child("last"));

  if (!first_pos)
  {
    throw std::runtime_error(
        "No position found for first loudspeaker! Array not loaded!");
    return;
  }
  if (!first_dir)
  {
    throw std::runtime_error(
        "No orientation found for first loudspeaker! Array not loaded!");
    return;
  }
  const DirectionalPoint first(*first_pos, *first_dir);
  auto increment = DirectionalPoint();

  if (second_pos && !last_pos)
  {
    // if no orientation was given for "second", it gets the same as "first"
    if (!second_dir)
    {
      second_dir.reset(new Orientation(*first_dir));
    }
    const auto second = DirectionalPoint(*second_pos, *second_dir);
    increment = second - first;
  }
  else if (last_pos && !second_pos)
  {
    // if no orientation was given for "last", it gets the same as "first"
    if (!last_dir)
    {
      last_dir.reset(new Orientation(*first_dir));
    }
    const auto last = DirectionalPoint(*last_pos, *last_dir);
    increment = (last - first) / (number - 1);
  }
  else
  {
    throw std::runtime_error(
        "Either \"second\" or \"last\" has to be present! (But not both!) "
        "Array not loaded.");
    return;
  }

  DirectionalPoint current = first;
  for (int i = 0; i < number; ++i)
  {
    // TODO: get loudspeaker type!

    typename Output::Params params;
    params.position = current.position;
    params.orientation = current.orientation;
    _set_connection(params);
    this->add(params);

    current += increment;
  }
}

template<typename Derived>
void
LoudspeakerRenderer<Derived>::_load_circular_array(const Node& node)
{
  if (!node) return;

  int number;
  std::unique_ptr<Position> first_pos, center_pos;
  std::unique_ptr<Orientation> first_dir, second_dir, last_dir;

  if (!apf::str::S2A(node.get_attribute("number"), number) || (number < 2))
  {
    throw std::runtime_error(
        "Invalid number of loudspeakers! Array not loaded.");
    return;
  }

  center_pos = _get_position   (node.child("center"));
  first_pos  = _get_position   (node.child("first"));
  first_dir  = _get_orientation(node.child("first"));
  second_dir = _get_angle      (node.child("second"));
  last_dir   = _get_angle      (node.child("last"));

  if (!first_pos)
  {
    throw std::runtime_error(
        "No position found for first loudspeaker! Array not loaded!");
    return;
  }
  if (!first_dir)
  {
    throw std::runtime_error(
        "No orientation found for first loudspeaker! Array not loaded!");
    return;
  }

  // if no center is given, it is set to the origin
  if (!center_pos)
  {
    center_pos.reset(new Position(0,0));
  }

  // WARNING: The center is not supposed to have an orientation!
  auto radius = DirectionalPoint(*first_pos - *center_pos, *first_dir);
  auto center = DirectionalPoint(*center_pos, Orientation());

  float angle_increment = 360.0f / number; // full circle
  // division by zero not possible, as number >= 2 (see above)

  if (second_dir)
  {
    angle_increment = second_dir->azimuth;
  }
  else if (last_dir)
  {
    angle_increment = last_dir->azimuth / (number - 1);
    // division by zero not possible, as number >= 2 (see above)
  }

  for (int i = 0; i < number; ++i)
  {
    // Loudspeakers are ordered in mathematic positive direction, i.e.
    // counterclockwise (seen from above).

    // TODO: get loudspeaker type!

    auto point = radius;
    point.rotate(i * angle_increment);
    point += center;
    typename Output::Params params;
    params.position = point.position;
    params.orientation = point.orientation;
    _set_connection(params);
    this->add(params);
  }
}

template<typename Derived>
std::unique_ptr<Position>
LoudspeakerRenderer<Derived>::_get_position(const Node& node)
{
  std::unique_ptr<Position> temp; // temp = NULL
  if (!node) return temp; // return NULL

  for (Node i = node.child(); !!i; ++i)
  {
    if (i == "position")
    {
      float x, y;

      // if read operation successful
      if (apf::str::S2A(i.get_attribute("x"), x)
          && apf::str::S2A(i.get_attribute("y"), y))
      {
        temp.reset(new Position(x, y));
        return temp; // return sucessfully
      }
      else
      {
        //ERROR("Invalid position!");
        return temp; // return NULL
      } // if read operation successful

    } // if (i == "position")
  }
  return temp; // return NULL
}

template<typename Derived>
std::unique_ptr<Orientation>
LoudspeakerRenderer<Derived>::_get_orientation(const Node& node)
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
        //ERROR("Invalid orientation!");
        return temp; // return NULL
      }
    }
  }
  return temp;       // return NULL
}

template<typename Derived>
std::unique_ptr<Orientation>
LoudspeakerRenderer<Derived>::_get_angle(const Node& node)
{
  std::unique_ptr<Orientation> temp; // temp = NULL
  if (!node) return temp;          // return NULL
  for (Node i = node.child(); !!i; ++i)
  {
    if (i == "angle")
    {
      float azimuth;
      if (apf::str::S2A(i.get_attribute("azimuth"), azimuth))
      {
        temp.reset(new Orientation(azimuth));
        return temp; // return sucessfully
      }
      else
      {
        //ERROR("Invalid angle!");
        return temp; // return NULL
      }
    }
  }
  return temp;       // return NULL
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
