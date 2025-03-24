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
/// Connection class (definition).

#ifndef SSR_CONNECTION_H
#define SSR_CONNECTION_H

#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*
#endif

#if !defined(ASIO_STANDALONE)
#define ASIO_STANDALONE
#endif
#include <asio.hpp>

#include <memory>

#include "networksubscriber.h"
#include "commandparser.h"

namespace ssr
{

namespace api { struct Publisher; }

namespace legacy_network
{

/// Connection class.
class Connection : public std::enable_shared_from_this<Connection>
{
  public:
    /// Ptr to Connection
    typedef std::shared_ptr<Connection> pointer;
    typedef asio::ip::tcp::socket socket_t;

    static pointer create(asio::io_context &io_context
        , api::Publisher &controller, char end_of_message_character);

    void start();
    void write(const std::string& writestring);

    /// @return Reference to socket
    socket_t& socket() { return _socket; }

    unsigned int get_source_number(id_t source_id) const;

  private:
    Connection(asio::io_context &io_context, api::Publisher &controller
        , char end_of_message_character);

    void start_read();
    void read_handler(const asio::error_code &error, size_t size);
    void write_handler(std::shared_ptr<std::string> str_ptr
        , const asio::error_code &error, size_t bytes_transferred);

    void timeout_handler(const asio::error_code &e);

    /// TCP/IP socket
    socket_t _socket;
    /// Buffer for incoming messages.
    asio::streambuf _streambuf;
    /// @see Connection::timeout_handler
    asio::steady_timer _timer;

    /// Reference to Controller
    api::Publisher &_controller;
    /// Subscriber obj
    NetworkSubscriber _subscriber;
    /// Commandparser obj
    CommandParser _commandparser;

    char _end_of_message_character;

    std::vector<std::unique_ptr<api::Subscription>> _subs;
};

}  // namespace legacy_network

}  // namespace ssr

#endif
