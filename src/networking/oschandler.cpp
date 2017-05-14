/**
 * Implementation of oschandler.h
 * @file oschandler.cpp
 */

#include "oschandler.h"
#include <vector>

/**
 * Constructor
 * @param controller reference to a Publisher object
 * @param port_in an integer describing the port number used for incoming
 * traffic
 * @param port_out an integer describing the port number used for outgoing
 * traffic
 * @param mode a string defining the mode (client|server)
 * @param clients a multimap holding hostname, port pairs (only used for
 * server)
 */
ssr::OscHandler::OscHandler(Publisher& controller, int port_in, int port_out,
    std::string mode, std::multimap<std::string, int> clients)
  : _mode(mode)
  , _controller(controller)
  , _osc_receiver(controller, *this, port_in)
  , _osc_sender(controller, *this, port_out)
{
  VERBOSE("OscHandler: Initialized.");
  if (mode == "server")
  {
    VERBOSE("OscHandler: " << clients.size() << " client(s).");
    for (const auto& client: clients)
    {
      _osc_sender.add_client(client.first, std::to_string(client.second));
    }
  }
}

/**
 * Destructor
 */
ssr::OscHandler::~OscHandler()
{
  stop();
  VERBOSE("OscHandler: Destructing.");
}

/**
 * Stop this OscHandler by stopping its OscReceiver and OscSender
 */
void ssr::OscHandler::stop()
{
  VERBOSE("OscHandler: Stopping");
  _osc_receiver.stop();
  _osc_sender.stop();
}

/**
 * Start this OscHandler by starting its OscReceiver and OscSender
 */
void ssr::OscHandler::start()
{
  VERBOSE("OscHandler: Starting");
  _osc_receiver.start();
  _osc_sender.start();
}

/**
 * OscHandler's friend function to set the OscSender's server_address
 * @param self reference to OscHandler holding OscSender
 * @param server_address lo::Address object to be used for OscSender
 */
void ssr::OscReceiver::set_server_for_client(ssr::OscHandler& self, lo::Address
    server_address)
{
  self._osc_sender.set_server_address(server_address.hostname(), server_address.port());
}

/**
 * OscHandler's friend function return OscSender's server_address
 * @param self reference to OscHandler holding the OscSender
 * @return lo::Address server_address of OscSender
 */
lo::Address ssr::OscReceiver::server_address(ssr::OscHandler& self)
{
  return self._osc_sender.server_address();
}

/**
 * OscHandler's friend function to send an OSC message to a client, using
 * OscSender.
 * @param self reference to OscHandler holding OscSender
 * @param client_address lo::Address of client to send to (must be in _client_addresses)
 * @param message lo::Message to be sent
 */
void ssr::OscReceiver::send_to_client(OscHandler& self, lo::Address
    client_address, std::string path, lo::Message message)
{
  self._osc_sender.send_to_client(client_address, path, message);
}

/**
 * OscHandler's friend function to send an OSC bundle to a client, using
 * OscSender.
 * @param self reference to OscHandler holding OscSender
 * @param client_address lo::Address of client to send to (must be in _client_addresses)
 * @param message lo::Bundle to be sent
 */
void ssr::OscReceiver::send_to_client(OscHandler& self, lo::Address
    client_address, lo::Bundle bundle)
{
  self._osc_sender.send_to_client(client_address, bundle);
}

/**
 * OscHandler's friend function to send an OSC message to all clients, using
 * OscSender.
 * @param self reference to OscHandler holding OscSender
 * @param message lo::Message to be sent
 */
void ssr::OscReceiver::send_to_all_clients(OscHandler& self, std::string path,
    lo::Message message)
{
  self._osc_sender.send_to_all_clients(path, message);
}

/**
 * OscHandler's friend function to send an OSC bundle to all clients, using
 * OscSender.
 * @param self reference to OscHandler holding OscSender
 * @param message lo::Bundle to be sent
 */
void ssr::OscReceiver::send_to_all_clients(OscHandler& self, lo::Bundle bundle)
{
  self._osc_sender.send_to_all_clients(bundle);
}

/**
 * OscHandler's friend function to send an OSC message to the server, using
 * OscSender and a designated path.
 * @param self reference to OscHandler holding OscSender
 * @param path std::string defining the path to send on
 * @param message lo::Bundle to be sent
 */
void ssr::OscReceiver::send_to_server(OscHandler& self, std::string path,
    lo::Message message)
{
  self._osc_sender.send_to_server(path, message);
}

/**
 * OscHandler's friend function to send an OSC bundle to the server, using
 * OscSender.
 * @param self reference to OscHandler holding OscSender
 * @param message lo::Bundle to be sent
 */
void ssr::OscReceiver::send_to_server(OscHandler& self, lo::Bundle bundle)
{
  self._osc_sender.send_to_server(bundle);
}

/**
 * This function returns the OscHandler's mode
 */
std::string ssr::OscHandler::mode()
{
  return _mode;
}
