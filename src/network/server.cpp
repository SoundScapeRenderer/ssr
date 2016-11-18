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
/// Server class (implementation).

#include "server.h"

ssr::Server::Server(Publisher& controller, int port
    , char end_of_message_character)
  : _controller(controller)
  , _io_service()
  , _acceptor(_io_service
      , asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
  , _network_thread(0)
  , _end_of_message_character(end_of_message_character)
{}

ssr::Server::~Server()
{
  this->stop();
}

void
ssr::Server::start_accept()
{
  Connection::pointer new_connection = Connection::create(_io_service
      , _controller, _end_of_message_character);

  _acceptor.async_accept(new_connection->socket()
      , std::bind(&Server::handle_accept, this, new_connection
      , std::placeholders::_1));
}

void
ssr::Server::handle_accept(Connection::pointer new_connection
    , const asio::error_code &error)
{
  if (!error)
  {
    new_connection->start();
    start_accept();
  }
}

void
ssr::Server::start()
{
  _network_thread = new std::thread(std::bind(&Server::run, this));
}

void
ssr::Server::stop()
{
  VERBOSE2("Stopping network thread ...");
  if (_network_thread)
  {
    _io_service.stop();
    _network_thread->join();
  }
  VERBOSE2("Network thread stopped.");
}

void
ssr::Server::run()
{
  start_accept();
  _io_service.run();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
