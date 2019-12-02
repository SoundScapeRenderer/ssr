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
/// FUDI server (implementation).

#include <thread>

#include "connection.h"

#include "server.h"

using ssr::fudi::Server;
using asio::ip::tcp;

Server::Server(api::Publisher& controller, short port)
  : _controller{controller}
  , _io_service{}
  , _acceptor{_io_service, tcp::endpoint(tcp::v4(), port)}
  , _socket{_io_service}
{
  _do_accept();
  _thread = std::thread{[this](){ _io_service.run(); }};
}

Server::~Server()
{
  _io_service.stop();
  if (_thread.joinable()) { _thread.join(); }
}

void
Server::_do_accept()
{
  // The signature has changed between Asio 1.10 and 1.12
  // TODO: use the new signature once Travis-CI has caught up
  _acceptor.async_accept(
      _socket,
      [this](std::error_code ec)
      {
        if (!ec)
        {
          std::make_shared<Connection>(
              std::move(_socket), _controller)->start();
        }
        _do_accept();
      });
}
