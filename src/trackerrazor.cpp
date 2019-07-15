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
/// Razor AHRS tracker (implementation).
/// See http://dev.qu.tu-berlin.de/projects/sf-razor-9dof-ahrs/wiki

#include "trackerrazor.h"

ssr::TrackerRazor::TrackerRazor(api::Publisher& controller
    , const std::string& ports)
  : Tracker()
  , _controller(controller)
  , _current_azimuth(0.0)
  , _tracker(nullptr)
{
  if (ports == "")
  {
    throw std::runtime_error("No serial port(s) specified!");
  }
  SSR_VERBOSE("Initializing Razor AHRS ...");

  std::istringstream iss(ports);
  std::string port;
  while (iss >> port)
  {
    if (port != "")
    {
      SSR_VERBOSE_NOLF("Trying port " << port << " ... ");
      try {
        _tracker = new RazorAHRS(port,
            std::bind(&TrackerRazor::on_data, this, std::placeholders::_1),
            std::bind(&TrackerRazor::on_error, this, std::placeholders::_1),
            RazorAHRS::YAW_PITCH_ROLL);
      }
      catch(std::runtime_error& e)
      {
        SSR_VERBOSE("failure! (" << std::string(e.what()) + ")");
        continue;
      }

      SSR_VERBOSE("success!");
      break; // stop trying
    }
  }
  if (_tracker == nullptr)
  {
    throw std::runtime_error("Could not open serial port!");
  }

  // wait until tracker has started
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  this->calibrate();
}

ssr::TrackerRazor::ptr_t
ssr::TrackerRazor::create(api::Publisher& controller, const std::string& ports)
{
  ptr_t temp; // temp = NULL
  try
  {
    temp.reset(new TrackerRazor(controller, ports));
  }
  catch(std::runtime_error& e)
  {
    SSR_ERROR(e.what());
  }
  return temp;
}

void ssr::TrackerRazor::update(const Tracker::Tracker_data &_data)
{
  _current_azimuth = _data.yaw;
  _controller.take_control()->reference_rotation_offset(
  Orientation(-_current_azimuth + Tracker::azi_correction));
}
