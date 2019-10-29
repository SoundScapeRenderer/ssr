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
/// A WebSocket-based network interface.

#ifndef SSR_WEBSOCKET_SERVER_H
#define SSR_WEBSOCKET_SERVER_H

#include <fstream>  // for std::ifstream
#include <regex>
#include <thread>

#include "ssr_global.h"  // for SSR_ERROR(), SSR_VERBOSE(), ...
#include "connection.h"  // for Connection

namespace ssr
{

namespace ws
{

class Server
{
public:
  explicit Server(api::Publisher& controller, uint16_t port
      , const std::string& resource_directory)
    : _controller{controller}
    , _serve_dir{resource_directory}
  {
    _server.clear_access_channels(websocketpp::log::alevel::all);
    //_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
    _server.set_error_channels(websocketpp::log::elevel::all);

    _server.init_asio();
    _server.set_reuse_addr(true);  // Avoid "Address already in use"

    _server.set_http_handler(
        [this](connection_hdl hdl) { return this->on_http(hdl); });
    _server.set_validate_handler(
        [this](connection_hdl hdl) { return this->on_validate(hdl); });
    _server.set_open_handler(
        [this](connection_hdl hdl) { return this->on_open(hdl); });
    _server.set_message_handler(
        [this](connection_hdl hdl, message_ptr msg) {
          return this->on_message(hdl, msg);
        });
    _server.set_close_handler(
        [this](connection_hdl hdl) { return this->on_close(hdl); });

    _server.listen(port);
    _server.start_accept();
    _thread = std::thread{&server_t::run, &_server};
  }

  ~Server()
  {
    _server.stop();
    if (_thread.joinable()) { _thread.join(); }
  }

  void on_http(connection_hdl hdl)
  {
    auto con = _server.get_con_from_hdl(hdl);
    std::string resource = con->get_resource();

    if (resource == "/config.json")
    {
      SSR_VERBOSE("Serving " << resource);
      con->append_header("Content-Type", "application/json");
      con->set_body(
          "{\"host\":\"" + con->get_host() +
          "\",\"port\":" + std::to_string(con->get_port()) +
          ",\"autoconnect\":true}");
      con->set_status(websocketpp::http::status_code::ok);
      return;
    }

    if (resource == "/") {
      // TODO: make configurable:
      resource = "/ssr-test-client.html";
    }
    resource = _serve_dir + resource;
    std::ifstream file{resource};
    std::string body{std::istreambuf_iterator<char>{file},
                     std::istreambuf_iterator<char>{}};
    if (body != "")
    {
      std::string content_type;
      std::smatch match;
      if (std::regex_match(resource, match, _re_filename))
      {
        if (match[1].str().size())
        {
          // If file name contains hash, cache expires after about 4 years:
          con->append_header("Cache-Control", "public, max-age=123456789");
        }
        const std::string extension = match[2].str();
        if (extension == "html")
        {
          content_type = "text/html";
        }
        else if (extension == "js")
        {
          content_type = "application/javascript";
        }
        else if (extension == "css")
        {
          content_type = "text/css";
        }
        else if (extension == "png")
        {
          content_type = "image/png";
        }
        else if (extension == "ico")
        {
          content_type = "image/x-icon";
        }
        else if (extension == "js.map")
        {
          content_type = "application/json";
        }
      }
      SSR_VERBOSE_NOLF("Serving " << resource);
      if (content_type == "")
      {
        SSR_VERBOSE(" (unknown file extension)");
      }
      else
      {
        SSR_VERBOSE(" as " << content_type);
        con->append_header("Content-Type", content_type);
      }
      con->set_body(body);
      con->set_status(websocketpp::http::status_code::ok);
    }
    else
    {
      con->set_status(websocketpp::http::status_code::not_found);
      SSR_VERBOSE("Requested file is not available: " << resource);
    }
  }

  bool on_validate(connection_hdl hdl)
  {
    auto websocket = _server.get_con_from_hdl(hdl);
    const auto& subp_requests = websocket->get_requested_subprotocols();
    for (const auto& proto: subp_requests)
    {
      SSR_VERBOSE2("Subprotocol \"" << proto << "\" requested");
      if (proto == _subprotocol)
      {
        websocket->select_subprotocol(proto);
        return true;
      }
    }
    // TODO: error status to client?
    SSR_ERROR("Wrong subprotocol(s) requested");
    return false;
  }

  void on_open(connection_hdl hdl)
  {
    assert(_connections.find(hdl) == _connections.end());
    _connections.try_emplace(hdl, hdl, _server, _controller);
    SSR_VERBOSE("WebSocket opened");
  }

  void on_message(connection_hdl hdl, message_ptr msg)
  {
    auto iter = _connections.find(hdl);
    if (iter != _connections.end())
    {
      iter->second.on_message(msg);
    }
    else
    {
      SSR_ERROR("Invalid connection handle for incoming message");
    }
  }

  void on_close(connection_hdl hdl)
  {
    auto result = _connections.erase(hdl);
    if (result == 1)
    {
      SSR_VERBOSE("WebSocket closed");
    }
    else
    {
      SSR_ERROR("Connection to be closed does not exist");
    }
  }

private:
  api::Publisher& _controller;
  std::string _serve_dir;
  std::string _subprotocol{"ssr-json"};
  server_t _server;
  std::map<connection_hdl, Connection, std::owner_less<connection_hdl>>
    _connections;
  std::thread _thread;

  std::regex _re_filename{
    // at least one character, non-greedy
    ".+?"
    // optional dot + hash
    "(\\.[0-9a-f]{20})?"
    // dot + known file extension
    "\\.(html|js|css|png|ico|js\\.map)"};
};

}  // namespace ws

}  // namespace ssr

#endif
