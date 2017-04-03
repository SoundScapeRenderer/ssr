#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H
#endif

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

/*
 * \class OscHandler
 * \brief Class holding Publisher and Subscriber implementation, while being responsible for
 * sending and receiving OSC messages.
 * This class holds a Publisher implementation (OscReceiver), which turns
 * incoming OSC messages into calls to the Controller.
 * It also holds an implementation of Subscriber (OscSender), which turns
 * Publisher functionality into outgoing OSC messages
 *
 * \author David Runge
 * \version $Revision: 0.1 $
 * \date $Date: 2017/03/29
 * Contact: dave@sleepmap.de
 *
 */
class OscHandler
{
  private:
    // mode: client|server
    std::string _mode;
    Publisher& _controller;
    OscReceiver _osc_receiver;
    OscSender _osc_sender;

  public:
    // client ctor
    OscHandler(Publisher& controller, int port_in, int port_out, std::string
        mode);
    // server ctor
    OscHandler(Publisher& controller, int port_in, int port_out, std::string
        mode, std::multimap<std::string, int> clients);
    ~OscHandler();
    void start();
    void stop();

    //declare set_server_for_client() as friend of class OscReceiver
    friend void OscReceiver::set_server_for_client(OscHandler& self,
        lo::Address server_address);
    //declare set_server_for_client() as friend of class OscReceiver
    friend lo::Address OscReceiver::server_address(OscHandler& self);

};

}
