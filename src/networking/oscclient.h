/**
 * Header for OscClient, defining a class, holding client information
 * @file oscclient.h
 */

#ifndef OSC_CLIENT_H
#define OSC_CLIENT_H

#include <lo/lo.h>
#include <lo/lo_cpp.h>

namespace ssr
{

enum class MessageLevel : unsigned int
{
  SERVER = 0,
  THIN_CLIENT = 1,
  CLIENT = 2
};

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
    void deactivate();
    void set_message_level(MessageLevel message_level);
    const std::string hostname();
    const std::string port();
};

}
#endif
