/**
 * Header for OscHandler, defining a class, responsible for sending OSC messages
 * and subscribing to the SSR's Publisher.
 * @file oschandler.h
 */

#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H

#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*
#endif

#include <map>
#include <lo/lo.h>
#include <lo/lo_cpp.h>
#include "ssr_global.h" // for VERBOSE, MessageLevel
#include "oscreceiver.h"
#include "oscsender.h"

namespace
{
  const std::string _message_type_false{"F"};
  const std::string _message_type_true{"T"};
  const std::string _string_false{"false"};
  const std::string _string_true{"true"};
}

namespace ssr
{

struct Publisher;

/**
 * OscHandler
 * This class holds a Subscriber derivate (OscSender), which turns actions of
 * the referenced Publisher into OSC messages to clients or a server and an
 * OscReceiver, responsible for callbacks on arriving OSC messages, that are
 * delegated to the Publisher reference.
 */
class OscHandler
{
  private:
    std::string _mode; //< mode: client|server
    Publisher& _controller; //< reference to Publisher object
    lo::ServerThread _server; //< ServerThread used for OSC communication
    OscReceiver _osc_receiver;
    OscSender _osc_sender;

  public:
    OscHandler(Publisher& controller, int port, std::string mode,
        std::multimap<std::string, int> clients);
    ~OscHandler();
    void start();
    void stop();
    std::string mode();
    bool is_client();
    bool is_server();
    lo::ServerThread& server();
    const std::string bool_to_message_type(const bool& message);
    const std::string bool_to_string(const bool& message);
    // OscReceiver friend functions
    friend void OscReceiver::set_server_address(OscHandler& self,
        std::string& hostname, std::string& port);
    friend void OscReceiver::set_server_message_level(OscHandler& self,
        MessageLevel message_level);
    friend lo::Address OscReceiver::server_address(OscHandler& self);
    friend bool OscReceiver::server_is_default(OscHandler& self);
    friend bool OscReceiver::server_is(OscHandler& self, std::string& hostname,
        std::string& port);
    friend void OscReceiver::send_to_client(OscHandler& self, lo::Address
        client_address, std::string path, lo::Message message);
    friend void OscReceiver::send_to_client(OscHandler& self, lo::Address
        client_address, lo::Bundle bundle);
    friend void OscReceiver::send_to_all_clients(OscHandler& self, std::string
        path, lo::Message message);
    friend void OscReceiver::send_to_all_clients(OscHandler& self, lo::Bundle
        bundle);
    friend void OscReceiver::send_to_server(OscHandler& self, std::string path,
        lo::Message message);
    friend void OscReceiver::send_to_server(OscHandler& self, lo::Bundle
        bundle);
    friend void OscReceiver::add_client(OscHandler& self, std::string hostname,
        std::string port, ssr::MessageLevel message_level);
    friend void OscReceiver::deactivate_client(OscHandler& self, std::string
        hostname, std::string port);
    friend void OscReceiver::set_message_level(OscHandler& self, std::string
        hostname, std::string port, ssr::MessageLevel message_level);
};

} // namespace ssr
#endif
