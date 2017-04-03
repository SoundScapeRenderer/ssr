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

/** OscSender
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
class OscSender : public Subscriber
{
  private:
    // address of server (only used for client -> server connection)
    lo::Address _server_address;
    // ServerThread to send from specific port (client <-> server)
    lo::ServerThread _send_from;
    // vector of client address objects (only for server -> clients)
    std::vector<lo::Address> _client_addresses;
    // reference to handler
    OscHandler& _handler;

  public:
    OscSender(OscHandler& handler, int port);
    OscSender(int port);
    ~OscSender();
    void start();
    void stop();

    // server -> clients
    void poll_clients(); // poll the list of clients


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

//    // declare set_server_address() as friend of class OscHandler
//    friend void OscHandler::set_server_address(OscSender& self, lo::Address
//        server_address);
//    // declare server_address() as friend of class OscHandler
//    friend lo::Address OscHandler::server_address(OscSender& self);
    // declare set_server_address() as friend of class OscHandler
    void set_server_address(lo::Address server_address);
    // declare server_address() as friend of class OscHandler
    lo::Address server_address();
};

} // namespace ssr
