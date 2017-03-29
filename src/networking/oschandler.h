#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H

#include <config.h> // for ENABLE_*
#include <lo/lo.h>
#include <lo/lo_cpp.h>
#include <osc_receiver.h>
#include <osc_sender.h>

namespace ssr
{
/*
 * \class OscHandler
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
class OscHandler
{
  private:
    int _port;
    lo::ServerThread *_serverThread;
    lo::Address _serverAddress;
    Publisher& _controller;
    OscReceiver _oscReceiver;
    OscSender _oscSender;

  public:
    OscHandler(Publisher& controller, int port);
    ~OscHandler();
    void setPort(int port);
    int getPort();
    void start();
    void stop();
};

}
