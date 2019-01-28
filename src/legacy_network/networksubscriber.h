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

#include "api.h"
#include <map>

namespace ssr
{

namespace legacy_network
{

class Connection;

/** NetworkSubscriber.
 * This Subscriber turns function calls to the Subscriber interface into
 * strings (XML-messages in ASDF format) and sends it over a Connection to
 * the connected client.
 **/
class NetworkSubscriber : public api::SceneControlEvents
                        , public api::RendererControlEvents
                        , public api::SourceMetering
                        , public api::OutputActivity
{
  public:
    explicit NetworkSubscriber(Connection &connection)
      : _connection(connection)
    {}

  private:

    // SceneControlEvents

    void auto_rotate_sources(bool) override {}
    void delete_source(id_t id) override;
    void source_position(id_t id, const Pos& position) override;
    void source_rotation(id_t id, const Rot& rotation) override;
    void source_volume(id_t id, float gain) override;
    void source_mute(id_t id, bool mute) override;
    void source_name(id_t, const std::string&) override {}
    void source_model(id_t id, const std::string& model) override;
    void source_fixed(id_t id, bool fix) override;

    void reference_position(const Pos& position) override;
    void reference_rotation(const Rot& rotation) override;

    void master_volume(float volume) override;
    void decay_exponent(float) override {}
    void amplitude_reference_distance(float) override {}

    // RendererControlEvents

    void processing(bool) override {}
    void reference_position_offset(const Pos& position) override;
    void reference_rotation_offset(const Rot& rotation) override;

    // SourceMetering

    void source_level(id_t id, float level) override;

    // OutputActivity

    void output_activity(id_t id, float* first, float* last) override;

    void _send_message(const std::string& str);
    void _send_source_message(
        const std::string& first_part, id_t id, const std::string& second_part);

    Connection &_connection;
};

}  // namespace legacy_network

}  // namespace ssr

#endif
