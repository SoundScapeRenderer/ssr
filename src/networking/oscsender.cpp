#include "oschandler.h"
#include "oscsender.h"

// client ctor
ssr::OscSender::OscSender(Publisher& controller, OscHandler& handler, int
    port_out)
  : _controller(controller)
  , _handler(handler)
  , _send_from(lo::ServerThread(port_out))
  , _is_subscribed(false)
{}

// server ctor
ssr::OscSender::OscSender(Publisher& controller, OscHandler& handler, int
    port_out, std::vector<lo::Address> client_addresses)
  : _controller(controller)
  , _handler(handler)
  , _send_from(lo::ServerThread(port_out))
  , _client_addresses(client_addresses)
  , _is_subscribed(false)
{}

ssr::OscSender::start()
{
  _controller.subscribe(*this);
  _is_subscribed = true;
}

ssr::OscSender::stop()
{
  _controller.unsubscribe(*this);
  _is_subscribed = false;
}

/** Implementation of function to return OscSender's _server_address
 * @return _server_address
 */
lo::Address server_address()
{
  return this->_server_address;
}

/** Implementation of function to set OscSender's _server_address
 * @return _server_address
 */
void set_server_address(lo::Address server_address)
{
  this->_server_address = server_address;
}

