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

#include <memory>
#include "connection.h"
#include "publisher.h"

/// ctor
ssr::Connection::Connection(asio::io_service &io_service
    , Publisher &controller, char end_of_message_character)
  : _socket(io_service)
  , _timer(io_service)
  , _controller(controller)
  , _subscriber(*this)
  , _commandparser(controller)
  , _is_subscribed(false)
  , _end_of_message_character(end_of_message_character)
{}

/// dtor
ssr::Connection::~Connection()
{
  if (_is_subscribed) _controller.unsubscribe(&_subscriber);
  _is_subscribed = false;
}

/** Get an instance of Connection.
 * @param io_service 
 * @param controller used to (un)subscribe and get the actual Scene
 * @return ptr to Connection
 **/
ssr::Connection::pointer
ssr::Connection::create(asio::io_service &io_service
    , Publisher& controller, char end_of_message_character)
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
  // ok... this Connection object is activated.
  // now we can connect the NetworkSubscriber.

  _controller.subscribe(&_subscriber);
  _is_subscribed = true;
  // this stuff should perhaps get refactored.
  // need to think about this. not sure if i like this mixed into
  // the Connection code. 
  std::string whole_scene = _controller.get_scene_as_XML();
  this->write(whole_scene);
  // And we can also start_read ing.
  start_read();

  // intialize the timer
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

  _subscriber.send_levels();

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
ssr::Connection::write(std::string &writestring)
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

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
