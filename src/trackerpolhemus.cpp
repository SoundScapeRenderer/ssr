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
/// Polhemus tracker (implementation).

#include <unistd.h>  // for write(), fsync(), close(), ...
#include <termios.h> // for cfsetispeed(), ...
#include <fcntl.h>   // for open(), ...
#include <sstream>   // for std::stringstream
#include <poll.h>    // for poll(), pollfd, ...
#include <cassert>   // for assert()
#include <chrono>    // std::chrono::milliseconds

#include "api.h"  // for Publisher
#include "trackerpolhemus.h"
#include "ssr_global.h"
#include "apf/stringtools.h"

using apf::str::A2S;

ssr::TrackerPolhemus::TrackerPolhemus(api::Publisher& controller
    , const std::string& type, const std::string& ports)
  : Tracker()
  , _controller(controller)
  , _corr_quat()
  , _stop_thread(false)
{
  if (ports == "")
  {
    throw std::runtime_error("No serial port(s) specified!");
  }
  SSR_VERBOSE("Opening serial port for Polhemus Fastrak/Patriot ...");

  std::istringstream iss(ports);
  std::string port;
  while (iss >> port)
  {
    if (port != "")
    {
      SSR_VERBOSE_NOLF("Trying to open port " << port << " ... ");
      _tracker_port = _open_serial_port(port.c_str());
      if (_tracker_port == -1)
      {
        SSR_VERBOSE("failure!");
      }
      else
      {
        SSR_VERBOSE("success!");
        break; // stop trying
      }
    }
  }
  if (_tracker_port == -1)
  {
    throw std::runtime_error("Could not open serial port!");
  }

  // get port attributes
  struct termios tio;
  if (int errorID = tcgetattr(_tracker_port, &tio))
  {
    throw std::runtime_error("Could not get serial port attributes! Error # "
        + A2S(errorID));
  }

  // set port attributes
  cfmakeraw(&tio);
  tio.c_cflag |= CLOCAL;
  tio.c_cflag |= CREAD;

  // set port speed
  speed_t newSpeed = B115200;
  if (int errorID = cfsetispeed(&tio, newSpeed))
  {
    throw std::runtime_error(" " + A2S(errorID)
        + ": Could not set new serial port input speed to "
        + A2S(newSpeed) + ".");
  }

  if (int errorID = cfsetospeed(&tio, newSpeed))
  {
    throw std::runtime_error(" " + A2S(errorID)
        + ": Could not set new serial port output speed to "
        + A2S(newSpeed) + ".");
  }

  // set port attributes
  // must be done after setting speed!
  if (int errorID = tcsetattr(_tracker_port, TCSANOW, &tio))
  {
    throw std::runtime_error(" " + A2S(errorID)
        + ": Could not set new serial port attributes.");
  }

  // "polhemus" is allowed for backwards compatibility
  if (type == "fastrak" || type == "polhemus")
  {
    // switch to "continuous" mode
    write(_tracker_port, "C", 1);
    _line_size = 47;
  }
  else if (type == "patriot")
  {
    write(_tracker_port, "C\r", 2);
    _line_size = 60;
  }
  else
  {
    assert(false);
  }

  fsync(_tracker_port);

  _start();

  // wait until tracker has started
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  this->calibrate();
}

ssr::TrackerPolhemus::~TrackerPolhemus()
{
  // stop thread
  _stop();
  close(_tracker_port);
}

ssr::TrackerPolhemus::ptr_t
ssr::TrackerPolhemus::create(api::Publisher& controller
    , const std::string& type, const std::string& ports)
{
  ptr_t temp; // temp = NULL
  try
  {
    temp.reset(new TrackerPolhemus(controller, type, ports));
  }
  catch(std::runtime_error& e)
  {
    SSR_ERROR(e.what());
  }
  return temp;
}

int
ssr::TrackerPolhemus::_open_serial_port(const char *portname)
{
  int port;
  int flags;

  // O_NDELAY allows open even with no carrier detect (e.g. needed for Razor)
  if ((port = open(portname, O_RDWR | O_NOCTTY | O_NDELAY)) != -1)
  {
    // clear O_NDELAY to make I/O blocking again
    if (((flags = fcntl(port, F_GETFL, 0)) != -1) &&
        (fcntl(port, F_SETFL, flags & ~O_NDELAY)) != -1)
    {
      return port;
    }
  }

  // something didn't work
  close(port);
  return -1;
}

void
ssr::TrackerPolhemus::calibrate()
{
  _corr_quat = inverse(_current_quat);
}

void
ssr::TrackerPolhemus::_start()
{
  // create thread
  _tracker_thread = std::thread(&ssr::TrackerPolhemus::_thread, this);
  SSR_VERBOSE("Starting tracker ...");
}

void
ssr::TrackerPolhemus::_stop()
{
  _stop_thread = true;
  if (_tracker_thread.joinable())
  {
    SSR_VERBOSE2("Stopping tracker...");
    _tracker_thread.join();
  }
}

void
ssr::TrackerPolhemus::_thread()
{
  char c;
  std::string line;

  struct pollfd fds;
  int error;
  fds.fd = _tracker_port;
  fds.events = POLLRDNORM;

  while (!_stop_thread)
  {
    c = 0;
    line.clear();

    while (c != '\n')
    {
      error = poll(&fds, 1, 100);

      if (error < 1)
      {
        SSR_ERROR("Cannot read from serial port. Stopping Polhemus tracker.");
      }

      if ((error = read(_tracker_port, &c, 1)))
      {
        line += c;
      }
      else
      {
        SSR_ERROR("Cannot read from serial port.");
      }
    }

    if (line.size() != _line_size)
    {
      // simply keep whatever is store in _current_quat
      continue;
    }

    std::stringstream lineparse(line);

// By the way, the data coming from the Polhemus Fastrak looks like this:
//
// 02   20.40 -61.06  30.01-150.70 -42.08 156.93
// 02   20.59 -60.99  30.01-150.34 -42.24 156.96
// 02   20.81 -60.86  30.01-150.09 -42.70 156.86
//
// At the end of each line there is a <CR> and a <LF>, including them, each line
// has 47 ASCII bytes.
// The first 3 bytes identify the tracker, our Fastrak has 4 sockets, named
// "ONE", "TWO", "THREE" and "FOUR", resulting in IDs of "01 ", "02 ", "03 " and
// "04 ", respectively. When hot-plugging, the IDs sometimes change to "02d" or
// similar, which seems to be kind of an error message.
// Hot-plugging isn't such a good idea anyway.
// If several trackers are connected, their lines are interleaved.
// Then, there are 6 x 7 bytes for the actual data in 6 degrees of freedom:
// x, y, z, azimuth, elevation and tilt, we only use azimuth, the others are
// untested!
// The 7-byte-groups contain optional spaces for padding, an optional sign and a
// decimal number with 2 digits after the comma.
// The last two bytes are the abovementioned <CR> and <LF>.
//
// The data coming from the Polhemus Patriot is similar, except that only two
// sockets are available and each of the 6 degrees of freedom are represented by
// 9 bytes, leading to a total of 60 ASCII bytes.

    float header;
    float x;
    float y;
    float z;
    float azimuth;
    float elevation;
    float roll;

    // extract data
    lineparse >> header
              >> x
              >> y
              >> z
              >> azimuth
              >> elevation
              >> roll;

    // convert to quaternion (-azimuth because the tracker assumes that positive
    // z is downwards; rotation by 90 deg because we assume that the sensor is
    // mounted such that the cable goes to the left relative to the user's
    // orientation)
    _current_quat = angles2quat(-azimuth, elevation, roll)
                        * angles2quat(90, 0, 0);

    // apply calibration
    _controller.take_control()->reference_rotation_offset(
        ssr::quat(_corr_quat * _current_quat));
  };
}
