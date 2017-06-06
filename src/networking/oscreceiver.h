/**
 * @file oscreceiver.h
 * Header for OscReceiver, declaring a class, responsible for evaluating
 * received OSC messages and interfacing the SSR's controller.
 */

#ifndef OSC_RECEIVER_H
#define OSC_RECEIVER_H


#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*
#endif

#include <lo/lo.h>
#include <lo/lo_cpp.h>

#include "ssr_global.h" // for VERBOSE, MessageLevel

namespace ssr
{

struct Publisher;

// forward declaration for OscHandler friend functions
class OscHandler;

/**
 * OscReceiver
 * This class holds a Publisher and an OscHandler reference.
 * It is responsible for all incoming OSC messages and using callback functions
 * to trigger functionality in the Publisher.
 * @todo add functionality to only process a subset of incoming client messages
 */
class OscReceiver
{
  private:
    Publisher& _controller;
    ssr::OscHandler& _handler;
    void add_client_to_server_methods();
    void add_update_notification_methods();
    void add_poll_methods();
    void add_source_methods();
    void add_reference_methods();
    void add_scene_methods();
    void add_processing_methods();
    void add_transport_methods();
    void add_tracker_methods();

  public:
    OscReceiver(Publisher& controller, OscHandler& handler);
    ~OscReceiver();
    void start();
    void stop();
    void set_server_for_client(OscHandler& handler, lo::Address
        server_address);
    lo::Address server_address(OscHandler& handler);
    void send_to_client(OscHandler& self, lo::Address client_address,
        std::string path, lo::Message message);
    void send_to_client(OscHandler& self, lo::Address client_address,
        lo::Bundle bundle);
    void send_to_all_clients(OscHandler& self, std::string path, lo::Message
        message);
    void send_to_all_clients(OscHandler& self, lo::Bundle bundle);
    void send_to_server(OscHandler& self, std::string path, lo::Message
        message);
    void send_to_server(OscHandler& self, lo::Bundle bundle);
    void add_client(OscHandler& self, std::string hostname, std::string port,
        ssr::MessageLevel message_level);
    void deactivate_client(OscHandler& self, std::string hostname, std::string
        port);
    void set_message_level(OscHandler& self, std::string hostname, std::string
        port, ssr::MessageLevel message_level);
};

} // namespace ssr
#endif
