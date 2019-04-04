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
/// Polhemus tracker (definition).

#ifndef SSR_TRACKERPOLHEMUS_H
#define SSR_TRACKERPOLHEMUS_H

#include <atomic>
#include <thread>
#include <string>
#include <memory>
#include <stdexcept> // for std::runtime_error

#include "tracker.h"

namespace ssr
{

namespace api { struct Publisher; }

/// Polhemus Fastrak/Patriot trackers
class TrackerPolhemus : public Tracker
{
  public:
    using ptr_t = std::unique_ptr<TrackerPolhemus>;

    virtual ~TrackerPolhemus(); ///< destructor

    /// "named constructor"
    static ptr_t create(api::Publisher& controller, const std::string& type
        , const std::string& ports);

    virtual void calibrate();

  private:
    /// constructor
    TrackerPolhemus(api::Publisher& controller, const std::string& type
        , const std::string& ports);

    struct tracker_data_t
    {
      float header;
      float x;
      float y;
      float z;
      float azimuth;
      float elevation;
      float roll;

      // contructor
      tracker_data_t()
        : header(0.0f), x(0.0f), y(0.0f), z(0.0f)
        , azimuth(0.0f), elevation(0.0f), roll(0.0f)
      {}
    };

    api::Publisher& _controller;

    tracker_data_t _current_data;

    int _tracker_port;
    int _open_serial_port(const char *portname);

    float _az_corr; ///< correction of the azimuth due to calibration

    std::string::size_type _line_size;

    // thread related stuff
    std::thread _tracker_thread;

    std::atomic<bool> _stop_thread; // thread stop flag
    void _start(); ///< start the tracking thread
    void _stop();  ///< stop the tracking thread

    void _thread();  // thread main function
};

}  // namespace ssr

#endif
