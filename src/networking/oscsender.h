/**
 * Header for OscSender, declaring a class, responsible for sending OSC
 * messages and subscribing to the SSR's Publisher.
 * @file oscsender.h
 */

#ifndef OSC_SENDER_H
#define OSC_SENDER_H

#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*
#endif

#include <vector>
#include <map>
#include <thread>
#include <lo/lo_cpp.h>
#include "oscclient.h"
#include "ssr_global.h" // for VERBOSE, MessageLevel
#include "subscriber.h"
#include "apf/parameter_map.h"

namespace ssr
{

/**
 * OscSender
 * This class holds a Publisher and an OscHandler reference, while implementing
 * the Subscriber interface.
 * The Publisher is subscribed to, using its interface to send out OSC messages
 * on all events it emmits.
 * @todo implement sending levels periodically using apf/cxx_thread_policy.h
 * @todo implement polling clients periodically using apf/cxx_thread_policy.h
 */
class OscSender : public Subscriber
{
  private:
    // reference to controller
    Publisher& _controller;
    // reference to handler
    OscHandler& _handler;
    // server object (client)
    OscClient _server;
    // bool, indicating if subscribed to _controller
    bool _is_subscribed;
    // vector of pointers to OscClient objects (server)
    std::vector<OscClient*> _clients;
    // map of id/parameter_map pairs for new sources (server)
    std::map<id_t, apf::parameter_map> _new_sources;
    // thread used for calling poll_all_clients continuously
    std::thread _poll_thread;
    const unsigned int _poll_milliseconds{1000};
    bool _poll_all_clients;
    typedef std::map<id_t,float> source_level_map_t;
    source_level_map_t _source_levels;
    float _master_level;

    void poll_all_clients();
    void remove_all_clients();
    bool is_new_source(id_t id); //< check, if source id is in _new_sources
    bool is_complete_source(id_t id); //< check, if source is complete
    void send_new_source_message_from_id(id_t id); //< creates a 'new source' OSC message

  public:
    OscSender(Publisher& controller, OscHandler& handler);
    ~OscSender();

    void start();
    void stop();
    void set_server_address(std::string& hostname, std::string& port);
    lo::Address& server_address();
    void set_server_message_level(MessageLevel message_level);
    bool server_is_default();
    bool is_server(std::string& hostname, std::string& port);
    void set_message_level(const unsigned int& message_level);
    void add_client(std::string hostname, std::string port, ssr::MessageLevel
        message_level);
    void set_client_message_level(std::string hostname, std::string port,
        ssr::MessageLevel message_level);
    void deactivate_client(std::string hostname, std::string port);
    void send_to_server(std::string path, lo::Message message);
    void send_to_server(lo::Bundle bundle);
    void send_to_client(lo::Address address, std::string path, lo::Message
        message);
    void send_to_client(lo::Address address, lo::Bundle bundle);
    void send_to_all_clients(std::string path, lo::Message message);
    void send_to_all_clients(lo::Bundle bundle);

    void send_levels();

    // Subscriber Interface
    virtual void set_loudspeakers(const Loudspeaker::container_t&
        loudspeakers);
    virtual void new_source(id_t id);
    virtual void delete_source(id_t id);
    virtual void delete_all_sources();
    virtual bool set_source_position(id_t id, const Position& position);
    virtual bool set_source_position_fixed(id_t id, const bool& fix);
    virtual bool set_source_orientation(id_t id, const Orientation&
        orientation);
    virtual bool set_source_gain(id_t id, const float& gain);
    virtual bool set_source_mute(id_t id, const bool& mute);
    virtual bool set_source_name(id_t id, const std::string& name);
    virtual bool set_source_properties_file(id_t id, const std::string& name);
    virtual bool set_source_model(id_t id, const Source::model_t& model);
    virtual bool set_source_port_name(id_t id, const std::string& port_name);
    virtual bool set_source_file_name(id_t id, const std::string& file_name);
    virtual bool set_source_file_channel(id_t id, const int& file_channel);
    virtual bool set_source_file_length(id_t id, const long int& length);
    virtual void set_reference_position(const Position& position);
    virtual void set_reference_orientation(const Orientation& orientation);
    virtual void set_reference_offset_position(const Position& position);
    virtual void set_reference_offset_orientation(const Orientation&
        orientation);
    virtual void set_master_volume(float volume);
    virtual void set_source_output_levels(id_t id, float* first, float* last);
    virtual void set_processing_state(bool state);
    virtual void set_transport_state( const std::pair<bool, jack_nframes_t>&
        state);
    virtual void set_auto_rotation(bool auto_rotate_sources);
    virtual void set_decay_exponent(float exponent);
    virtual void set_amplitude_reference_distance(float distance);
    virtual void set_master_signal_level(float level);
    virtual void set_cpu_load(float load);
    virtual void set_sample_rate(int sample_rate);
    virtual bool set_source_signal_level(const id_t id, const float& level);

};

} // namespace ssr
#endif
