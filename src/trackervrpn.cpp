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

#include "ssr_global.h"
#include "api.h"  // for Publisher
#include "legacy_orientation.h"  // for Orientation
#include "apf/math.h"  // for rad2deg()


ssr::TrackerVrpn::TrackerVrpn(api::Publisher& controller
    , const std::string& address)
  : vrpn_Tracker_Remote(address.c_str())
  , _controller(controller)
  , _current_azimuth(0.0)
  , _stop_thread(false)
{
  SSR_VERBOSE("Starting VRPN tracker \"" << address << "\"");

  // TODO: what exactly is this supposed to do?
  //this->set_update_rate(120);

  // register vrpn callback
  this->vrpn_Tracker_Remote::register_change_handler(&Tracker::current_data, this->handle_tracker);

  // start thread
  _start();

  // wait until tracker has started
  vrpn_SleepMsecs(50);

  this->calibrate();
}

ssr::TrackerVrpn::~TrackerVrpn()
{
  // Probably not absolutely necessary
  this->vrpn_Tracker_Remote::unregister_change_handler(&Tracker::current_data, this->handle_tracker);
  // stop thread
  _stop();
  // Release any ports?
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
    SSR_ERROR(e.what());
  }
  return temp;
}

void VRPN_CALLBACK
ssr::TrackerVrpn::handle_tracker(void *userdata, const vrpn_TRACKERCB t)
{
  //this function gets called when the tracker's POSITION xform is updated

  ssr::Tracker::Tracker_data *_data = reinterpret_cast<ssr::Tracker::Tracker_data*>(userdata);

  // https://github.com/vrpn/vrpn/wiki/Client-side-VRPN-Devices#type-definitions
  double x = t.quat[0];
  double y = t.quat[1];
  double z = t.quat[2];
  double w = t.quat[3];

  // write back to tracker_data
  _data->x = x;
  _data->y = y;
  _data->z = z;
  _data->w = w;
}

void
ssr::TrackerVrpn::update(const Tracker::Tracker_data& _data)
{
  double x = _data.x;
  double y = _data.y;
  double z = _data.z;
  double w = _data.w;

  // yaw (z-axis rotation), from https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
  double yaw = std::atan2(2.0f * (w * z + x * y),
                          1.0f - 2.0f * (y * y + z * z));
  _current_azimuth = apf::math::rad2deg(yaw);

  _controller.take_control()->reference_rotation_offset(
      Orientation(-_current_azimuth + Tracker::azi_correction));
}

void
ssr::TrackerVrpn::_start()
{
  // create thread
  _tracker_thread = std::thread(&ssr::TrackerVrpn::_thread, this);
  SSR_VERBOSE("Starting tracker ...");
}

void
ssr::TrackerVrpn::_stop()
{
  _stop_thread = true;
  if (_tracker_thread.joinable())
  {
    SSR_VERBOSE2("Stopping tracker...");
    _tracker_thread.join();
  }
}

void
ssr::TrackerVrpn::_thread()
{
  while (!_stop_thread)
  {
    // This calls the callback
    this->vrpn_Tracker_Remote::mainloop();
    // Push updates to SSR
    this->update(*Tracker::get_tracker_data());
    // TODO: make this configurable:
    vrpn_SleepMsecs(10);
  };
}
