#include "oschandler.h"
#include <vector>

// client ctor
ssr::OscHandler::OscHandler(Publisher& controller, int port_in, int
    port_out, std::string mode)
  : _controller(controller),
  , _osc_receiver(controller, *this, port_in)
  , _osc_sender(*this, port_out)
  , _mode(mode)
{}

// server ctor
ssr::OscHandler::OscHandler(Publisher& controller, int port_in, int port_out,
    std::string mode, std::multimap<std::string hostname, int port>
    clients)
  : _controller(controller),
  , _osc_receiver(controller, *this, port_in)
  , _mode(mode)
{
  std::vector<lo::Address> new_clients;
  for (const auto& hostname: clients)
  {
    new_clients.push_back(new lo::Address(hostname.first,
          std::to_string(hostname.second)));
  }
  _osc_sender(*this, port_out, new_clients);
}

ssr::OscHandler::~OscHandler()
{}

ssr::OscHandler::start()
{
  _oscReceiver.start();
}

/** Implementation of friend function to set the OscSender's server_address
 * @param OscHandler holding both OscReceiver and OscSender
 * @param Hostname to be set
 * @param Port to be set
 */
void ssr::OscReceiver::set_server_for_client(OscHandler& self, lo::Address
    server_address)
{
  self->_osc_sender->set_server_address(server_address);
}

lo::Address OscReceiver::server_address(OscHandler& self)
{
  return self->_osc_sender.server_address;
}
