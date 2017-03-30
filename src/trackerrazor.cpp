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

ssr::TrackerRazor::TrackerRazor(Publisher& controller, const std::string& ports)
  : Tracker()
  , _controller(controller)
  , _current_azimuth(0.0f)
  , _az_corr(90.0f)
  , _init_az_corr(true)
  , _tracker(nullptr)
{
  if (ports == "")
  {
    throw std::runtime_error("No serial port(s) specified!");
  }
  VERBOSE("Initializing Razor AHRS ...");

  std::istringstream iss(ports);
  std::string port;
  while (iss >> port)
  {
    if (port != "")
    {
      VERBOSE_NOLF("Trying port " << port << " ... ");
      try {
        _tracker = new RazorAHRS(port,
            std::bind(&TrackerRazor::on_data, this, std::placeholders::_1),
            std::bind(&TrackerRazor::on_error, this, std::placeholders::_1),
            RazorAHRS::YAW_PITCH_ROLL);
      }
      catch(std::runtime_error& e)
      {
        VERBOSE("failure! (" << std::string(e.what()) + ")");
        continue;
      }

      VERBOSE("success!");
      break; // stop trying
    }
  }
  if (_tracker == nullptr)
  {
    throw std::runtime_error("Could not open serial port!");
  }
}

ssr::TrackerRazor::ptr_t
ssr::TrackerRazor::create(Publisher& controller, const std::string& ports)
{
  ptr_t temp; // temp = NULL
  try
  {
    temp.reset(new TrackerRazor(controller, ports));
  }
  catch(std::runtime_error& e)
  {
    ERROR(e.what());
  }
  return temp;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
