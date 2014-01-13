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

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

#include "networksubscriber.h"
#include "commandparser.h"

namespace ssr
{

struct Publisher;

/// Connection class.
class Connection : public boost::enable_shared_from_this<Connection>
{
  public:
    /// Ptr to Connection
    typedef boost::shared_ptr<Connection> pointer;
    typedef boost::asio::ip::tcp::socket socket_t;

    static pointer create(boost::asio::io_service &io_service
        , Publisher &controller);

    void start();
    void write(std::string &writestring);

    /// @return Reference to socket
    socket_t& socket() { return _socket; }

    ~Connection();

  private:
    Connection(boost::asio::io_service &io_service, Publisher &controller);

    void start_read();
    void read_handler(const boost::system::error_code &error, size_t size);
    void write_handler(boost::shared_ptr<std::string> str_ptr
        , const boost::system::error_code &error, size_t bytes_transferred);

    void timeout_handler(const boost::system::error_code &e);

    /// TCP/IP socket
    socket_t _socket;
    /// Buffer for incoming messages.  
    boost::asio::streambuf _streambuf;
    /// @see Connection::timeout_handler
    boost::asio::deadline_timer _timer;

    /// Reference to Controller
    Publisher &_controller;
    /// Subscriber obj
    NetworkSubscriber _subscriber;
    /// Commandparser obj 
    CommandParser _commandparser;

    bool _is_subscribed;
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
