/**
 * Header for OscClient, defining a class, holding client information
 * @file oscclient.h
 */

#ifndef OSC_CLIENT_H
#define OSC_CLIENT_H

#include <lo/lo.h>
#include <lo/lo_cpp.h>
#include "ssr_global.h" // for VERBOSE, MessageLevel

namespace ssr
{

class OscClient
{
  private:
    lo::Address _address;
    MessageLevel _message_level;
    bool _active = false;

  public:
    OscClient(std::string hostname, std::string port, MessageLevel
        message_level);
    ~OscClient();

    ssr::MessageLevel message_level();
    lo::Address& address();
    bool active();
    void activate();
    void deactivate();
    void set_address(std::string& hostname, std::string& port);
    void set_message_level(MessageLevel message_level);
    const std::string hostname();
    const std::string port();
};

}
#endif
