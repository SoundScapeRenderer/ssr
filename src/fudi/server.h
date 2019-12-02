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
/// A FUDI server for communicating with Puredata.

#ifndef SSR_FUDI_SERVER_H
#define SSR_FUDI_SERVER_H

#include <asio.hpp>

namespace ssr
{

namespace api { struct Publisher; }

namespace fudi
{

class Server
{
public:
  explicit Server(api::Publisher& controller, short port);
  ~Server();

private:
  void _do_accept();

  api::Publisher& _controller;
  // TODO: io_context is not yet supported in Asio 1.10
  //asio::io_context _io_context;
  asio::io_service _io_service;
  asio::ip::tcp::acceptor _acceptor;
  asio::ip::tcp::socket _socket;
  std::thread _thread;
};

}  // namespace fudi

}  // namespace ssr

#endif
