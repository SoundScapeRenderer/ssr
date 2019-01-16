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
/// VRPN tracker (implementation).

#include "trackervrpn.h"

#include <stdexcept>  // for runtime_error
#include <cmath>  // for std::atan2()

#include "api.h"  // for Publisher
#include "legacy_orientation.h"  // for Orientation
#include "ssr_global.h"

ssr::TrackerVrpn::TrackerVrpn(api::Publisher& controller
    , const std::string& address)
  : vrpn_Tracker_Remote(address.c_str())
  , _controller(controller)
  , _stopped(false)
  , _az_corr(0.0f)
  , _thread_id(0)
{
  VERBOSE("Starting VRPN tracker \"" << address << "\"");

  // TODO: what exactly is this supposed to do?
  //this->set_update_rate(120);

  this->register_change_handler(this, _vrpn_change_handler);

  _start();

  // wait until tracker has started
  vrpn_SleepMsecs(50);

  this->calibrate();
}

ssr::TrackerVrpn::~TrackerVrpn()
{
  if (_thread_id) _stop();
}

ssr::TrackerVrpn::ptr_t
ssr::TrackerVrpn::create(api::Publisher& controller, const std::string& ports)
{
  ptr_t temp; // temp = NULL
  try
  {
    temp.reset(new TrackerVrpn(controller, ports));
  }
  catch(std::runtime_error& e)
  {
    ERROR(e.what());
  }
  return temp;
}

void
ssr::TrackerVrpn::calibrate()
{
  _az_corr = _current_azimuth;
}

void
ssr::TrackerVrpn::_start()
{
  pthread_create(&_thread_id, nullptr, _thread, this);
  VERBOSE("Starting tracker ...");
}

void
ssr::TrackerVrpn::_stop()
{
  _stopped = true;
  pthread_join(_thread_id, 0);
}

void*
ssr::TrackerVrpn::_thread(void *arg)
{
  return static_cast<TrackerVrpn*>(arg)->thread(nullptr);
}

void*
ssr::TrackerVrpn::thread(void *arg)
{
  while (!_stopped)
  {
    this->mainloop();

    // TODO: make this configurable:
    vrpn_SleepMsecs(10);
  };
  return arg;
}

void VRPN_CALLBACK
ssr::TrackerVrpn::_vrpn_change_handler(void* arg, const vrpn_TRACKERCB t)
{
  return static_cast<TrackerVrpn*>(arg)->vrpn_change_handler(t);
}

void
ssr::TrackerVrpn::vrpn_change_handler(const vrpn_TRACKERCB t)
{
  // TODO: check t.sensor for sensor number!

  // get quaternions information
  double w = t.quat[0];
  double x = t.quat[1];
  double y = t.quat[2];
  double z = t.quat[3];

  // TODO: store _az_corr as quaternion and directly set 3D rotation

  // calculate yaw (azimuth) (in radians) from quaternions
  double azi = std::atan2(2*(w*x+y*z),1-2*(x*x+y*y));

  _current_azimuth = azi;
  _controller.take_control()->reference_offset_rotation(
      Orientation(-azi + _az_corr));
}
