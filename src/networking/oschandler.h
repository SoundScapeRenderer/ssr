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

#include "oscreceiver.h"
#include "oscsender.h"

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
    const unsigned int _message_level = 2;

  public:
    OscHandler(Publisher& controller, int port, std::string mode,
        std::multimap<std::string, int> clients);
    ~OscHandler();
    void start();
    void stop();
    std::string mode();
    lo::ServerThread& server();
    const unsigned int& message_level();
    friend void OscReceiver::set_server_for_client(OscHandler& self,
        lo::Address server_address);
    friend lo::Address OscReceiver::server_address(OscHandler& self);
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
    friend void OscReceiver::add_client(OscHandler& self, lo::Address client);
    friend void OscReceiver::remove_client(OscHandler& self, lo::Address client);
};

} // namespace ssr
#endif
