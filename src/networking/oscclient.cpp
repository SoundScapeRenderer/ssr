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
{
  _active = true;
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
 * Function to set the OscClient's _active state
 * @param bool, representing the OscClient's new state
 **/
void ssr::OscClient::deactivate()
{
  _active = false;
}


/**
 * Function to get the OscClient's _address
 * @return a lo::Address used for the OscClient
 **/
lo::Address ssr::OscClient::address()
{
  return _address;
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
