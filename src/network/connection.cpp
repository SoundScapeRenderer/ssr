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
/// Connection class (implementation).

#include <functional>
#include <memory>
#include "connection.h"


/// ctor
ssr::Connection::Connection(asio::io_service &io_service
    , api::Publisher &controller, char end_of_message_character)
  : _socket(io_service)
  , _timer(io_service)
  , _controller(controller)
  , _subscriber(*this)
  , _commandparser(controller)
  , _end_of_message_character(end_of_message_character)
{}

/** Get an instance of Connection.
 * @param io_service
 * @param controller used to (un)subscribe and get the actual Scene
 * @return ptr to Connection
 **/
ssr::Connection::pointer
ssr::Connection::create(asio::io_service &io_service
    , api::Publisher& controller, char end_of_message_character)
{
  return pointer(new Connection(io_service, controller
      , end_of_message_character));
}

/** Start the connection.
 * - Subscribe this instance of Connection to the Controller.
 * - Send the actual scene over the network.
 * - Start reading incoming messages.
 * - Initialize the timer.
 **/
void
ssr::Connection::start()
{
  _subs.push_back(_controller.subscribe_scene_control(&_subscriber));
  _subs.push_back(_controller.subscribe_renderer_control(&_subscriber));
  _subs.push_back(_controller.subscribe_source_metering(&_subscriber));
  _subs.push_back(_controller.subscribe_output_activity(&_subscriber));

  start_read();

  // initialize the timer
  _timer.expires_from_now(std::chrono::milliseconds(100));
  _timer.async_wait(std::bind(&Connection::timeout_handler, shared_from_this()
        , std::placeholders::_1));
}

/** Send levels on timeout.
 * - Send level.
 * - Reset timer.
 * - Wait async.
 * @param e self explanatory
 **/
void
ssr::Connection::timeout_handler(const asio::error_code &e)
{
  if (e) return;

  // Set timer again.
  _timer.expires_from_now(std::chrono::milliseconds(100));
  _timer.async_wait(std::bind(&Connection::timeout_handler, shared_from_this()
        , std::placeholders::_1));
}

/// Start reading from socket.
void
ssr::Connection::start_read()
{
  async_read_until(_socket, _streambuf, _end_of_message_character
      , std::bind(&Connection::read_handler, shared_from_this()
        , std::placeholders::_1
        , std::placeholders::_2));
}

/// Forward string from socket to CommandParser.
void
ssr::Connection::read_handler(const asio::error_code &error
    , size_t size)
{
  if (!error)
  {
    std::istream input_stream(&_streambuf);
    std::string  packet_string;
    getline(input_stream, packet_string, _end_of_message_character);
    (void) size;
    //cout << "size= " << size << endl;
    //cout << "line: " << packet_string << endl;

    _commandparser.parse_cmd(packet_string);

    this->start_read();
  }
  else
  {
    _timer.cancel();
  }
}

/** Write to socket.
 * @param writestring: String to be send over the network.
 **/
void
ssr::Connection::write(const std::string& writestring)
{
  // Create a Copy of this string.
  // Put into shared_ptr bound to the callback
  // handler. This is sufficient to make it
  // be destroyed on exit.

  std::shared_ptr<std::string> str_ptr(new std::string(writestring
      + _end_of_message_character));

  asio::async_write(_socket, asio::buffer(*str_ptr)
      , std::bind(&Connection::write_handler, shared_from_this(), str_ptr
        , std::placeholders::_1
        , std::placeholders::_2));
}

/** Empty callback handler.
 *
 * @param str_ptr String passed to Connection::write
 * @param error error code
 * @param bytes_transferred self explanatory
 *
 * @todo Check if we can delete this function.
 **/
void
ssr::Connection::write_handler(std::shared_ptr<std::string> str_ptr
    , const asio::error_code &error, size_t bytes_transferred)
{
  (void) str_ptr;
  (void) error;
  (void) bytes_transferred;
  // Dont do nothing. we could check for error and stuff.
  // the shared_ptr was just used to keep the string alive and
  // destroy it now.
}

unsigned int
ssr::Connection::get_source_number(id_t source_id) const
{
  return _controller.get_source_number(source_id);
}
