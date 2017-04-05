/**
 * Header for OscSender, declaring a class, responsible for sending OSC
 * messages and subscribing to the SSR's Publisher.
 * @file oscsender.h
 */

#ifndef OSC_SENDER_H
#define OSC_SENDER_H
#endif

#ifdef HAVE_CONFIG_H
#include <config.h> // for ENABLE_*
#endif

#include <vector>
#include <lo/lo.h>
#include <lo/lo_cpp.h>

#include "ssr_global.h"
#include "subscriber.h"

namespace ssr
{

/**
 * OscSender
 * This class holds a Publisher and an OscHandler reference, while implementing
 * the Subscriber interface.
 * The Publisher is subscribed to, using its interface to send out OSC messages
 * on all events it emmits.
 */
class OscSender : public Subscriber
{
  private:
    // address of server (client)
    lo::Address _server_address;
    // ServerThread to send from specific port (client|server)
    lo::ServerThread _send_from;
    // vector of client address objects (server)
    std::vector<lo::Address> _client_addresses;
    // reference to handler
    OscHandler& _handler; // TODO: really needed?
    // reference to controller
    Publisher& _controller;
    bool _is_subscribed;
    std::string _mode;
    typedef std::map<id_t,float> source_level_map_t;
    source_level_map_t _source_levels;
    float _master_level;

  public:
    OscSender(Publisher& controller, OscHandler& handler, int port_out);
    OscSender(Publisher& controller, OscHandler& handler, int port_out,
        std::vector<lo::Address> client_addresses);
    ~OscSender();

    void start();
    void stop();
    void set_server_address(lo::Address server_address);
    lo::Address server_address();
    void poll_clients();
    void send_to_server(lo::Message message);
    void send_to_server(lo::Bundle bundle);

    void update_all_clients(std::string str);
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
    virtual void set_transport_state(
        const std::pair<bool, jack_nframes_t>& state);
    virtual void set_auto_rotation(bool auto_rotate_sources);
    virtual void set_decay_exponent(float exponent);
    virtual void set_amplitude_reference_distance(float distance);
    virtual void set_master_signal_level(float level);
    virtual void set_cpu_load(float load);
    virtual void set_sample_rate(int sample_rate);
    virtual bool set_source_signal_level(const id_t id, const float& level);

};

} // namespace ssr
