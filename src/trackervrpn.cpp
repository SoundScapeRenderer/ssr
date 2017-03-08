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

#include "publisher.h"
#include "ssr_global.h"

#include "apf/math.h"  // for pi

ssr::TrackerVrpn::TrackerVrpn(Publisher& controller, const std::string& address)
  : vrpn_Tracker_Remote(address.c_str())
  , _controller(controller)
  , _stopped(false)
  , _az_corr(90.0f)
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
ssr::TrackerVrpn::create(Publisher& controller, const std::string& ports)
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
  _az_corr = _current_azimuth + 90.0f;
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

  // calculate yaw (azimuth) from quaternions
  double azi = atan2(2*(w*x+y*z),1-2*(x*x+y*y))*(180/apf::math::pi<double>());

  _current_azimuth = azi;
  _controller.set_reference_orientation(Orientation(-azi + _az_corr));
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
