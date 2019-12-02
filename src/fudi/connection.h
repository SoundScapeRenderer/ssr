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
/// A single bi-directional FUDI connection.

#ifndef SSR_FUDI_CONNECTION_H
#define SSR_FUDI_CONNECTION_H

#include <asio.hpp>

#include "parser.h"
#include "subscriber.h"
#include "ssr_global.h"  // for SSR_ERROR()

namespace ssr
{

namespace fudi
{

class Connection : public std::enable_shared_from_this<Connection>
{
public:
  Connection(asio::ip::tcp::socket socket, api::Publisher& controller)
    : _socket{std::move(socket)}
    , _subscriber{*this, controller}
    , _bundle_subscription{controller.subscribe()->bundle(&_subscriber)}
    , _parser{controller, _subscriber}
  {}

  void start()
  {
    _read_socket();
  }

  void write(std::shared_ptr<buffer_t> buffer)
  {
    assert(buffer);
    auto self(shared_from_this());
    asio::async_write(_socket, asio::buffer(buffer->data(), buffer->size()),
        [self, buffer](std::error_code, size_t)
        {
          // Nothing to do here
        });
  }

private:
  void _read_socket()
  {
    auto self{shared_from_this()};
    // NB: minus one for zero termination
    auto buffersize = _read_buffer.size() - 1;
    if (_read_offset >= buffersize)
    {
      SSR_ERROR("Input buffer is full; dropping contents");
      _read_offset = 0;
    }
    auto* data = _read_buffer.data() + _read_offset;
    const auto size = buffersize - _read_offset;
    _socket.async_read_some(asio::buffer(data, size),
        [this, self](const std::error_code& ec, std::size_t length)
        {
          if (!ec)
          {
            _read_offset += length;
            // NB: zero termination is needed for std::strtof and std::strtoul
            // TODO: remove this once std::from_chars is used instead.
            _read_buffer[_read_offset] = '\0';
            auto input = std::string_view{_read_buffer.data(), _read_offset};
            _parser.parse(input);
            auto consumed = input.data() - _read_buffer.data();
            if (consumed)
            {
              // Move remaining data to beginning of buffer
              auto* buffer = _read_buffer.data();
              std::copy(buffer + consumed, buffer + _read_offset, buffer);
              _read_offset -= consumed;
            }
            _read_socket();
          }
        });
  }

  // See MAXPDSTRING in m_pd.h, and INBUFSIZE in s_inter.c
  // ... plus one for zero termination
  std::array<char, 4096 + 1> _read_buffer;
  std::size_t _read_offset{0};
  asio::ip::tcp::socket _socket;
  Subscriber _subscriber;
  std::unique_ptr<api::Subscription> _bundle_subscription;
  Parser _parser;
};

}  // namespace fudi

}  // namespace ssr

#endif
