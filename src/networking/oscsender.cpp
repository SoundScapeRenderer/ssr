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
 * Constructor used to create client objects
 * @param controller reference to a Publisher object
 * @param handler reference to an OscHandler object
 * @param port_out an integer describing the port number to be used
 * for outgoing traffic
 */
ssr::OscSender::OscSender(Publisher& controller, OscHandler& handler, int
    port_out)
  : _controller(controller)
  , _handler(handler)
  , _send_from(port_out)
  , _server_address("none", "50002")
{
  VERBOSE("OscSender: Initialized.");
}

/**
 * Destructor
 */
ssr::OscSender::~OscSender()
{}

/** Function to start the OscSender object
 * This subscribes the OscSender to the Publisher and starts the
 * lo::ServerThread to send from
 */
void ssr::OscSender::start()
{
  _controller.subscribe(this);
  _is_subscribed = true;
  // check if lo::ServerThread is valid
  if (!_send_from.is_valid()) {
    ERROR("OscSender: ServerThread could not be started!");
  }
  _send_from.set_callbacks([this]()
    {
      VERBOSE("OscSender: Started ServerThread for sending messages.");
    },
    []()
    {
      VERBOSE2("OscSender: ServerThread cleanup.");
    }
  );
  VERBOSE("OscSender: url = " << _send_from.url() << ".");
  _send_from.start();
  if (is_server())
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
  if (is_server())
  {
    _poll_all_clients = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

/**
 * Returns true, if the instance of OscHandler is a 'client', false otherwise.
 * @return true, if _oschandler.mode() returns 'client', false otherwise.
 */
bool ssr::OscSender::is_client()
{
  if(_handler.mode() == "client")
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Returns true, if the instance of OscHandler is a 'server', false otherwise.
 * @return true, if _oschandler.mode() returns 'server', false otherwise.
 */
bool ssr::OscSender::is_server()
{
  if(_handler.mode() == "server")
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Sends a '/poll' message to all client instances listed in _client_addresses,
 * then makes the thread calling this function sleep for 100 milliseconds
 */
void ssr::OscSender::poll_all_clients()
{
  VERBOSE("OscSender: Starting to poll all clients.");
  while(_poll_all_clients)
  {
    send_to_all_clients("/poll", lo::Message());
    //TODO find better solution to compensate for execution time
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  VERBOSE2("OscSender: Stopped polling all clients.");
}

/**
 * Function to return OscSender's _server_address
 * @return a lo::Address object representing the current server for this client
 */
lo::Address ssr::OscSender::server_address()
{
  lo::Address server(_server_address.hostname(), _server_address.port());
  return server;
}

/**
 * Function to set OscSender's _server_address
 * @param server_address a lo::Address to be used as _server_address
 */
void ssr::OscSender::set_server_address(std::string hostname, std::string port)
{
  _server_address = lo::Address(hostname, port);
  VERBOSE2("OscSender: Setting up new server address: "<<
      _server_address.hostname() << ":" << _server_address.port() << ".");
}

/**
 * Function to send a lo::Message to the lo:Address setup as server using a
 * predefined path
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_server(std::string path, lo::Message message)
{
  if((_server_address.hostname().compare("none") != 0) &&
      (_server_address.port().compare("50002") != 0))
  {
    _server_address.send_from(_send_from, path, message.types(), message);
    VERBOSE3("OscSender: Sending ["<< path << ", " << message.types() <<
        "] to server " << _server_address.hostname() << ":" <<
        _server_address.port() << ".");
  }
}

/**
 * Function to send a lo::Bundle to the lo:Address setup as server
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_server(lo::Bundle bundle)
{
  _server_address.send_from(_send_from, bundle);
  VERBOSE3("OscSender: Sending bundle (" << bundle.length() <<
      " messages) to server " << _server_address.hostname() << ":" <<
      _server_address.port() << ".");
}

/**
 * Function to send a lo::Message to a client.
 * @param address a lo:Address that will be sent to, when found in
 * _client_addresses.
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_client(lo::Address address, std::string path,
    lo::Message message)
{
  for (const auto& client: _client_addresses)
  {
    if(client->hostname() == address.hostname() && client->port() ==
        address.port())
    {
      client->send_from(_send_from, path, message.types(), message);
      VERBOSE3("OscSender: Sending ["<< path << ", " << message.types() <<
          "] to client " << address.hostname() << ":" << address.port() <<
          ".");
    }
  }
}

/**
 * Function to send a lo::Bundle to a client.
 * @param address a lo:Address that will be sent to, when found in
 * _client_addresses.
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_client(lo::Address address, lo::Bundle bundle)
{
  for (const auto& client: _client_addresses)
  {
    if(client->hostname() == address.hostname() && client->port() ==
        address.port())
    {
      client->send_from(_send_from, bundle);
      VERBOSE3("OscSender: Sending bundle to client " << address.hostname() <<
          ":" << address.port() << "."); }
  }
}

/**
 * Function to send a lo::Message to all clients setup in _client_addresses
 * vector.
 * @param path a std::string defining the path to send to
 * @param message a predefined lo::Messge object to be sent
 */
void ssr::OscSender::send_to_all_clients(std::string path, lo::Message message)
{
  for (const auto& client_address: _client_addresses)
  {
    VERBOSE3("OscSender: Sending ["<< path << ", " << message.types() <<
        "] to client " << client_address->hostname() << ":" <<
        client_address->port() << ".");
    client_address->send_from(_send_from, path, message.types(), message);
  }
}

/**
 * Sends a lo::Bundle to all clients setup in _client_addresses vector.
 * @param bundle a predefined lo::Bundle object to be sent
 */
void ssr::OscSender::send_to_all_clients(lo::Bundle bundle)
{
  for (const auto& client_address: _client_addresses)
  {
    client_address->send_from(_send_from, bundle);
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
        _new_sources.at(id).has_key("volume") &&
        _new_sources.at(id).has_key("channel") &&
        _new_sources.at(id).has_key("properties_file") &&
        _new_sources.at(id).has_key("position_fixed") &&
        _new_sources.at(id).has_key("orientation_fixed") &&
        _new_sources.at(id).has_key("muted"))||
        (_new_sources.at(id).has_key("name") &&
        _new_sources.at(id).has_key("model") &&
        _new_sources.at(id).has_key("file_name_or_port_number") &&
        _new_sources.at(id).has_key("x") &&
        _new_sources.at(id).has_key("y") &&
        _new_sources.at(id).has_key("orientation") &&
        _new_sources.at(id).has_key("volume") &&
        _new_sources.at(id).has_key("position_fixed") &&
        _new_sources.at(id).has_key("orientation_fixed") &&
        _new_sources.at(id).has_key("muted")))
      is_complete = true;
  }
  return is_complete;
}

/**
 * Creates a lo::Message used to create a new source on clients. It will
 * collect all parameters from a parameter_map in _new_sources according to an
 * id.
 * @param id id_t id of the local source a message will be created for.
 */
void ssr::OscSender::send_new_source_message_from_id(id_t id)
{
  lo::Message message;
  if(_new_sources.at(id).has_key("name") &&
      _new_sources.at(id).has_key("model") &&
      _new_sources.at(id).has_key("file_name_or_port_number") &&
      _new_sources.at(id).has_key("x") &&
      _new_sources.at(id).has_key("y") &&
      _new_sources.at(id).has_key("orientation") &&
      _new_sources.at(id).has_key("volume") &&
      _new_sources.at(id).has_key("channel") &&
      _new_sources.at(id).has_key("properties_file") &&
      _new_sources.at(id).has_key("position_fixed") &&
      _new_sources.at(id).has_key("orientation_fixed") &&
      _new_sources.at(id).has_key("muted") &&
      _new_sources.at(id).size() == 12 )
  {
    message.add_string(_new_sources.at(id).get<std::string>("name", ""));
    message.add_string(_new_sources.at(id).get<std::string>("model", ""));
    message.add_string(_new_sources.at(id).get<std::string>("file_name_or_port_number",
          ""));
    message.add_float(_new_sources.at(id).get<float>("x", 0.0));
    message.add_float(_new_sources.at(id).get<float>("y", 0.0));
    message.add_float(_new_sources.at(id).get<float>("orientation", 0.0));
    message.add_float(_new_sources.at(id).get<float>("volume", 0.0));
    message.add_int32(_new_sources.at(id).get<int>("channel", 1));
    message.add_string(_new_sources.at(id).get<std::string>("properties_file", ""));
    (_new_sources.at(id).get<bool>("position_fixed", false)?
     message.add_true(): message.add_false());
    (_new_sources.at(id).get<bool>("orientation_fixed", false)?
     message.add_true(): message.add_false());
    (_new_sources.at(id).get<bool>("muted", false)?
     message.add_true(): message.add_false());
    this->send_to_all_clients("/source/new", message);
  }
  else if(_new_sources.at(id).has_key("name") &&
    _new_sources.at(id).has_key("model") &&
    _new_sources.at(id).has_key("file_name_or_port_number") &&
    _new_sources.at(id).has_key("x") &&
    _new_sources.at(id).has_key("y") &&
    _new_sources.at(id).has_key("orientation") &&
    _new_sources.at(id).has_key("volume") &&
    _new_sources.at(id).has_key("position_fixed") &&
    _new_sources.at(id).has_key("orientation_fixed") &&
    _new_sources.at(id).has_key("muted") &&
    _new_sources.at(id).size() == 10 )
  {
    message.add_string(_new_sources.at(id).get<std::string>("name", ""));
    message.add_string(_new_sources.at(id).get<std::string>("model", ""));
    message.add_string(_new_sources.at(id).get<std::string>("file_name_or_port_number",
          ""));
    message.add_float(_new_sources.at(id).get<float>("x", 0.0));
    message.add_float(_new_sources.at(id).get<float>("y", 0.0));
    message.add_float(_new_sources.at(id).get<float>("orientation", 0.0));
    message.add_float(_new_sources.at(id).get<float>("volume", 0.0));
    (_new_sources.at(id).get<bool>("position_fixed", false)?
     message.add_true(): message.add_false());
    (_new_sources.at(id).get<bool>("orientation_fixed", false)?
     message.add_true(): message.add_false());
    (_new_sources.at(id).get<bool>("muted", false)?
     message.add_true(): message.add_false());
    this->send_to_all_clients("/source/new", message);
  }
}

/**
 * Adds a new client to the vector of clients
 * @param hostname std::string representing the hostname of a client
 * @param port std::string representing the port of a client
 */
void ssr::OscSender::add_client(std::string hostname, std::string port)
{
  _client_addresses.push_back(new lo::Address(hostname, port));
  VERBOSE2("OscSender: Added client " << hostname << ":" << port << ".");
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
  if(is_server())
  {
    if(!this->is_new_source(id))
      _new_sources.insert(make_pair(id, apf::parameter_map()));
    if(is_complete_source(id))
      this->send_new_source_message_from_id(id);
  }
  else if(is_client())
  {
    lo::Message message;
    int32_t message_id = id;
    message.add_int32(message_id);
    this->send_to_server("/update/source/new", message);
  }
}

/**
 * Subscriber function called, when Publisher deleted a source.
 * On server: Sends out OSC message all clients to delete source with given id.
 * On client: Sends out OSC message about successful deletion of source with id
 * to server and erases complementing volume level from _source_levels.
 * @param id id_t representing the source
 */
void ssr::OscSender::delete_source(id_t id)
{
  lo::Message message;
  int32_t message_id = id;
  message.add_int32(message_id);
  if(is_server())
  {
    this->send_to_all_clients("/source/delete", message);
  }
  else if (is_client())
  {
    this->send_to_server("/update/source/delete", message);
    _source_levels.erase(id);
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
  lo::Message message;
  message.add_int32(0);
  if(is_server())
    this->send_to_all_clients("/source/delete", message);
  else if(is_client())
    this->send_to_server("/update/source/delete", message);

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
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<float>("x", position.x);
      _new_sources.at(id).set<float>("y", position.y);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_float(position.x);
      message.add_float(position.y);
      this->send_to_all_clients("/source/position", message);
    }
  }
  else if (is_client())
  {
    message.add_int32(message_id);
    message.add_float(position.x);
    message.add_float(position.y);
    this->send_to_server("/update/source/position", message);
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
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<bool>("position_fixed", fixed);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_bool(fixed);
      this->send_to_all_clients("/source/position_fixed", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_bool(fixed);
    this->send_to_server("/update/source/position_fixed", message);
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
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<float>("orientation", orientation.azimuth);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_float(orientation.azimuth);
      this->send_to_all_clients("/source/orientation", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_float(orientation.azimuth);
    this->send_to_server("/update/source/orientation", message);
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's volume.
 * On server: Sends out OSC message to set volume of given source on all
 * clients. If id is found in _new_sources, the volume will be stored in the
 * parameter_map for id and an OSC message will be send to clients only, if the
 * source is complete.
 * On client: Sends out OSC to server message about the successful updating of
 * the source's volume.
 * @param id id_t representing the source
 * @param gain new volume of source
 * @return true
 */
bool ssr::OscSender::set_source_gain(id_t id, const float& gain)
{
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<float>("volume", gain);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_float(gain);
      this->send_to_all_clients("/source/volume", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_float(gain);
    this->send_to_server("/update/source/volume", message);
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
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<bool>("mute", mute);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_bool(mute);
      this->send_to_all_clients("/source/mute", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_bool(mute);
    this->send_to_server("/update/source/mute", message);
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
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<std::string>("name", name);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_string(name);
      this->send_to_all_clients("/source/name", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_string(name);
    this->send_to_server("/update/source/name", message);
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
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<std::string>("properties_file", name);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_string(name);
      this->send_to_all_clients("/source/properties_file", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_string(name);
    this->send_to_server("/update/source/properties_file", message);
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
  lo::Message message;
  message.add_float(exponent);
  if(is_server())
  {
    this->send_to_all_clients("/scene/decay_exponent", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/scene/decay_exponent", message);
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
  lo::Message message;
  message.add_float(distance);
  if(is_server())
  {
    this->send_to_all_clients("/scene/amplitude_reference_distance",
        message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/scene/amplitude_reference_distance",
        message);
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
  lo::Message message;
  int32_t message_id = id;
  std::string tmp_model;
  tmp_model = apf::str::A2S(model);
  if (tmp_model == "") return false;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<std::string>("model", tmp_model);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_string(tmp_model);
      this->send_to_all_clients("/source/model", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_string(tmp_model);
    this->send_to_server("/update/source/model", message);
  }
  return true;
}

/**
 * Subscriber function called, when Publisher set a source's port_name.
 * On server: Sends out OSC message to set the port_name of given source on all
 * clients. If id is found in _new_sources, the port_name will be stored in the
 * parameter_map for id and an OSC message will be send to clients only, if the
 * source is complete.
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
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<std::string>("port_name", port_name);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_string(port_name);
      this->send_to_all_clients("/source/port_name", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_string(port_name);
    this->send_to_server("/update/source/port_name", message);
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
  lo::Message message;
  int32_t message_id = id;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<std::string>("file_name_or_port_number",
          file_name);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_string(file_name);
      this->send_to_all_clients("/source/file_name_or_port_number", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_string(file_name);
    this->send_to_server("/update/source/file_name_or_port_number", message);
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
  lo::Message message;
  int32_t message_id = id;
  int32_t message_file_channel = file_channel;
  if(is_server())
  {
    if(is_new_source(id))
    {
      _new_sources.at(id).set<int>("channel", file_channel);
      if(is_complete_source(id))
        this->send_new_source_message_from_id(id);
    }
    else
    {
      message.add_int32(message_id);
      message.add_int32(message_file_channel);
      this->send_to_all_clients("/source/channel", message);
    }
  }
  else if(is_client())
  {
    message.add_int32(message_id);
    message.add_int32(message_file_channel);
    this->send_to_server("/update/source/channel", message);
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
  lo::Message message;
  int32_t message_id = id;
  int32_t message_length = length;
  if(is_client())
  {
    message.add_int32(message_id);
    message.add_int32(message_length);
    this->send_to_server("/update/source/length", message);
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
  lo::Message message;
  message.add_float(position.x);
  message.add_float(position.y);
  if(is_server())
  {
    this->send_to_all_clients("/reference/position", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/reference/position", message);
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
  lo::Message message;
  message.add_float(orientation.azimuth);
  if(is_server())
  {
    this->send_to_all_clients("/reference/orientation", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/reference/orientation", message);
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
  lo::Message message;
  message.add_float(position.x);
  message.add_float(position.y);
  if(is_server())
  {
    this->send_to_all_clients("/reference_offset/position", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/reference_offset/position", message);
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
  lo::Message message;
  message.add_float(orientation.azimuth);
  if(is_server())
  {
    this->send_to_all_clients("/reference_offset/orientation", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/reference_offset/orientation", message);
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
  lo::Message message;
  message.add_float(apf::math::linear2dB(volume));
  if(is_server())
  {
    this->send_to_all_clients("/scene/volume", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/scene/volume", message);
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
  lo::Message message;
  message.add_bool(state);
  if(is_server())
  {
    this->send_to_all_clients("/processing/state", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/processing/state", message);
  }
}

/**
 * Subscriber function called, when Publisher set the processing state.
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
  lo::Message message_state;
  lo::Message message_time;
  message_state.add_bool(state.first);
  message_time.add_string(std::to_string(state.second));
  if(is_server())
  {
    lo::Bundle bundle({
        {"/transport/state", message_state},
        {"/transport/seek", message_time}
    });

// TODO: debug this!
//    this->send_to_all_clients(bundle);
  }
  else if(is_client())
  {
    lo::Bundle bundle({
        {"/update/transport/state", message_state},
        {"/update/transport/seek", message_time}
    });
// TODO: debug this!
//    this->send_to_server(bundle);
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
  lo::Message message;
  message.add_bool(auto_rotate_sources);
  if(is_server())
  {
    this->send_to_all_clients("/scene/auto_rotate_sources", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/scene/auto_rotate_sources", message);
  }
}

/**
 * Subscriber function called, when Publisher set cpu_load.
 * On server: Does nothing.
 * On client: Sends out OSC message to server indicating the cpu_load.
 * @param load a float representing the current cpu load
 * @todo implement receiver callback in
 * OscReceiver::add_server_to_client_methods()
 */
void ssr::OscSender::set_cpu_load(float load)
{
  lo::Message message;
  message.add_float(load);
  if(is_client())
  {
    this->send_to_server("/update/cpu_load", message);
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
  lo::Message message;
  int32_t message_sr = sr;
  message.add_int32(message_sr);
  if(is_server())
  {
    this->send_to_all_clients("/scene/sample_rate", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/scene/sample_rate", message);
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
  lo::Message message;
  float message_level(apf::math::linear2dB(level));
  message.add_float(message_level);
  if(is_server())
  {
    this->send_to_all_clients("/scene/master_signal_level", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/scene/master_signal_level", message);
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
  lo::Message message;
  int32_t message_id = id;
  message.add_int32(message_id);
  message.add_float(apf::math::linear2dB(level));
  if(is_server())
  {
    this->send_to_all_clients("/source/volume", message);
  }
  else if(is_client())
  {
    this->send_to_server("/update/source/level", message);
  }
  return true;
}

