#include "oschandler.h"

ssr::OscHandler::OscHandler(Publisher& controller, int port)
  : _controller(controller),
  , _serverThread(port),
  , _serverAddress(port)
{}

ssr::OscHandler::~OscHandler()
{}

//ssr::OscHandler::setPort(int port)
//{
//  
//}
