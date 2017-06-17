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
ssr::OscHandler::OscHandler(Publisher& controller, int port, std::string mode,
    std::multimap<std::string, int> clients)
  : _mode(mode)
  , _controller(controller)
  , _server(port)
  , _osc_receiver(controller, *this)
  , _osc_sender(controller, *this)
{
  VERBOSE("OscHandler: Initialized.");
  if (mode == "server")
  {
    VERBOSE("OscHandler: " << clients.size() << " client(s).");
    for (const auto& client: clients)
    {
      _osc_sender.add_client(client.first, std::to_string(client.second),
          ssr::MessageLevel::CLIENT);
    }
  }
}

/**
 * Destructor
 */
ssr::OscHandler::~OscHandler()
{
  stop();
  //TODO: delete members of _client_addresses
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
  // check if lo::ServerThread is valid
  if (!_server.is_valid()) {
    ERROR("OscHandler: ServerThread could not be started!");
  }
  _server.set_callbacks([this]()
    {
      VERBOSE("OscHandler: Started ServerThread.");
    },
    []()
    {
      VERBOSE2("OscHandler: ServerThread cleanup.");
    }
  );
  VERBOSE("OscHandler: url = " << _server.url() << ".");
  _osc_receiver.start();
  _osc_sender.start();
  _server.start();
}

/**
 * OscHandler's friend function to set the OscSender's server_address
 * @param self reference to OscHandler holding OscSender
 * @param hostname reference to std::string representing hostname
 * @param port reference to std::string representing port
 */
void ssr::OscReceiver::set_server_address(ssr::OscHandler& self,
    std::string& hostname, std::string& port)
{
  self._osc_sender.set_server_address(hostname, port);
}

/**
 * OscHandler's friend function to set a the OscSender _server's message_level
 * @param self reference to OscHandler holding OscSender
 * @param hostname reference to std::string representing the client's hostname
 * @param port reference to std::string representing the client's port
 * @param message_level ssr::MessageLevel to be used for server
 */
void ssr::OscReceiver::set_server_message_level(OscHandler& self,
    ssr::MessageLevel message_level)
{
  self._osc_sender.set_server_message_level(message_level);
}

/**
 * OscHandler's friend function to return OscSender's server_address
 * @param self reference to OscHandler holding the OscSender
 * @return lo::Address server_address of OscSender
 */
lo::Address ssr::OscReceiver::server_address(ssr::OscHandler& self)
{
  return self._osc_sender.server_address();
}

/**
 * OscHandler's friend function to check, if the current lo::Address of the
 * server is the default
 * @see ssr::OscSender::server_is_default()
 * @param self reference to OscHandler holding the OscSender
 * @return bool true, if OscSender's server address is the default, false
 * otherwise
 */
bool ssr::OscReceiver::server_is_default(ssr::OscHandler& self)
{
  return self._osc_sender.server_is_default();
}

/**
 * OscHandler's friend function to check, if the provided hostname and port
 * match the OscSender's _server lo::Address
 * @param self reference to OscHandler holding the OscSender
 * @param hostname reference to std::string representing the hostname
 * @param port reference to std::string representing the port
 * @return bool true, if OscSender's server address matches the provided
 * hostname and port
 */
bool ssr::OscReceiver::server_is(ssr::OscHandler& self, std::string& hostname,
    std::string& port)
{
  return self._osc_sender.server_is(hostname, port);
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
 * OscHandler's friend function to add a client to the list of OscSender's
 * _client_addresses.
 * @param self reference to OscHandler holding OscSender
 * @param hostname std::string representing the client's hostname
 * @param port std::string representing the client's port 
 * @param message_level ssr::MessageLevel representing the client's message
 * level
 */
void ssr::OscReceiver::add_client(OscHandler& self, std::string hostname,
    std::string port, ssr::MessageLevel message_level =
    ssr::MessageLevel::CLIENT)
{
  self._osc_sender.add_client(hostname, port, message_level);
}

/**
 * OscHandler's friend function to deactivate a client from the list of
 * OscSender's _clients
 * @param self reference to OscHandler holding OscSender
 * @param hostname std::string representing the client's hostname
 * @param port std::string representing the client's port 
 */
void ssr::OscReceiver::deactivate_client(OscHandler& self, std::string
    hostname, std::string port)
{
  self._osc_sender.deactivate_client(hostname, port);
}

/**
 * OscHandler's friend function to set a client's message_level
 * @param self reference to OscHandler holding OscSender
 * @param hostname std::string representing the client's hostname
 * @param port std::string representing the client's port
 * @param message_level ssr::MessageLevel to be used for client
 */
void ssr::OscReceiver::set_client_message_level(OscHandler& self, std::string
    hostname, std::string port, ssr::MessageLevel message_level)
{
  self._osc_sender.set_client_message_level(hostname, port, message_level);
}


/**
 * This function returns the OscHandler's mode
 * @return std::string (either server or client)
 */
std::string ssr::OscHandler::mode()
{
  return _mode;
}

/**
 * Returns true, if the instance of OscHandler is a 'client', false otherwise.
 * @return true, if _oschandler.mode() returns 'client', false otherwise.
 */
bool ssr::OscHandler::is_client()
{
  if(!_mode.compare("client"))
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Returns true, if the instance of OscHandler is a 'server', false otherwise.
 * @return true, if _oschandler.mode() returns 'server', false otherwise.
 */
bool ssr::OscHandler::is_server()
{
  if(!_mode.compare("server"))
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Return reference to OscHandler's lo::ServerThread
 * @return lo::ServerThread& reference to _server
 */
lo::ServerThread& ssr::OscHandler::server()
{
  return _server;
}

/**
 * Returns a std::string representing the message type of a boolean type
 * @param message bool
 * @return _message_type_true if message true, _message_type_false otherwise
 */
const std::string ssr::OscHandler::bool_to_message_type(const bool& message)
{
  if(message)
  {
    return _message_type_true;
  }
  else
  {
    return _message_type_false;
  }
}

/**
 * Returns a std::string representing the string of a boolean type
 * @param message bool
 * @return 'true' if message true, 'false' otherwise
 */
const std::string ssr::OscHandler::bool_to_string(const bool& message)
{
  if(message)
  {
    return _string_true;
  }
  else
  {
    return _string_false;
  }
}
