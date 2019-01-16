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
/// Global variables and typedefs and preprocessor macros for the SSR.

#ifndef SSR_GLOBAL_H
#define SSR_GLOBAL_H

#include <iostream>

/// Contains variables and typedefs for the SSR
namespace ssr
{

/** Verbosity level.
 * @arg 0 - Only errors and warnings are shown.
 * @arg 1 - A few more messages are shown.
 * @arg 2 - Quite a lot messages are shown.
 * @arg 3 - Even messages which can repeat many times per second are shown.
 *   This is a lot of messages!
 **/
extern int verbose;
extern float c;         ///< speed of sound (meters per second)
extern float c_inverse; ///< reciprocal value of c
/// time to sleep after connecting a new soundfile with ecasound
extern unsigned int usleeptime;

}  // namespace ssr

// some preprocessor macros:

/// turn the argument into a string
#define __STR__(x)  #x
/// workaround to evaluate the argument and pass the result to __STR__
#define __XSTR__(x) __STR__(x)
/// make a string with filename and line number
#define __POS__ "(" __FILE__ ":" __XSTR__(__LINE__) ")"

/// Write message to stdout, if ssr::verbose is non-zero.
#define VERBOSE(msg) __VERBOSE(msg,1)
/// Write message to stdout, if ssr::verbose is greater than 1.
#define VERBOSE2(msg) __VERBOSE(msg,2)
/// Write message to stdout, if ssr::verbose is greater than 2.
#define VERBOSE3(msg) __VERBOSE(msg,3)

/// Write message to stdout, if ssr::verbose >= level
#define __VERBOSE(msg,level) __VERBOSE_NOLF(msg,level) << std::endl

// TODO: the NOLF thing is a little ugly, still searching for a better thing ...

/// NOLF = no line feed
#define __VERBOSE_NOLF(msg,level) \
  if (ssr::verbose >= (level)) std::cout << msg << std::flush
/// like VERBOSE(), but without line feed at the end.
#define VERBOSE_NOLF(msg) __VERBOSE_NOLF(msg,1)
/// like VERBOSE2(), but without line feed at the end.
#define VERBOSE2_NOLF(msg) __VERBOSE_NOLF(msg,2)
/// like VERBOSE3(), but without line feed at the end.
#define VERBOSE3_NOLF(msg) __VERBOSE_NOLF(msg,3)

/// Write a warning message to stderr.
#define WARNING(msg) \
  std::cerr << "Warning: " << msg << " " __POS__ << std::endl
/// Write an error message to stderr.
#define ERROR(msg) \
  std::cerr << "Error: " << msg << " " __POS__ << std::endl

#endif
