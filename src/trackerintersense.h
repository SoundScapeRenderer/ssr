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
/// InterSense tracker (definition).

#ifndef SSR_TRACKERINTERSENSE_H
#define SSR_TRACKERINTERSENSE_H

#include <thread>
#include <isense.h>
#include <string>
#include <memory>
#include <stdexcept> // for std::runtime_error

#include "tracker.h"

namespace ssr
{

namespace api { struct Publisher; }

/// Intersense InertiaCube3 head tracker
class TrackerInterSense : public Tracker
{
  public:
    using ptr_t = std::unique_ptr<TrackerInterSense>;

    virtual ~TrackerInterSense(); ///< destructor

    /// "named constructor"
    static ptr_t create(api::Publisher& controller
        , const std::string& ports = "", const unsigned int read_interval = 20);

    virtual void calibrate();

  private:
    /// constructor
    TrackerInterSense(api::Publisher& controller, const std::string& ports
        , const unsigned int read_interval);

    api::Publisher& _controller;

    /// interval in ms to wait after each read cycle
    unsigned int _read_interval;


    ISD_TRACKER_HANDLE _tracker_h; ///< tracker handle

    // thread related stuff
    std::thread _tracker_thread;
    std::thread::id _thread_id;

    volatile bool _stop_thread; // thread stop flag
    void _start(); ///< start the tracking thread
    void _stop();  ///< stop the tracking thread

    static void* _thread_starter(void*);
    void* _thread(void*);  // thread main function
};

}  // namespace ssr

#endif
