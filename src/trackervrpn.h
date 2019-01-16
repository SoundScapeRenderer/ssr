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
/// VRPN tracker (definition).

#ifndef SSR_TRACKERVRPN_H
#define SSR_TRACKERVRPN_H

#include <pthread.h>
#include <string>
#include <memory>  // for std::unique_ptr

#include <vrpn_Tracker.h>

#include "tracker.h"

namespace ssr
{

namespace api { struct Publisher; }

/// VRPN tracker
class TrackerVrpn : public vrpn_Tracker_Remote, public Tracker
{
  public:
    using ptr_t = std::unique_ptr<TrackerVrpn>;

    virtual ~TrackerVrpn();

    /// "named constructor"
    static ptr_t create(api::Publisher& controller, const std::string& ports);

    virtual void calibrate();
    void set_value(double azi);

  private:
    TrackerVrpn(api::Publisher& controller, const std::string& ports);

    api::Publisher& _controller;

    std::string _address;

    double _current_azimuth;

    volatile bool _stopped; ///< stops the tracking thread

    float _az_corr; ///< correction of the azimuth due to calibration
    void _start(); ///< start the tracking thread
    void _stop();  ///< stop the tracking thread
    // thread related stuff
    pthread_t _thread_id;
    static void* _thread(void*);
    void* thread(void*);

    static void VRPN_CALLBACK _vrpn_change_handler(void* arg
        , const vrpn_TRACKERCB t);

    void vrpn_change_handler(const vrpn_TRACKERCB t);
};

}  // namespace ssr

#endif
