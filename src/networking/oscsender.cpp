/**
 * Implementation of oscsender.h
 * @file oscsender.cpp
 */

#include <chrono>
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
  , _poll_all_clients(false)
  , _mode(handler.mode())
  , _poll_thread(0)
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
  _poll_all_clients = true;
  _poll_thread = new std::thread(std::bind(&OscSender::poll_all_clients(), this);
}

/**
 * Function to stop the OscSender object
 * This unsubscribes the OscSender from the Publisher and stops the client
 * polling thread
 */
void ssr::OscSender::stop()
{
  _controller.unsubscribe(*this);
  _is_subscribed = false;
  VERBOSE2("Stopping client polling thread ...");
  _poll_all_clients = false;
  if (_poll_thread)
  {
    _poll_thread->join();
  }
  VERBOSE2("Client polling thread stopped.");
}

/**
 * Sends a '/poll' message to all client instances listed in _client_addresses,
 * then makes the thread calling this function sleep for 1000 milliseconds
 */
void ssr::OscSender::poll_all_clients()
{
  while(_poll_all_clients)
  {
    send_to_all_clients("poll", lo::Message());
    //TODO find better solution to compensate for execution time
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

/**
 * Function to return OscSender's _server_address
 * @return a lo::Address object representing the current server for this client
 */
lo::Address ssr::OscSender::server_address()
{
  return this->_server_address;
}

/**
 * Function to set OscSender's _server_address
 * @param server_address a lo::Address to be used as _server_address
 */
void ssr::OscSender::set_server_address(lo::Address server_address)
{
  this->_server_address = server_address;
}

/**
 * Function to send a lo::Message to the lo:Address setup as server using a
 * predefined path
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_server(std::string path, lo::Message message)
{
  _server_address.send_from(_send_from, path, message);
}

/**
 * Function to send a lo::Bundle to the lo:Address setup as server
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_server(lo::Bundle bundle)
{
  _server_address.send_from(_send_from, bundle);
}

/**
 * Function to send a lo::Message to a client.
 * @param address a lo:Address that will be sent to, when found in
 * _client_addresses.
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_client(lo::Address address, std::string path,
    lo::Message message)
{
  for (const auto& client _client_addresses)
  {
    if(client.hostname() == address.hostname() && client.port() ==
        address.port())
    {
      client.send_from(_send_from, path, message);
    }
  }
}

/**
 * Function to send a lo::Bundle to a client.
 * @param address a lo:Address that will be sent to, when found in
 * _client_addresses.
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_client(lo::Address address, lo::Bundle bundle)
{
  for (const auto& client _client_addresses)
  {
    if(client.hostname() == address.hostname() && client.port() ==
        address.port())
    {
      client.send_from(_send_from, bundle);
    }
  }
}

/**
 * Function to send a lo::Message to all clients setup in _client_addresses
 * vector.
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_all_clients(std::string path, lo::Message message)
{
  for (const auto& client_address: _client_addresses)
  {
    client_address.send_from(_send_from, path, message);
  }
}

/**
 * Function to send a lo::Bundle to all clients setup in _client_addresses
 * vector.
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_all_clients(lo::Bundle bundle)
{
  for (const auto& client_address: _client_addresses)
  {
    client_address.send_from(_send_from, bundle);
  }
}
