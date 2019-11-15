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
#include <thread>

#include "geometry.h"


namespace ssr
{

/// Class definition
class Tracker
{
  public:
    Tracker(api::Publisher& controller) : _controller(controller){};  ///< constructor

    virtual ~Tracker() = default;  ///< destructor

    /// reset tracker; set the instantaneous position to be the reference
    virtual void reset()
    {
      SSR_VERBOSE2("Tracker reset.");
      this->_correction_rot = this->_current_rot;
    }

    // Update SSR
    virtual void update()
    {
      ssr::quat r = inverse(_correction_rot) * inverse(_current_rot);
      _controller.take_control()->reference_rotation_offset(r);
    }

    protected:
      // Current tracker data (should be atomic)
      ssr::quat _current_rot;
      ssr::quat _correction_rot;

    private:
      api::Publisher& _controller;
};

}  // namespace ssr

#endif
