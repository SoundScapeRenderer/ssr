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

#include "tracker.h"  // base class
#include "razor-ahrs/RazorAHRS.h"
#include "publisher.h"

namespace ssr
{

/// Razor AHRS tracker
class TrackerRazor : public Tracker
{
  public:
    using ptr_t = std::unique_ptr<TrackerRazor>;

    /// "named constructor"
    static ptr_t create(Publisher& controller, const std::string& ports);

    /// destructor
    ~TrackerRazor()
    {
      if (_tracker != nullptr) delete _tracker;
    }

  virtual void calibrate() { _az_corr = _current_azimuth + 90.0f; }

  private:
    /// constructor
    TrackerRazor(Publisher& controller, const std::string& ports);

    /// Razor AHRS callback functions
    void on_data(const float ypr[])
    {
      _current_azimuth = ypr[0];
      if (_init_az_corr)
      {
        calibrate();
        _init_az_corr = false;
      }
      _controller.set_reference_orientation(Orientation(-_current_azimuth + _az_corr));
    }
    void on_error(const std::string &msg) { ERROR("Razor AHRS: " << msg); }

    Publisher& _controller;
    volatile float _current_azimuth;
    volatile float _az_corr;
    volatile bool _init_az_corr;

    RazorAHRS* _tracker;
};

}  // namespace ssr

#endif // TRACKERRAZOR_H
