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
  MESSAGE_LEVEL_SERVER = 0,
  MESSAGE_LEVEL_THIN_CLIENT = 1,
  MESSAGE_LEVEL_CLIENT = 2
};

class OscClient
{
  private:
    const lo::Address _address;
    MessageLevel _message_level;
  public:
    OscClient(std::string hostname, std::string port, MessageLevel
        message_level);
    ~OscClient();

    ssr::MessageLevel message_level();
    void set_message_level(MessageLevel message_level);
    const std::string hostname();
    const std::string port();
};

}
#endif
