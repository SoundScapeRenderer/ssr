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
/// Razor AHRS tracker (definition).
/// See http://dev.qu.tu-berlin.de/projects/sf-razor-9dof-ahrs/wiki

#ifndef SSR_TRACKERRAZOR_H
#define SSR_TRACKERRAZOR_H

#include "legacy_orientation.h"  // for Orientation
#include "ssr_global.h"  // for ERROR, VERBOSE
#include "tracker.h"  // base class
#include "apf/math.h"  // for deg2rad()

#include "razor-ahrs/RazorAHRS.h"

namespace ssr
{

namespace api { struct Publisher; }

/// Razor AHRS tracker
class TrackerRazor : public Tracker
{
  public:
    using ptr_t = std::unique_ptr<TrackerRazor>;

    /// "named constructor"
    static ptr_t create(api::Publisher& controller, const std::string& ports);

    /// destructor
    ~TrackerRazor()
    {
      if (_tracker != nullptr) delete _tracker;
    }

  private:
    /// constructor
    TrackerRazor(api::Publisher& controller, const std::string& ports);

    RazorAHRS* _tracker;

    /// Razor AHRS callback functions
    void on_data(const float ypr[])
    {
      Tracker::_current_rot = ypr2quaternion(apf::math::deg2rad(ypr[0]),
                                             apf::math::deg2rad(ypr[1]),
                                             apf::math::deg2rad(ypr[2]));
      // Push updates to SSR
      Tracker::update();
    }
    void on_error(const std::string &msg) { SSR_ERROR("Razor AHRS: " << msg); }

};

}  // namespace ssr

#endif // TRACKERRAZOR_H
