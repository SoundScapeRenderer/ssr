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
/// Legacy network server class (definition).

#ifndef SSR_SERVER_H
#define SSR_SERVER_H

#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*
#endif

#if !defined(ASIO_STANDALONE)
#define ASIO_STANDALONE
#endif
#include <asio.hpp>

#include <thread>

#include "connection.h"

namespace ssr
{

namespace api { struct Publisher; }
struct LegacyXmlSceneProvider;

namespace legacy_network
{

/// Server class.
class Server
{
  public:
    Server(api::Publisher& controller, LegacyXmlSceneProvider& scene_provider
        , int port, char end_of_message_character);
    ~Server();
    void start();
    void stop();

  private:
    void run();
    void start_accept();
    void handle_accept(Connection::pointer new_connection
        , const asio::error_code &error);

    api::Publisher& _controller;
    // Just a hack for get_scene_as_XML():
    LegacyXmlSceneProvider& _scene_provider;
    asio::io_context _io_context;
    asio::ip::tcp::acceptor _acceptor;
    std::thread *_network_thread;

    char _end_of_message_character;
};

}  // namespace legacy_network

}  // namespace ssr

#endif
