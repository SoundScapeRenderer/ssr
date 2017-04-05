/**
 * @file oscreceiver.h
 * Header for OscReceiver, declaring a class, responsible for evaluating
 * received OSC messages and interfacing the SSR's controller.
 */

#ifndef OSC_RECEIVER_H
#define OSC_RECEIVER_H
#endif


#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*
#endif

#include <lo/lo.h>
#include <lo/lo_cpp.h>

#include "ssr_global.h"

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
 */
class OscReceiver
{
  private:
    lo::ServerThread _receiver;
    Publisher& _controller;
    ssr::OscHandler& _handler;
    std::string _mode;
    void add_client_to_server_methods();
    void add_server_to_client_methods();
  public:
    OscReceiver(Publisher& controller, OscHandler& handler, int port);
    ~OscReceiver();
    void start();
    void stop();
    void set_server_for_client(OscHandler& handler, lo::Address
        server_address);
    lo::Address server_address(OscHandler& handler);
};

} // namespace ssr
