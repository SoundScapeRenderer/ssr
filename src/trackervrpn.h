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

#include <atomic>
#include <thread>
#include <string>
#include <memory>  // for std::unique_ptr

#include <vrpn_Tracker.h>

#include "ssr_global.h"  // for ERROR, VERBOSE
#include "tracker.h"  // base class


namespace ssr
{

namespace api { struct Publisher; }

/// VRPN tracker
class TrackerVrpn : public Tracker, public vrpn_Tracker_Remote
{
  public:
    using ptr_t = std::unique_ptr<TrackerVrpn>;

    virtual ~TrackerVrpn();

    /// "named constructor"
    static ptr_t create(api::Publisher& controller, const std::string& ports);

    virtual void calibrate() override
    {
        VERBOSE2("Calibrate.");
        Tracker::azi_correction = _current_azimuth + 90;
    }

  private:
    /// constructor
    TrackerVrpn(api::Publisher& controller, const std::string& ports);

    api::Publisher& _controller;
    double _current_azimuth;

    std::string _address;

    /// VRPN callback function
    static void VRPN_CALLBACK handle_tracker(void *userdata, const vrpn_TRACKERCB t);

    void update(const Tracker::Tracker_data& _data);

    // thread related stuff
    std::thread _tracker_thread;
    std::atomic<bool> _stop_thread; // thread stop flag
    void _start() override; ///< start the tracking thread
    void _stop() override;  ///< stop the tracking thread
    void _thread() override;  // thread main function
};

}  // namespace ssr

#endif
