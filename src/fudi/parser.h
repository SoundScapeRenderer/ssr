/******************************************************************************
 * Copyright © 2019 SSR Contributors                                          *
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
/// A parser for SSR-specific FUDI messages.

#ifndef SSR_FUDI_PARSER_H
#define SSR_FUDI_PARSER_H

#include <cstdlib>  // for std::strtof(), std::strtoul()
#include <functional>  // for std::function
#include <variant>

#include "api.h"
#include "ssr_global.h"  // for SSR_ERROR
#include "geometry.h"
#include "subscriber.h"

namespace ssr
{

namespace fudi
{

enum class Match { yes, no, incomplete };

constexpr bool is_whitespace(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

constexpr size_t consume_whitespace(std::string_view& input)
{
  size_t consumed = 0;
  while (!input.empty())
  {
    if (is_whitespace(input.front()))
    {
      input.remove_prefix(1);
      ++consumed;
    }
    else
    {
      break;
    }
  }
  return consumed;
}

class Parser
{
public:
  explicit Parser(api::Publisher& controller, Subscriber& subscriber);

  void parse(std::string_view& input)
  {
    consume_whitespace(input);
    while (!input.empty())
    {
      auto result = _parse_message(input);
      if (result == Match::incomplete)
      {
        break;
      }
      else if (result == Match::no)
      {
        size_t idx = 0;
        while (idx < input.size())
        {
          idx = input.find_first_of("\\;", idx);
          if (idx == std::string_view::npos)
          {
            // New data will be requested until there is a semicolon
            return;
          }
          // Jump over escaped character
          else if (input[idx] == '\\')
          {
            if (input.size() > idx + 2)
            {
              idx += 2;
              continue;
            }
            else
            {
              return;
            }
          }
          else if (idx == 0)
          {
            assert(input[idx] == ';');
            // Silently ignore stray semicolon
            input.remove_prefix(1);
          }
          else
          {
            assert(input[idx] == ';');
            auto invalid = input.substr(0, idx + 1);
            SSR_ERROR("Invalid data: \"" << invalid << "\"");
            input.remove_prefix(invalid.size());
          }
          break;
        }
      }
      else
      {
        assert(result == Match::yes);
      }
      consume_whitespace(input);
    }
  }

private:
  std::function<Match(std::string_view&)> _parse_message;
};

namespace internal
{

constexpr auto string(std::string_view name)
{
  return [name](std::string_view& input) {
    auto size = std::min(name.size(), input.size());
    if (input.substr(0, size) == name.substr(0, size))
    {
      if (size < name.size())
      {
        return Match::incomplete;
      }
      input.remove_prefix(size);
      return Match::yes;
    }
    return Match::no;
  };
}

template<typename F>
constexpr auto maybe(F f)
{
  return [f](std::string_view& input) {
    if (auto result = f(input); result == Match::no)
    {
      return Match::yes;
    }
    else
    {
      return result;
    }
  };
}

constexpr Match whitespace(std::string_view& input)
{
  return consume_whitespace(input) ? Match::yes : Match::no;
}

template<typename F>
constexpr auto semicolon(F f)
{
  return [f](std::string_view& input) {
    if (input[0] != ';')
    {
      return Match::no;
    }
    input.remove_prefix(1);
    f();
    return Match::yes;
  };
}

template<typename First, typename... Rest>
constexpr auto sequence(First first, Rest... rest)
{
  return [=](std::string_view& input) {
    auto temp = input;
    auto result = first(temp);
    if constexpr (sizeof...(Rest) > 0)
    {
      if (result != Match::yes) { return result; }
      if (temp.empty()) { return Match::incomplete; }
      result = sequence(rest...)(temp);
    }
    if (result == Match::yes)
    {
      input = temp;
    }
    return result;
  };
}

template<typename First, typename... Rest>
constexpr auto choice(First first, Rest... rest)
{
  return [=](std::string_view& input) {
    auto result = first(input);
    if constexpr (sizeof...(Rest) > 0)
    {
      if (result != Match::no) { return result; }
      result = choice(rest...)(input);
    }
    return result;
  };
}

constexpr auto parse_string(std::string& value)
{
  return [&value](std::string_view& input) {
    size_t idx = 0;
    while (idx < input.size())
    {
      auto previous_idx = idx;
      idx = input.find_first_of(" \n\t\r\\;", idx);
      if (idx == std::string_view::npos)
      {
        return Match::incomplete;
      }
      if (input[idx] == '\\')
      {
        if (idx + 2 >= input.size())
        {
          return Match::incomplete;
        }
        value += input.substr(previous_idx, idx - previous_idx);
        ++idx;
        auto c = input[idx];
        if (c == ' '
            || c == '\n'
            || c == '\t'
            || c == '\r'
            || c == '\\'
            || c == ';')
        {
          value += c;
        }
        else
        {
          value += '\\';
          value += c;
        }
        ++idx;
        continue;
      }
      else
      {
        if (idx == 0)
        {
          return Match::no;
        }
        value += input.substr(previous_idx, idx - previous_idx);
        break;
      }
    }
    assert(idx > 0);
    input.remove_prefix(idx);
    return Match::yes;
  };
}

constexpr auto id_or_number(std::variant<std::string, unsigned int>& value)
{
  return [&value](std::string_view& input) {
    char* str_end;
    unsigned int number = std::strtoul(input.data(), &str_end, 10);
    if (str_end == input.data())
    {
      std::string temp;
      auto result = parse_string(temp)(input);
      if (result == Match::yes)
      {
        value = std::move(temp);
      }
      return result;
    }
    input.remove_prefix(str_end - input.data());
    value = number;
    return Match::yes;
  };
}

constexpr auto parse_bool(bool& value)
{
  return [&value](std::string_view& input) {
    assert(!input.empty());
    if (input[0] == '1')
    {
      value = true;
    }
    else if (input[0] == '0')
    {
      value = false;
    }
    else
    {
      SSR_ERROR("Error parsing boolean value");
      return Match::no;
    }
    input.remove_prefix(1);
    return Match::yes;
  };
}

constexpr auto parse_float(float& value)
{
  return [&value](std::string_view& input) {
    auto temp = input;
    // Request more data on incomplete packets (truncated numbers)
    if (temp.size() == 1 && (temp[0] == '-' || temp[0] == '.'))
    {
      return Match::incomplete;
    }
    if (temp.size() == 2 && temp[0] == '-' && temp[1] == '.')
    {
      return Match::incomplete;
    }
    char* str_end;
    // TODO: use std::from_chars once it is widely available
    value = std::strtof(temp.data(), &str_end);
    if (str_end == temp.data())
    {
      return Match::no;
    }
    temp.remove_prefix(str_end - temp.data());
    if (temp.empty())
    {
      return Match::incomplete;
    }
    // Request more data on truncated exponentials
    if (temp.size() == 1
        && (temp[0] == 'e' || temp[0] == 'E'))
    {
      return Match::incomplete;
    }
    if (temp.size() == 2
        && (temp[0] == 'e' || temp[0] == 'E')
        && temp[1] == '-')
    {
      return Match::incomplete;
    }
    input = temp;
    return Match::yes;
  };
}

template<typename F>
constexpr auto pos(F f)
{
  return [f](std::string_view& input) {
    Pos value{};
    return sequence(
      string("pos"),
      whitespace,
      parse_float(value.x),
      whitespace,
      parse_float(value.y),
      maybe(sequence(
        whitespace,
        parse_float(value.z))),
      maybe(whitespace),
      semicolon([&]() {
        f(value);
      }))(input);
  };
}

template<typename F>
constexpr auto rot(F f)
{
  return [f](std::string_view& input) {
    Rot value{};
    float azimuth{}, elevation{}, roll{};
    return choice(
      sequence(
        string("rot"),
        whitespace,
        parse_float(azimuth),
        maybe(sequence(
          whitespace,
          parse_float(elevation),
          maybe(sequence(
            whitespace,
            parse_float(roll))))),
        maybe(whitespace),
        semicolon([&]() {
          f(angles2quat(azimuth, elevation, roll));
        })),
      sequence(
        string("quat"),
        whitespace,
        parse_float(value.x),
        whitespace,
        parse_float(value.y),
        whitespace,
        parse_float(value.z),
        whitespace,
        parse_float(value.w),
        maybe(whitespace),
        semicolon([&]() {
          f(value);
        }))
    )(input);
  };
}

template<typename T, typename P, typename F>
constexpr auto _argument(P parser, std::string_view name, F f)
{
  return [parser, name, f](std::string_view& input) {
    T value;
    return sequence(
      string(name),
      whitespace,
      parser(value),
      maybe(whitespace),
      semicolon([&]() {
        f(value);
      }))(input);
  };
}

template<typename F>
constexpr auto float_argument(std::string_view name, F f)
{
  return _argument<float>(parse_float, name, f);
}

template<typename F>
constexpr auto bool_argument(std::string_view name, F f)
{
  return _argument<bool>(parse_bool, name, f);
}

template<typename F>
constexpr auto string_argument(std::string_view name, F f)
{
  return _argument<std::string>(parse_string, name, f);
}

template<typename PTMF, typename... Args>
inline void set_source(api::Publisher& controller
    , const std::variant<std::string, unsigned int>& id_or_number
    , PTMF f, Args&&... args)
{
  if (auto id = std::get_if<std::string>(&id_or_number))
  {
    (controller.take_control().get()->*f)(*id, std::forward<Args>(args)...);
  }
  else if (auto number = std::get_if<unsigned int>(&id_or_number))
  {
    auto control = controller.take_control();
    auto id = control->get_source_id(*number);
    if (id.size())
    {
      (control.get()->*f)(id, std::forward<Args>(args)...);
    }
    else
    {
      SSR_ERROR("Source number " << *number << " not found");
    }
  }
  else
  {
    assert(false);
  }
}

constexpr auto parse_message(api::Publisher& controller, Subscriber& subscriber)
{
  return [&controller, &subscriber](std::string_view& input) {
    std::variant<std::string, unsigned int> id;
    return choice(
      sequence(
        string("src"),
        whitespace,
        id_or_number(id),
        whitespace,
        choice(
          pos([&](const Pos& pos) {
            set_source(controller, id
                , &api::SceneControlEvents::source_position, pos);
          }),
          rot([&](const Rot& rot) {
            set_source(controller, id
                , &api::SceneControlEvents::source_rotation, rot);
          }),
          float_argument("vol", [&](float vol) {
            set_source(controller, id
                , &api::SceneControlEvents::source_volume, vol);
          }),
          bool_argument("mute", [&](bool mute) {
            set_source(controller, id
                , &api::SceneControlEvents::source_mute, mute);
          }),
          string_argument("model", [&](const std::string& model) {
            set_source(controller, id
                , &api::SceneControlEvents::source_model, model);
          }))),
      sequence(
        string("ref"),
        whitespace,
        choice(
          pos([&](const Pos& pos) {
            controller.take_control()->reference_position(pos);
          }),
          rot([&](const Rot& rot) {
            controller.take_control()->reference_rotation(rot);
          }),
          sequence(
            string("offset"),
            whitespace,
            choice(
              pos([&](const Pos& pos) {
                controller.take_control()->reference_position_offset(pos);
              }),
              rot([&](const Rot& rot) {
                controller.take_control()->reference_rotation_offset(rot);
            }))))),
      float_argument("vol", [&](float volume) {
        controller.take_control()->master_volume(volume);
      }),
      bool_argument("processing", [&](bool processing) {
        controller.take_control()->processing(processing);
      }),
      bool_argument("transport", [&](bool rolling) {
        controller.take_control()->transport_rolling(rolling);
      }),
      float_argument("seek", [&](float time) {
        controller.take_control()->transport_locate_seconds(time);
      }),
      // TODO: frame?
      sequence(
        string("tracker"),
        whitespace,
        string("reset"),
        maybe(whitespace),
        semicolon([&](){
          controller.take_control()->reset_tracker();
        })),
      string_argument("subscribe", [&](const std::string& name) {
        subscriber.subscribe(name);
      }),
      string_argument("unsubscribe", [&](const std::string& name) {
        subscriber.unsubscribe(name);
      }),
      bool_argument("source-numbers", [&](bool value) {
        subscriber.source_numbers(value);
      }),
      bool_argument("quaternions", [&](bool value) {
        subscriber.quaternions(value);
      }),
      string_argument("load-scene", [&](const std::string& name) {
        controller.take_control()->load_scene(name);
      }),
      string_argument("save-scene", [&](const std::string& name) {
        controller.take_control()->save_scene(name);
      })
      // TODO: new-src?
      // TODO: del-src?
    )(input);
  };
}

}  // namespace internal

inline Parser::Parser(api::Publisher& controller, Subscriber& subscriber)
  : _parse_message{internal::parse_message(controller, subscriber)}
{}

}  // namespace fudi

}  // namespace ssr

#endif
