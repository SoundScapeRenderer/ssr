/**
 * \file oscreceiver.h
 * \brief Header for OscReceiver, defining a class, responsible for evaluating
 * received OSC messages and interfacing the SSR's controller
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

/** OscReceiver
 * \brief Class holding Publisher and Subscriber implementation, while being responsible for
 * sending and receiving OSC messages.
 * This class holds a Publisher implementation (OscReceiver), which turns
 * incoming OSC messages into calls to the Controller.
 * It also holds an implementation of Subscriber (OscSender), which 
 * \author David Runge
 * \version $Revision: 0.1 $
 * \date $Date: 2017/03/29
 * Contact: dave@sleepmap.de
 *
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
