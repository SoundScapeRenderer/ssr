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
/// Provides helper functions for calculations concerning time.

#ifndef SSR_TIMETOOLS_H
#define SSR_TIMETOOLS_H


/// Provides helper functions for calculations concerning time.
namespace timetools
{

/** Returns time interval in seconds between two time instances
 * @param start time instance where interval started
 * @param stop time instance where interval started
 * @return time interval in seconds
 **/
inline float get_time_interval(struct timeval start, struct timeval stop)
{
  return static_cast<float>(stop.tv_sec - start.tv_sec
      + (stop.tv_usec - start.tv_usec) / 1000000.0);
}

inline bool is_time_stamp_valid(struct timeval time_stamp)
{
  if (time_stamp.tv_sec == 0 && time_stamp.tv_usec == 0) return false;
  else return true;
}

}  // namespace timetools

#endif
