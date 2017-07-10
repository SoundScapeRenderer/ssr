/**
 * Implementation of oscclient.h
 * @file oscclient.cpp
 */

#include "oscclient.h"

/**
 * Constructor used to create OSC client objects
 * @param hostname a std::string used in the object's address
 * @param port a std::string used in the object's address
 * @param message_level the MessageLevel defining the clients level of messages
 */
ssr::OscClient::OscClient(std::string hostname, std::string port, MessageLevel
    message_level)
  : _address(hostname, port)
  , _message_level(message_level)
  , _alive_counter(10)
{
  _active = true;
  VERBOSE("OscClient: Initialized as '" << _address.hostname() << ":" <<
      _address.port() << "'.");
}

/**
 * Destructor
 **/
ssr::OscClient::~OscClient()
{}

/**
 * Function to get the OscClient's _message_level.
 * @return message_level a MessageLevel used for the OscClient
 **/
ssr::MessageLevel ssr::OscClient::message_level()
{
  return _message_level;
}

/**
 * Function to get the OscClient's _active state
 * @return a bool, representing the OscClient's state
 **/
bool ssr::OscClient::active()
{
  return _active;
}

/**
 * Function to set the OscClient's _active state to false
 **/
void ssr::OscClient::deactivate()
{
  _active = false;
}

/**
 * Function to set the OscClient's _active state to true
 **/
void ssr::OscClient::activate()
{
  _active = true;
}


/**
 * Function to get the OscClient's _address
 * @return a lo::Address used for the OscClient
 **/
lo::Address& ssr::OscClient::address()
{
  return _address;
}

/**
 * Function to set the OscClient's _address.
 * @param hostname reference to a std::string representing the hostname to be
 * used
 * @param port reference to a std::string& representing the port to be used
 **/
void ssr::OscClient::set_address(std::string& hostname, std::string& port)
{
  _address = lo::Address(hostname, port);
  VERBOSE3("OscClient: Address changed to: " << _address.hostname() << ":" <<
      _address.port() << ".");
}

/**
 * Function to set the OscClient's _message_level.
 * @param message_level a MessageLevel to be used for the OscClient
 **/
void ssr::OscClient::set_message_level(MessageLevel message_level)
{
  _message_level = message_level;
}

/**
 * Function to get the OscClient's hostname.
 * @return std::string representing the OscClient's hostname
 **/
const std::string ssr::OscClient::hostname()
{
  return _address.hostname();
}

/**
 * Function to get the OscClient's port.
 * @return std::string representing the OscClient's port
 **/
const std::string ssr::OscClient::port()
{
  return _address.port();
}

/**
 * Function to increment the OscClient's _alive_counter
 **/
void ssr::OscClient::increment_alive_counter()
{
  _alive_counter++;
}

/**
 * Function to decrement the OscClient's _alive_counter.
 * Deactivates the OscClient, in the case where _alive_counter <= 0
 **/
void ssr::OscClient::decrement_alive_counter()
{
  _alive_counter--;
  if(_alive_counter < 0)
  {
    deactivate();
    VERBOSE("OscClient: Deactivated '" << _address.hostname() << ":" <<
        _address.port() << "' due to inactivity.");
  }
}

/**
 * Function to reset the OscClient's _alive counter
 **/
void ssr::OscClient::reset_alive_counter()
{
  _alive_counter=10;
}
