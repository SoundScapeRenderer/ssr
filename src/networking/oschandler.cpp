/**
 * Implementation of oschandler.h
 * @file oschandler.cpp
 */

#include "oschandler.h"
#include <vector>

/**
 * Constructor used to create client objects
 * @param controller reference to a Publisher object
 * @param port_in an integer describing the port number used for incoming
 * traffic
 * @param port_out an integer describing the port number used for outgoing
 * traffic
 * @param mode a string defining the mode (client|server)
 */
ssr::OscHandler::OscHandler(Publisher& controller, int port_in, int
    port_out, std::string mode)
  : _controller(controller),
  , _osc_receiver(controller, *this, port_in)
  , _osc_sender(controller, *this, port_out)
  , _mode(mode)
{}

/**
 * Constructor used to create server objects
 * @param controller reference to a Publisher object
 * @param port_in an integer describing the port number used for incoming
 * traffic
 * @param port_out an integer describing the port number used for outgoing
 * traffic
 * @param mode a string defining the mode (client|server)
 * @param clients a multimap holding hostname, port pairs
 */
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

/**
 * Destructor
 */
ssr::OscHandler::~OscHandler()
{}

/**
 * Stop this OscHandler by stopping its OscReceiver and OscSender
 */
ssr::OscHandler::stop()
{
  _oscSender.stop();
  _oscReceiver.stop();
}

/**
 * Start this OscHandler by starting its OscReceiver and OscSender
 */
ssr::OscHandler::start()
{
  _oscReceiver.start();
  _oscSender.start();
}

/**
 * OscHandler's friend function to set the OscSender's server_address
 * @param self reference to OscHandler holding OscSender
 * @param server_address lo::Address object to be used for OscSender
 */
void ssr::OscReceiver::set_server_for_client(OscHandler& self, lo::Address
    server_address)
{
  self->_osc_sender->set_server_address(server_address);
}

/**
 * OscHandler's friend function return OscSender's server_address
 * @param self reference to OscHandler holding the OscSender
 * @return lo::Address server_address of OscSender
 */
lo::Address OscReceiver::server_address(OscHandler& self)
{
  return self->_osc_sender.server_address;
}

/**
 * This function returns the OscHandler's mode
 */
std::string mode()
{
  return *this->_mode;
}
