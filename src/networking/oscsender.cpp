/**
 * Implementation of oscsender.h
 * @file oscsender.cpp
 */

#include <chrono>
#include "oschandler.h"
#include "oscsender.h"
#include "publisher.h"
#include "apf/stringtools.h"
#include "apf/math.h"

/**
 * Constructor used to create OscSender objects
 * @param controller reference to a Publisher object
 * @param handler reference to an OscHandler object
 */
ssr::OscSender::OscSender(Publisher& controller, OscHandler& handler)
  : _controller(controller)
  , _handler(handler)
  , _server("none", "50001", MessageLevel::SERVER)
{
  VERBOSE("OscSender: Initialized.");
}

/**
 * Destructor
 */
ssr::OscSender::~OscSender()
{}

/**
 * Function to start the OscSender object
 * This subscribes the OscSender to the Publisher and starts the
 * lo::ServerThread to send from
 */
void ssr::OscSender::start()
{
  _controller.subscribe(this);
  _is_subscribed = true;
  if (_handler.is_server())
  {
    _poll_all_clients = true;
    std::thread _poll_thread(&OscSender::poll_all_clients, this);
    _poll_thread.detach();
  }
}

/**
 * Function to stop the OscSender object
 * This unsubscribes the OscSender from the Publisher and stops the client
 * polling thread
 */
void ssr::OscSender::stop()
{
  _controller.unsubscribe(this);
  _is_subscribed = false;
  if (_handler.is_server())
  {
    _poll_all_clients = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(_poll_milliseconds));
    remove_all_clients();
  }
}

/**
 * Returns true, if the _server.address() is the default (setup at initialization)
 * @return true, if _server.address() is the default, false otherwise.
 */
bool ssr::OscSender::server_is_default()
{
  if((_server.hostname().compare("none") == 0) &&
      (_server.port().compare("50001") == 0))
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Returns true, if the _server.address()'s hostname and port are the same as
 * the provided.
 * @param hostname a reference to a std::string representing the hostname
 * @param port a reference to a std::string representing the port
 * @return true, if _server.address() has the same hostname and port, false
 * otherwise.
 */
bool ssr::OscSender::is_server(std::string& hostname, std::string& port)
{
  if((_server.hostname().compare(hostname) == 0) &&
      (_server.port().compare(port) == 0))
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Sends a '/poll' message to all client instances listed in _clients, then
 * makes the thread calling this function sleep for 1000 milliseconds
 */
void ssr::OscSender::poll_all_clients()
{
  VERBOSE("OscSender: Starting to poll all clients.");
  while(_poll_all_clients)
  {
    for(const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(), "/poll", "");
      }
    }
    //TODO find better solution to compensate for execution time
    std::this_thread::sleep_for(std::chrono::milliseconds(_poll_milliseconds));
  }
  VERBOSE2("OscSender: Stopped polling all clients.");
}

/**
 * Function to return OscSender's _server.address()
 * @return a lo::Address object representing the current server for this client
 */
lo::Address& ssr::OscSender::server_address()
{
  return _server.address();
}

/**
 * Function to set OscSender server's _message_level (client).
 * @param MessageLevel enum representing the new message level
 */
void ssr::OscSender::set_server_message_level(MessageLevel message_level)
{
  _server.set_message_level(message_level);
}

/**
 * Function to set OscSender's _server address
 * @param hostname a std::string& to be used as hostname
 * @param port a std::string& to be used as port
 */
void ssr::OscSender::set_server_address(std::string& hostname, std::string& port)
{
  _server.set_address(hostname, port);
  VERBOSE2("OscSender: Setting up new server address: "<<
      _server.hostname() << ":" << _server.port() << ".");
}

/**
 * Function to send a lo::Message to the lo:Address setup as server using a
 * predefined path
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_server(std::string path, lo::Message message)
{
  if(!server_is_default())
  {
    _server.address().send_from(_handler.server(), path, message.types(), message);
    VERBOSE3("OscSender: Sending ["<< path << ", " << message.types() <<
        "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Function to send a lo::Bundle to the lo:Address setup as server
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_server(lo::Bundle bundle)
{
  if(!server_is_default())
  {
    _server.address().send_from(_handler.server(), bundle);
    VERBOSE3("OscSender: Sending bundle (" << bundle.length() <<
        " messages) to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Function to send a lo::Message to a client.
 * @param address a lo:Address that will be sent to, when found in
 * _clients.
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_client(lo::Address address, std::string path,
    lo::Message message)
{
  for (const auto& client: _clients)
  {
    if(client->hostname() == address.hostname() && client->port() ==
        address.port())
    {
      client->address().send_from(_handler.server(), path, message.types(),
          message);
      VERBOSE3("OscSender: Sending ["<< path << ", " << message.types() <<
          "] to client " << address.hostname() << ":" << address.port() <<
          ".");
    }
  }
}

/**
 * Function to send a lo::Bundle to a client.
 * @param address a lo:Address that will be sent to, when found in
 * _clients.
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_client(lo::Address address, lo::Bundle bundle)
{
  for (const auto& client: _clients)
  {
    if(client->hostname() == address.hostname() && client->port() ==
        address.port())
    {
      client->address().send_from(_handler.server(), bundle);
      VERBOSE3("OscSender: Sending bundle to client " << address.hostname() <<
          ":" << address.port() << "."); }
  }
}

/**
 * Function to send a lo::Message to all clients setup in _clients
 * vector.
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_all_clients(std::string path, lo::Message message)
{
  for (const auto& client: _clients)
  {
    client->address().send_from(_handler.server(), path, message.types(),
        message);
    VERBOSE3("OscSender: Sending ["<< path << ", " << message.types() <<
        "] to client " << client->hostname() << ":" <<
        client->port() << ".");
  }
}

/**
 * Sends a lo::Bundle to all clients setup in _clients vector.
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_all_clients(lo::Bundle bundle)
{
  for (const auto& client: _clients)
  {
    client->address().send_from(_handler.server(), bundle);
    VERBOSE3("OscSender: Sending bundle to all clients.");
  }
}

/**
 * Checks keys of _new_sources map against provided source id
 * @return true, if source id is found in _new_sources, false otherwise
 */
bool ssr::OscSender::is_new_source(id_t id)
{
  if (_new_sources.empty())
    return false;
  //TODO: introduce exception handling for this call
  if(_new_sources.find(id) != _new_sources.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Checks, whether the settings stored in _new_sources under given source id
 * are sufficient to send a '/source/new' message to all clients
 * @return true, if sufficient fields are available, false otherwise
 */
bool ssr::OscSender::is_complete_source(id_t id)
{
  bool is_complete = false;
  if(is_new_source(id))
  {
    if((_new_sources.at(id).has_key("name") &&
        _new_sources.at(id).has_key("model") &&
        _new_sources.at(id).has_key("file_name_or_port_number") &&
        _new_sources.at(id).has_key("x") &&
        _new_sources.at(id).has_key("y") &&
        _new_sources.at(id).has_key("orientation") &&
        _new_sources.at(id).has_key("gain") &&
        _new_sources.at(id).has_key("file_channel") &&
        _new_sources.at(id).has_key("properties_file") &&
        _new_sources.at(id).has_key("position_fixed") &&
        _new_sources.at(id).has_key("orientation_fixed") &&
        _new_sources.at(id).has_key("mute") &&
        _new_sources.at(id).size() == 12)||
        (_new_sources.at(id).has_key("name") &&
        _new_sources.at(id).has_key("model") &&
        _new_sources.at(id).has_key("file_name_or_port_number") &&
        _new_sources.at(id).has_key("x") &&
        _new_sources.at(id).has_key("y") &&
        _new_sources.at(id).has_key("orientation") &&
        _new_sources.at(id).has_key("gain") &&
        _new_sources.at(id).has_key("position_fixed") &&
        _new_sources.at(id).has_key("orientation_fixed") &&
        _new_sources.at(id).has_key("mute") &&
        _new_sources.at(id).size() == 10))
      is_complete = true;
  }
  return is_complete;
}

/**
 * Creates a message used to create a new source on clients. It will
 * collect all parameters from a parameter_map in _new_sources according to an
 * id.
 * @param id id_t id of the local source a message will be created for.
 */
void ssr::OscSender::send_new_source_message_from_id(id_t id)
{
  if(_new_sources.at(id).has_key("name") &&
      _new_sources.at(id).has_key("model") &&
      _new_sources.at(id).has_key("file_name_or_port_number") &&
      _new_sources.at(id).has_key("x") &&
      _new_sources.at(id).has_key("y") &&
      _new_sources.at(id).has_key("orientation") &&
      _new_sources.at(id).has_key("gain") &&
      _new_sources.at(id).has_key("file_channel") &&
      _new_sources.at(id).has_key("properties_file") &&
      _new_sources.at(id).has_key("position_fixed") &&
      _new_sources.at(id).has_key("orientation_fixed") &&
      _new_sources.at(id).has_key("mute"))
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(), "/source/new",
            "sssffffis"+
            _handler.bool_to_message_type(_new_sources.at(id).get<bool>(
                "position_fixed", false))
            +_handler.bool_to_message_type(_new_sources.at(id).get<bool>(
                "orientation_fixed", false))
            +_handler.bool_to_message_type(_new_sources.at(id).get<bool>(
                "mute", false)),
            _new_sources.at(id).get<std::string>("name", "").c_str(),
            _new_sources.at(id).get<std::string>("model", "").c_str(),
            _new_sources.at(id).get<std::string>(
              "file_name_or_port_number","").c_str(),
            _new_sources.at(id).get<float>("x", 0.0),
            _new_sources.at(id).get<float>("y", 0.0),
            _new_sources.at(id).get<float>("orientation", 0.0),
            _new_sources.at(id).get<float>("gain", 0.0),
            _new_sources.at(id).get<int>("file_channel", 1),
            _new_sources.at(id).get<std::string>("properties_file", ""));
        VERBOSE2("OscSender: Sent [/source/new, sssffffis" <<
            _handler.bool_to_message_type(_new_sources.at(id).get<bool>(
                "position_fixed", false)) <<
            _handler.bool_to_message_type(_new_sources.at(id).get<bool>(
                "orientation_fixed", false)) <<
            _handler.bool_to_message_type(_new_sources.at(id).get<bool>(
                "mute", false)) << ", " <<
            _new_sources.at(id).get<std::string>("name", "") << ", " <<
            _new_sources.at(id).get<std::string>("model", "") << ", " <<
            _new_sources.at(id).get<std::string>("file_name_or_port_number","")
            << ", " <<
            _new_sources.at(id).get<float>("x", 0.0) << ", " <<
            _new_sources.at(id).get<float>("y", 0.0) << ", " <<
            _new_sources.at(id).get<float>("orientation", 0.0) << ", " <<
            _new_sources.at(id).get<float>("gain", 0.0) << ", " <<
            _new_sources.at(id).get<int>("file_channel", 1) << ", " <<
            _new_sources.at(id).get<std::string>("properties_file", "")
             << "] to client " <<
            client->address().hostname() << ":" <<
            client->address().port() << ".");
      }
    }
    _new_sources.erase(id);
  }
  else if(_new_sources.at(id).has_key("name") &&
    _new_sources.at(id).has_key("model") &&
    _new_sources.at(id).has_key("file_name_or_port_number") &&
    _new_sources.at(id).has_key("x") &&
    _new_sources.at(id).has_key("y") &&
    _new_sources.at(id).has_key("orientation") &&
    _new_sources.at(id).has_key("gain") &&
    _new_sources.at(id).has_key("position_fixed") &&
    _new_sources.at(id).has_key("orientation_fixed") &&
    _new_sources.at(id).has_key("mute"))
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(), "/source/new",
            "sssffff"+
            _handler.bool_to_message_type(
              _new_sources.at(id).get<bool>("position_fixed", false))
            +_handler.bool_to_message_type(
              _new_sources.at(id).get<bool>("orientation_fixed", false))
            +_handler.bool_to_message_type(_new_sources.at(id).get<bool>(
                "mute", false)),
            _new_sources.at(id).get<std::string>("name", "").c_str(),
            _new_sources.at(id).get<std::string>("model", "").c_str(),
            _new_sources.at(id).get<std::string>(
              "file_name_or_port_number","").c_str(),
            _new_sources.at(id).get<float>("x", 0.0),
            _new_sources.at(id).get<float>("y", 0.0),
            _new_sources.at(id).get<float>("orientation", 0.0),
            _new_sources.at(id).get<float>("gain", 0.0));
        VERBOSE2("OscSender: Sent [/source/new, sssffff" <<
            _handler.bool_to_message_type(
              _new_sources.at(id).get<bool>( "position_fixed", false)) <<
            _handler.bool_to_message_type(
              _new_sources.at(id).get<bool>("orientation_fixed", false)) <<
            _handler.bool_to_message_type(
              _new_sources.at(id).get<bool>("mute", false)) << ", " <<
            _new_sources.at(id).get<std::string>("name", "") << ", " <<
            _new_sources.at(id).get<std::string>("model", "") << ", " <<
            _new_sources.at(id).get<std::string>("file_name_or_port_number","")
            << ", " <<
            _new_sources.at(id).get<float>("x", 0.0) << ", " <<
            _new_sources.at(id).get<float>("y", 0.0) << ", " <<
            _new_sources.at(id).get<float>("orientation", 0.0) << ", " <<
            _new_sources.at(id).get<float>("gain", 0.0)
             << "] to client " <<
            client->address().hostname() << ":" <<
            client->address().port() << ".");
      }
    }
    _new_sources.erase(id);
  }
}

/**
 * Adds a new client to the vector of clients
 * @param hostname std::string representing the hostname of a client
 * @param port std::string representing the port of a client
 */
void ssr::OscSender::add_client(std::string hostname, std::string port,
    ssr::MessageLevel message_level)
{
  bool setup = false;
  for (auto& client: _clients)
  {
    if(client && !(client->active()) &&
        !(client->address().hostname().compare(hostname)) &&
        !(client->address().port().compare(port)) )
    {
      if(client->message_level() != message_level)
        client->set_message_level(message_level);
      client->activate();
      setup = true;
      VERBOSE2("OscSender: Recycled client " << hostname << ":" << port <<
          ".");
      break;
    }
    else if(client && client->active() &&
        !(client->address().hostname().compare(hostname)) &&
        !(client->address().port().compare(port)) )
    {
      setup = true;
      VERBOSE2("OscSender: Client " << hostname << ":" << port <<
          " already active.");
    }
  }
  if (!setup)
  {
    _clients.push_back(new OscClient(hostname, port, message_level));
    VERBOSE2("OscSender: Added new client " << hostname << ":" << port <<
        " using message level " << static_cast<unsigned int>(message_level) <<
        ".");
  }
}

/**
 * Deactivate a client
 * @param hostname std::string representing the hostname of a client
 * @param port std::string representing the port of a client
 */
void ssr::OscSender::deactivate_client(std::string hostname, std::string port)
{
  for (auto& client: _clients)
  {
    if(!(client->hostname().compare(hostname)) &&
        !(client->port().compare(port)) && client->active())
    {
      client->deactivate();
      VERBOSE2("OscSender: Deactivated client " << hostname << ":" << port << ".");
    }
  }
}

/**
 * Set message level of a client
 * @param hostname std::string representing the hostname of a client
 * @param port std::string representing the port of a client
 * @param message_level ssr::MessageLevel enum representing the message level
 * to use
 */
void ssr::OscSender::set_client_message_level(std::string hostname, std::string
    port, ssr::MessageLevel message_level)
{
  for (auto& client: _clients)
  {
    if(!(client->hostname().compare(hostname)) &&
        !(client->port().compare(port)))
    {
      client->set_message_level(message_level);
      VERBOSE2("OscSender: Set message level of client '" << hostname << ":" <<
          port << "' to: " << static_cast<unsigned int>(message_level) << ".");
    }
  }
}

/**
 * Removes all clients from the vector of clients.
 */
void ssr::OscSender::remove_all_clients()
{
  for (auto client: _clients) delete client;
  _clients.clear();
  VERBOSE2("OscSender: Removed all clients.");
}


// Subscriber interface (differentiating between client and server)


/**
 * Subscriber function called, when Publisher sets up the list of loudspeakers.
 * Not implemented in OscSender.
 * @param loudpspeakers Loudspeaker container representing the list of
 * loudspeakers to set up.
 */
void ssr::OscSender::set_loudspeakers(const Loudspeaker::container_t&
    loudspeakers)
{
  (void) loudspeakers;
  VERBOSE3("OscSender: set_loudspeakers");
}

/**
 * Subscriber function called, when Publisher created a new source.
 * On server: Creates new parameter_map in _new_sources if not present
 * On client: Sends message about successful creation of source to server
 * @param id id_t representing the source
 */
void ssr::OscSender::new_source(id_t id)
{
  if(_handler.is_server())
  {
    if(!is_new_source(id))
      _new_sources.insert(make_pair(id, apf::parameter_map()));
    if(is_complete_source(id))
      send_new_source_message_from_id(id);
  }
  else if(_handler.is_client())
  {
    int32_t message_id = static_cast<int32_t>(id);
    _server.address().send_from(_handler.server(), "/update/source/new", "i",
        message_id);
    VERBOSE3("OscSender: Sent [/update/source/new, i, " << message_id <<
        "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher deleted a source.
 * On server: Sends out OSC message all clients to delete source with given id.
 * On client: Sends out OSC message about successful deletion of source with id
 * to server and erases complementing gain level from _source_levels.
 * @param id id_t representing the source
 */
void ssr::OscSender::delete_source(id_t id)
{
  int32_t message_id = static_cast<int32_t>(id);
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(), "/source/delete", "i",
            message_id);
        VERBOSE3("OscSender: Sent [/source/delete, i," << message_id <<
            "] to client " << client->address().hostname() << ":" <<
            client->address().port() << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _source_levels.erase(id);
    _server.address().send_from(_handler.server(), "/update/source/delete",
        "i", message_id);
    VERBOSE3("OscSender: Sent [/update/source/delete, i, " << message_id <<
        "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher deleted all sources.
 * On server: Sends out OSC message to delete all sources on all clients.
 * On client: Sends out OSC message about successful deletion to server.
 * Clears local _source_levels.
 */
void ssr::OscSender::delete_all_sources()
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(), "/source/delete", "i", 0);
        VERBOSE3("OscSender: Sent [/source/delete, i, 0] to client " <<
            client->address().hostname() << ":" << client->address().port()
            << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/delete",
        "i", 0);
    VERBOSE3("OscSender: Sent [/update/source/delete, i, 0] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
  _source_levels.clear();
}

/**
 * Subscriber function called, when Publisher set a source's position.
 * On server: Sends out OSC message to set position of given source on all
 * clients. If id is found in _new_sources, the Position will be stored in the
 * parameter_map for id and an OSC message will be send to clients only, if the
 * source is complete.
 * On client: Sends out OSC message about successful positioning of source to
 * server.
 * @param id id_t representing the source
 * @param position new Position of source
 * @return true
 */
bool ssr::OscSender::set_source_position(id_t id, const Position& position)
{
  int32_t message_id = static_cast<int32_t>(id);
  if(_handler.is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<float>("x", position.x);
      _new_sources.at(id).set<float>("y", position.y);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(), "/source/position",
              "iff", message_id, position.x, position.y);
          VERBOSE3("OscSender: Sent [/source/position, iff, " << message_id <<
              position.x << ", " <<  position.y << "] to client " <<
              client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/position",
        "iff", message_id, position.x, position.y);
    VERBOSE3("OscSender: Sent [/update/source/position, iff, " << message_id <<
        ", " << position.x << ", " << position.y << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's position fixed
 * state.
 * On server: Sends out OSC message to set position fixed state of given source
 * on all clients. If id is found in _new_sources, the position_fixed state
 * will be stored in the parameter_map for id and an OSC message will be send
 * to clients only, if the source is complete.
 * On client: Sends out OSC message about the source's position_fixed state to
 * server.
 * @param id id_t representing the source
 * @param fixed bool representing the source's position_fixed state
 * @return true
 */
bool ssr::OscSender::set_source_position_fixed(id_t id, const bool& fixed)
{
  int32_t message_id = static_cast<int32_t>(id);
  if(_handler.is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<bool>("position_fixed", fixed);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(),
              "/source/position_fixed", "i"+_handler.bool_to_message_type(fixed),
              message_id);
          VERBOSE3("OscSender: Sent [/source/position_fixed, i" <<
              _handler.bool_to_message_type(fixed) << ", " << message_id <<
              "] to client " << client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/position_fixed",
        "i"+_handler.bool_to_message_type(fixed), message_id);
    VERBOSE3("OscSender: Sent [/update/source/position_fixed, i"
        +_handler.bool_to_message_type(fixed) << ", " << message_id <<
        "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's orientation.
 * On server: Sends out OSC message to set Orientation of given source on all
 * clients. If id is found in _new_sources, the Orientation will be stored in the
 * parameter_map for id and an OSC message will be send to clients only, if the
 * source is complete.
 * On client: Sends out OSC message about new orientation of source to server.
 * @param id id_t representing the source
 * @param orientation new Orientation of source
 * @return true
 */
bool ssr::OscSender::set_source_orientation(id_t id , const Orientation&
    orientation)
{
  int32_t message_id = static_cast<int32_t>(id);
  float message_orientation = orientation.azimuth;
  if(_handler.is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<float>("orientation", orientation.azimuth);
      _new_sources.at(id).set<bool>("orientation_fixed", false);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(),
              "/source/orientation", "if", message_id, message_orientation);
          VERBOSE3("OscSender: Sent [/source/orientation, if, " << message_id
              << ", " << message_orientation << "] to client " <<
              client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/orientation",
        "if", message_id, message_orientation);
    VERBOSE3("OscSender: Sent [/update/source/orientation, if, " <<
        apf::str::A2S(message_id) << ", " <<
        apf::str::A2S(message_orientation) << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's gain.
 * On server: Sends out OSC message to set gain of given source on all
 * clients. If id is found in _new_sources, the gain will be stored in the
 * parameter_map for id and an OSC message will be send to clients only, if the
 * source is complete.
 * On client: Sends out OSC to server message about the successful updating of
 * the source's gain.
 * @param id id_t representing the source
 * @param gain new gain of source
 * @return true
 */
bool ssr::OscSender::set_source_gain(id_t id, const float& gain)
{
  int32_t message_id = static_cast<int32_t>(id);
  if(_handler.is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<float>("gain", gain);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(), "/source/gain",
              "if", message_id, gain);
          VERBOSE3("OscSender: Sent [/source/gain, if, " << message_id <<
              ", " << gain << "] to client " << client->address().hostname()
              << ":" << client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/gain",
        "if", message_id, gain);
    VERBOSE3("OscSender: Sent [/update/source/gain, if, " <<
        apf::str::A2S(message_id) << ", " <<  apf::str::A2S(gain) <<
        "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's mute state.
 * On server: Sends out OSC message to set mute state of given source on all
 * clients. If id is found in _new_sources, the mute state will be stored in
 * the parameter_map for id and an OSC message will be send to clients only, if
 * the source is complete.
 * On client: Sends out OSC to server message about the successful updating of
 * the source's mute state.
 * @param id id_t representing the source
 * @param mute a bool representing the mute state of source
 * @return true
 */
bool ssr::OscSender::set_source_mute(id_t id, const bool& mute)
{
  int32_t message_id = static_cast<int32_t>(id);
  if(_handler.is_server())
  {
    if(is_new_source(id) &&
        _new_sources.at(id).has_key("file_name_or_port_number"))
    {
      _new_sources.at(id).set<bool>("mute", mute);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(), "/source/mute",
              "i"+_handler.bool_to_message_type(mute), message_id);
          VERBOSE3("OscSender: Sent [/source/mute, i" <<
              _handler.bool_to_message_type(mute) << ", " << message_id <<
              "] to client " << client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/mute",
        "i"+_handler.bool_to_message_type(mute), message_id);
    VERBOSE3("OscSender: Sent [/update/source/mute, i" <<
        _handler.bool_to_message_type(mute) << ", " <<
        apf::str::A2S(message_id) << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's name.
 * On server: Sends out OSC message to set the name of given source on all
 * clients. If id is found in _new_sources, the name will be stored in
 * the parameter_map for id and an OSC message will be send to clients only, if
 * the source is complete.
 * On client: Sends out OSC to server message about the successful updating of
 * the source's name.
 * @param id id_t representing the source
 * @param name a std::string representing the name of source
 * @return true
 */
bool ssr::OscSender::set_source_name(id_t id, const std::string& name)
{
  int32_t message_id = static_cast<int32_t>(id);
  const char * message_name = name.c_str();
  if(_handler.is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<std::string>("name", name);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(), "/source/name",
              "is", message_id, message_name);
          VERBOSE3("OscSender: Sent [/source/name, is, " << message_id << ", "
              << message_name << "] to client " <<
              client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/name", "is",
        message_id, message_name);
    VERBOSE3("OscSender: Sent [/update/source/name, is, " << message_id << ", "
        << message_name << "] to server " << _server.hostname() << ":"
        << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's properties_file.
 * On server: Sends out OSC message to set the properties_file of given source
 * on all clients. If id is found in _new_sources, the properties_file will be
 * stored in the parameter_map for id and an OSC message will be send to
 * clients only, if the source is complete.
 * On client: Sends out OSC message to server about the successful updating of
 * the source's properties_file.
 * @param id id_t representing the source
 * @param name a std::string representing the properties_file of source
 * @return true
 */
bool ssr::OscSender::set_source_properties_file(id_t id, const std::string&
    name)
{
  int32_t message_id = static_cast<int32_t>(id);
  const char * file_name = name.c_str();
  if(_handler.is_server())
  {
    if(is_new_source(id) && !name.empty())
    {
      _new_sources.at(id).set<std::string>("properties_file", file_name);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(),
              "/source/properties_file", "is", message_id, file_name);
          VERBOSE3("OscSender: Sent [/source/properties_file, is, " <<
              message_id << ", " << file_name << "] to client " <<
              client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/source/properties_file", "is", message_id, file_name);
    VERBOSE3("OscSender: Sent [/update/source/properties_file, is, " <<
        message_id << ", " << file_name << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set the decay exponent.
 * On server: Sends out OSC message to set the decay_exponent on all clients.
 * On client: Sends out OSC message to server about the updating of decay_exponent
 * @param exponentn float representing the decay exponent
 * @param name a std::string representing the properties_file of source
 * @todo implement receiving in OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_decay_exponent(float exponent)
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(),
            "/scene/decay_exponent", "f", exponent);
        VERBOSE3("OscSender: Sent [/scene/decay_exponent, f, " << exponent <<
            "] to client " << client->address().hostname() << ":" <<
            client->address().port() << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/scene/decay_exponent", "f", exponent);
    VERBOSE3("OscSender: Sent [/update/scene/decay_exponent, f, " << exponent
        << "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set the amplitude reference
 * distance.
 * On server: Sends out OSC message to set the amplitude reference distance on
 * all clients.
 * On client: Sends out OSC message to server about the updating of the
 * amplitude reference distance.
 * @param distance a float representing the amplitude reference distance
 * @todo implement receiving in OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_amplitude_reference_distance(float distance)
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(),
            "/scene/amplitude_reference_distance", "f", distance);
        VERBOSE3("OscSender: Sent [/scene/decay_exponent, f, " << distance <<
            "] to client " << client->address().hostname() << ":" <<
            client->address().port() << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/scene/amplitude_reference_distance", "f", distance);
    VERBOSE3("OscSender: Sent [/update/scene/amplitude_reference_distance, f, "
        << distance << "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set a source's model.
 * On server: Sends out OSC message to set the model of given source on all
 * clients. If id is found in _new_sources, the model will be stored in the
 * parameter_map for id and an OSC message will be send to clients only, if the
 * source is complete.
 * On client: Sends out OSC message to server about the successful updating of
 * the source's model.
 * @param id id_t representing the source
 * @param model a model_t representing the model of source
 * @return true
 */
bool ssr::OscSender::set_source_model(id_t id, const Source::model_t& model)
{
  int32_t message_id = static_cast<int32_t>(id);
  std::string message_model = apf::str::A2S(model);
  if (message_model == "") return false;
  if(_handler.is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<std::string>("model", message_model);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(), "/source/model",
              "is", message_id, message_model.c_str());
          VERBOSE3("OscSender: Sent [/source/model, is, " << message_id << ", "
              << message_model << "] to client " <<
              client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/model", "is",
        message_id, message_model.c_str());
    VERBOSE3("OscSender: Sent [/update/source/model, is, " << message_id <<
        ", " << message_model << "] to server " << _server.hostname() <<
        ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's port_name.
 * On server: Does nothing, as port_name is local and depends on prefix
 * On client: Sends out OSC message to server about the successful updating of
 * the source's port_name.
 * @param id id_t representing the source
 * @param port_name a std::string representing the port_name of source
 * @return true
 * @todo check if it has to be ignored during _controller.new_source()
 * @see Controller::new_source()
 */
bool ssr::OscSender::set_source_port_name(id_t id, const std::string&
    port_name)
{
  int32_t message_id = static_cast<int32_t>(id);
  if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/port_name",
        "is", message_id, port_name.c_str());
    VERBOSE3("OscSender: Sent [/update/source/port_name, is, " << message_id <<
        ", " << port_name << "] to server " << _server.hostname() <<
        ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's
 * file_name_or_port_number member.
 * On server: Sends out OSC message to set the file_name_or_port_number of
 * given source on all clients. If id is found in _new_sources, the
 * file_name_or_port_number will be stored in the parameter_map for id and an
 * OSC message will be send to clients only, if the source is complete.
 * On client: Sends out OSC message to server about the updating of the
 * source's file_name_or_port_number.
 * @param id id_t representing the source
 * @param file_name a std::string representing the file name or
 * port number of source
 * @return true
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
bool ssr::OscSender::set_source_file_name(id_t id, const std::string&
    file_name)
{
  int32_t message_id = static_cast<int32_t>(id);
  if(_handler.is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<std::string>("file_name_or_port_number",
          file_name);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(),
              "/source/file_name_or_port_number", "is", message_id,
              file_name.c_str());
          VERBOSE3("OscSender: Sent [/source/file_name_or_port_number, is, " <<
              message_id << ", " << file_name << "] to client " <<
              client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/source/file_name_or_port_number", "is", message_id,
        file_name.c_str());
    VERBOSE3("OscSender: Sent [/update/source/file_name_or_port_number, is, "
        << message_id << ", " << file_name << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's channel.
 * On server: Sends out OSC message to set the channel of given source on all
 * clients. If id is found in _new_sources, the channel will be stored in the
 * parameter_map for id and an OSC message will be send to clients only, if the
 * source is complete.
 * On client: Sends out OSC message to server about the updating of the
 * source's channel.
 * @param id id_t representing the source
 * @param file_channel an int representing the channel in use for source
 * @return true
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
bool ssr::OscSender::set_source_file_channel(id_t id, const int& file_channel)
{
  int32_t message_id = static_cast<int32_t>(id);
  int32_t message_file_channel = static_cast<int32_t>(file_channel);
  if(_handler.is_server())
  {
    if(is_new_source(id) && file_channel > 0)
    {
      _new_sources.at(id).set<int>("file_channel", file_channel);
      if(is_complete_source(id))
        send_new_source_message_from_id(id);
    }
    else
    {
      for (const auto& client: _clients)
      {
        if(client && client->active())
        {
          client->address().send_from(_handler.server(),
              "/source/file_channel", "ii", message_id, message_file_channel);
          VERBOSE3("OscSender: Sent [/source/file_channel, ii, " << message_id
              << ", " << message_file_channel << "] to client " <<
              client->address().hostname() << ":" <<
              client->address().port() << ".");
        }
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/source/file_channel", "ii", message_id, message_file_channel);
    VERBOSE3("OscSender: Sent [/update/source/file_channel, ii, " << message_id
        << ", " << message_file_channel << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's file length.
 * On server: Does nothing, as the Publisher only emits this on creation of a
 * new source.
 * On client: Sends out OSC message to server about the updating of the
 * source's file length.
 * @param id id_t representing the source
 * @param length an int representing the source file's length
 * @return true
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
bool ssr::OscSender::set_source_file_length(id_t id, const long int& length)
{
  int32_t message_id = static_cast<int32_t>(id);
  int32_t message_length = static_cast<int32_t>(length);
  if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/source/length", "ii",
        message_id, message_length);
    VERBOSE3("OscSender: Sent [/update/source/length, ii, " << message_id
        << ", " << message_length << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set the reference position.
 * On server: Sends out OSC message to all clients to update to the given
 * reference position.
 * On client: Sends out OSC message to server about the updating of the
 * reference position.
 * @param position a Position representing the new reference position
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_reference_position(const Position& position)
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
      client->address().send_from(_handler.server(), "/reference/position",
          "ff", position.x, position.y);
      VERBOSE3("OscSender: Sent [/reference/position, ff, " << position.x <<
          ", " << position.y << "] to client " <<
          client->address().hostname() << ":" << client->address().port()
          << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/reference/position",
        "ff", position.x, position.y);
    VERBOSE3("OscSender: Sent [/update/reference/position, ff, " << position.x
        << ", " << position.y << "] to server " << _server.hostname()
        << ":" << _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set the reference orientation.
 * On server: Sends out OSC message to all clients to update to the given
 * reference orientation.
 * On client: Sends out OSC message to server about the updating of the
 * reference orientation.
 * @param orientation an Orientation representing the new reference orientation
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_reference_orientation(const Orientation& orientation)
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(),
            "/reference/orientation", "f", orientation.azimuth);
        VERBOSE3("OscSender: Sent [/reference/orientation, f, " <<
            orientation.azimuth << "] to client " <<
            client->address().hostname() << ":" << client->address().port()
            << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/reference/orientation", "f", orientation.azimuth);
    VERBOSE3("OscSender: Sent [/update/reference/orientation, f, " <<
        orientation.azimuth << "] to server " << _server.hostname() <<
        ":" << _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set the reference offset
 * position.
 * On server: Sends out OSC message to all clients to update to the given
 * reference offset position.
 * On client: Sends out OSC message to server about the updating of the
 * reference offset position.
 * @param position a Position representing the new reference offset position
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_reference_offset_position(const Position& position)
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(),
            "/reference_offset/position", "ff", position.x, position.y);
        VERBOSE3("OscSender: Sent [/reference_offset/position, ff, " <<
            position.x << ", " << position.y << "] to client " <<
            client->address().hostname() << ":" << client->address().port()
            << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/reference_offset/position", "ff", position.x, position.y);
    VERBOSE3("OscSender: Sent [/update/reference_offset/position, ff, " <<
        position.x << ", " << position.y << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set the reference offset
 * orientation.
 * On server: Sends out OSC message to all clients to update to the given
 * reference offset orientation.
 * On client: Sends out OSC message to server about the updating of the
 * reference offset orientation.
 * @param orientation an Orientation representing the new reference offset
 * orientation
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_reference_offset_orientation(const Orientation&
    orientation)
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(),
            "/reference_offset/orientation", "f", orientation.azimuth);
        VERBOSE3("OscSender: Sent [/reference_offset/orientation, f, " <<
            orientation.azimuth << "] to client " <<
            client->address().hostname() << ":" << client->address().port()
            << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/reference_offset/orientation", "f", orientation.azimuth);
    VERBOSE3("OscSender: Sent [/update/reference_offset/orientation, f, " <<
        orientation.azimuth << "] to server " << _server.hostname() <<
        ":" << _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set the master volume.
 * On server: Sends out OSC message to all clients to update to the given
 * master volume (submits in dB!).
 * On client: Sends out OSC message to server about the updating of the master
 * volume (submits in dB!).
 * @param volume float representing the new volume (linear scale)
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_master_volume(float volume)
{
  float message_volume = apf::math::linear2dB(volume);
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(), "/scene/volume", "f",
            message_volume);
        VERBOSE3("OscSender: Sent [/scene/volume, f, " << message_volume <<
            "] to client " << client->address().hostname() << ":" <<
            client->address().port() << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/scene/volume", "f",
        message_volume);
    VERBOSE3("OscSender: Sent [/update/scene/volume, f, " << message_volume <<
        "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set a source's output levels.
 * On server:
 * On client:
 * @param id id_t representing the source
 * @param first float*
 * @param last float*
 * @todo understand what should actually be done here
 */
void ssr::OscSender::set_source_output_levels(id_t id, float* first , float*
    last)
{
  (void) id;
  (void) first;
  (void) last;
}

/**
 * Subscriber function called, when Publisher set the processing state.
 * On server: Sends out OSC message to all clients to update their processing
 * state.
 * On client: Sends out OSC message to server about the update of its
 * processing state.
 * @param state bool representing the processing state
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_processing_state(bool state)
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(), "/processing/state",
            _handler.bool_to_message_type(state));
        VERBOSE3("OscSender: Sent [/processing/state, " <<
            _handler.bool_to_message_type(state) << "] to client " <<
            client->address().hostname() << ":" << client->address().port()
            << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/processing/state",
        _handler.bool_to_message_type(state));
    VERBOSE3("OscSender: Sent [/update/processing/state, " <<
        _handler.bool_to_message_type(state) << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher sets the transport state.
 * On server: Sends out OSC message to all clients to update their processing
 * state.
 * On client: Sends out OSC message to server about the update of its
 * processing state.
 * @param state a std::pair of a bool representing the processing state and
 * jack_nframes_t, representing the current location in the transport
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_transport_state( const std::pair<bool,
    jack_nframes_t>& state)
{
  int32_t message_nframes = static_cast<int32_t>(state.second);
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active() && client->message_level() >=
          MessageLevel::CLIENT)
      {
        client->address().send_from(_handler.server(), "/transport/state",
            _handler.bool_to_message_type(state.first));
        VERBOSE3("OscSender: Sent [/transport/state, " <<
            _handler.bool_to_message_type(state.first) << "] to client " <<
            client->address().hostname() << ":" << client->address().port() <<
            ".");
        client->address().send_from(_handler.server(), "/transport/seek", "i",
            message_nframes);
        VERBOSE3("OscSender: Sent [/transport/seek, i, " << message_nframes <<
            "] to client " << client->address().hostname() << ":" <<
            client->address().port() << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default() &&
      _server.message_level() == MessageLevel::GUI_SERVER)
  {
    _server.address().send_from(_handler.server(), "/update/transport/state",
        _handler.bool_to_message_type(state.first));
    VERBOSE3("OscSender: Sent [/update/transport/state, " <<
        _handler.bool_to_message_type(state.first) << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
    _server.address().send_from(_handler.server(), "/update/transport/seek", "i",
        message_nframes);
    VERBOSE3("OscSender: Sent [/update/transport/state, i, " << message_nframes
        << "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set whether to auto rotate
 * sources.
 * On server: Sends out OSC message to all clients to update their
 * auto rotate settings.
 * On client: Sends out OSC message to server about the update of its
 * auto rotate settings.
 * @param auto_rotate_sources a bool representing the current
 * auto_rotate_sources setting.
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_auto_rotation(bool auto_rotate_sources)
{
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(),
            "/scene/auto_rotate_sources",
            _handler.bool_to_message_type(auto_rotate_sources));
        VERBOSE3("OscSender: Sent [/scene/auto_rotate_sources, " <<
            _handler.bool_to_message_type(auto_rotate_sources) <<
            "] to client " << client->address().hostname() << ":" <<
            client->address().port() << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(),
        "/update/scene/auto_rotate_sources",
        _handler.bool_to_message_type(auto_rotate_sources));
    VERBOSE3("OscSender: Sent [/update/scene/auto_rotate_sources, " <<
        _handler.bool_to_message_type(auto_rotate_sources) << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set cpu_load.
 * On server: Does nothing.
 * On client: Sends out OSC message to server indicating the cpu_load.
 * @param load a float representing the current cpu load
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 * @todo implement pooling of cpu_load updates
 */
void ssr::OscSender::set_cpu_load(float load)
{
  if(_handler.is_client() && !server_is_default() && _server.message_level() ==
      MessageLevel::GUI_SERVER)
  {
    _server.address().send_from(_handler.server(), "/update/cpu_load", "f",
        load);
    VERBOSE3("OscSender: Sent [/update/cpu_load, f, " << apf::str::A2S(load)
        << "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set sample rate.
 * On server: Sends out OSC message to all clients to update their
 * sample rate.
 * On client: Sends out OSC message to server about the update of its
 * sample rate.
 * @param sr an integer representing the current sample rate.
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_sample_rate(int sr)
{
  int32_t message_sr = static_cast<int32_t>(sr);
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active())
      {
        client->address().send_from(_handler.server(), "/scene/sample_rate",
            "i", message_sr);
        VERBOSE3("OscSender: Sent [/scene/sample_rate, i, " <<
            apf::str::A2S(message_sr) << "] to client " <<
            client->address().hostname() << ":" << client->address().port()
            << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default())
  {
    _server.address().send_from(_handler.server(), "/update/scene/sample_rate",
        "i", message_sr);
    VERBOSE3("OscSender: Sent [/update/scene/sample_rate, i, " <<
        apf::str::A2S(message_sr) << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set the master signal level.
 * On server: Sends out OSC message to all clients to update their master
 * signal level.
 * On client: Sends out OSC message to server about the update of its signal
 * level.
 * @param level a float representing the current master signal level.
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_master_signal_level(float level)
{
  float message_level(apf::math::linear2dB(level));
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active() && client->message_level() >=
          MessageLevel::CLIENT)
      {
        client->address().send_from(_handler.server(),
            "/scene/master_signal_level", "f", message_level);
        VERBOSE3("OscSender: Sent [/scene/master_signal_level, f, " <<
            apf::str::A2S(message_level) << "] to client " <<
            client->address().hostname() << ":" << client->address().port() <<
            ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default() &&
      _server.message_level() == MessageLevel::GUI_SERVER)
  {
    _server.address().send_from(_handler.server(),
        "/update/scene/master_signal_level", "f", message_level);
    VERBOSE3("OscSender: Sent [/update/scene/master_signal_level, f, " <<
        apf::str::A2S(message_level) << "] to server " <<
        _server.hostname() << ":" << _server.port() << ".");
  }
}

/**
 * Subscriber function called, when Publisher set a source's signal level.
 * On server: Sends out OSC message to all clients to update a source's signal
 * level.
 * On client: Sends out OSC message to server about the update of the source's
 * signal level.
 * @param id an id_t representing the source.
 * @param level a float representing the signal level of the source.
 * @return true
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
bool ssr::OscSender::set_source_signal_level(const id_t id, const float& level)
{
  int32_t message_id = static_cast<int32_t>(id);
  float message_level(apf::math::linear2dB(level));
  if(_handler.is_server())
  {
    for (const auto& client: _clients)
    {
      if(client && client->active() && client->message_level() >=
          MessageLevel::CLIENT)
      {
        client->address().send_from(_handler.server(), "/source/level", "if",
            message_id, message_level);
        VERBOSE3("OscSender: Sent [/source/level, if, " << message_id << ", "
            << message_level << "] to client " << client->address().hostname()
            << ":" << client->address().port() << ".");
      }
    }
  }
  else if(_handler.is_client() && !server_is_default() &&
      _server.message_level() == MessageLevel::GUI_SERVER)
  {
    _server.address().send_from(_handler.server(), "/update/source/level",
        "if", message_id, message_level);
    VERBOSE3("OscSender: Sent [/update/source/level, if, " <<
        apf::str::A2S(message_id) << ", " << apf::str::A2S(message_level) <<
        "] to server " << _server.hostname() << ":" <<
        _server.port() << ".");
  }
  return true;
}

