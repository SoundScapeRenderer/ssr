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
/// Abstract %Tracker class (definition).

#ifndef SSR_TRACKER_H
#define SSR_TRACKER_H

#include <atomic>

#include "api.h"

namespace ssr
{

/// Class definition
struct Tracker
{
  virtual ~Tracker() = default;  ///< destructor

  /// calibrate tracker; set the instantaneous position to be the reference
  virtual void calibrate() = 0;

  // Azimuth value at calibration in degree
  std::atomic<double> azi_correction{0.0f};

  // Current tracker data
  struct tracker_data
  {
    // Sensor orientation in quaternions
    struct Rot {std::atomic<double> x{}, y{}, z{}, w{1}; } orientation;
  };

};

}  // namespace ssr

#endif
