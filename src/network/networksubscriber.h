/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://spatialaudio.net/ssr                           ssr@spatialaudio.net *
 ******************************************************************************/

/// @file
/// NetworkSubscriber (definition).

#ifndef SSR_NETWORKSUBSCRIBER_H
#define SSR_NETWORKSUBSCRIBER_H

#include "subscriber.h"
#include <map>

namespace ssr
{

class Connection;

/** NetworkSubscriber.  
 * This Subscriber turns function calls to the Subscriber interface into 
 * strings (XML-messages in ASDF format) and sends it over a Connection to
 * connected clients.  
 *
 * @todo There will be a set of flags, which can filter certain
 * events. But this will be done by deriving.
 **/
class NetworkSubscriber : public Subscriber
{
  public:
    NetworkSubscriber(Connection &connection);
    ~NetworkSubscriber();

    // XXX: This is just to make the old code work.
    //	only sends string to one connection.
    void update_all_clients(std::string str);
    void send_levels();

    // Subscriber Interface
    virtual void set_loudspeakers(const Loudspeaker::container_t& loudspeakers);
    virtual void new_source(id_t id);
    virtual void delete_source(id_t id);
    virtual void delete_all_sources();
    virtual bool set_source_position(id_t id, const Position& position);
    virtual bool set_source_position_fixed(id_t id, const bool& fix);
    virtual bool set_source_orientation(id_t id, const Orientation& orientation);
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
    virtual void set_reference_offset_orientation(const Orientation& orientation);
    virtual void set_master_volume(float volume);

    virtual void set_source_output_levels(id_t id, float* first, float* last);
    virtual void set_processing_state(bool state);
    //virtual void set_transport_state(JackClient::State state);
    virtual void set_transport_state(
        const std::pair<bool, jack_nframes_t>& state);

    virtual void set_auto_rotation(bool auto_rotate_sources);
    virtual void set_decay_exponent(float exponent);
    virtual void set_amplitude_reference_distance(float distance);
    virtual void set_master_signal_level(float level);
    virtual void set_cpu_load(float load);
    virtual void set_sample_rate(int sample_rate);
    virtual bool set_source_signal_level(const id_t id, const float& level);

  private:
    Connection &_connection;

    typedef std::map<id_t,float> source_level_map_t;
    source_level_map_t           _source_levels;
    float                        _master_level;
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
