/**
 * Implementation of oscsender.h
 * @file oscsender.cpp
 */

#include "oschandler.h"
#include "oscsender.h"

/**
 * Constructor used to create client objects
 * @param controller reference to a Publisher object
 * @param handler reference to an OscHandler object
 * @param port_out an integer describing the port number to be used
 * for outgoing traffic
 */
ssr::OscSender::OscSender(Publisher& controller, OscHandler& handler, int
    port_out)
  : _controller(controller)
  , _handler(handler)
  , _send_from(lo::ServerThread(port_out))
  , _is_subscribed(false)
  , _mode(handler.mode())
{}

/**
 * Constructor used to create server objects
 * @param controller reference to a Publisher object
 * @param handler reference to an OscHandler object
 * @param port_out an integer describing the port number to be used
 * for outgoing traffic
 * @param client_addresses vector of lo::Address objects representing
 * clients to this server
 */
ssr::OscSender::OscSender(Publisher& controller, OscHandler& handler, int
    port_out, std::vector<lo::Address> client_addresses)
  : _controller(controller)
  , _handler(handler)
  , _send_from(lo::ServerThread(port_out))
  , _client_addresses(client_addresses)
  , _is_subscribed(false)
  , _mode(handler.mode())
{}

/**
 * Destructor
 */
~OscSender();

/** Function to start the OscSender object
 * This subscribes the OscSender to the Publisher and starts the
 * lo::ServerThread to send from
 */
void ssr::OscSender::start()
{
  _controller.subscribe(*this);
  _is_subscribed = true;
  // check if lo::ServerThread is valid
  if (!_send_from.is_valid()) {
    ERROR("OSC ServerThread to send from could not be started!");
    return 1;
  }
  _send_from.set_callbacks([&_send_from]()
    {
      VERBOSE2("OSC ServerThread init: "<<&_send_from <<".");
    },
    []()
    {
      VERBOSE2("OSC ServerThread cleanup.");
    }
  );
  VERBOSE("OSC URL: " << _send_from.url()<<".");
  _send_from.start();
}

/**
 * Function to stop the OscSender object
 * This unsubscribes the OscSender from the Publisher
 */
void ssr::OscSender::stop()
{
  _controller.unsubscribe(*this);
  _is_subscribed = false;
}

/**
 * Function to return OscSender's _server_address
 * @return a lo::Address object representing the current server for this client
 */
lo::Address server_address()
{
  return this->_server_address;
}

/**
 * Function to set OscSender's _server_address
 * @param server_address a lo::Address to be used as _server_address
 */
void set_server_address(lo::Address server_address)
{
  this->_server_address = server_address;
}

/**
 * Function to send /poll OSC message to all clients
 */
void poll_clients()
{
  for (const auto& client_address: _client_addresses)
  {
    client_address.send_from(_send_from, "poll");
  }
}

/**
 * Function to send a lo::Message to the lo:Address setup as server
 * @param message a predefined lo::Messge object to be sent
 */
void send_to_server(lo::Message message)
{
  _server_address.send_from(_send_from, msg);
}

/**
 * Function to send a lo::Bundle to the lo:Address setup as server
 * @param bundle a predefined lo::Bundle object to be sent
 */
void send_to_server(lo::Bundle bundle)
{
  _server_address.send_from(_send_from, bundle);
}


