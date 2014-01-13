/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the Audio Processing Framework (APF).                 *
 *                                                                            *
 * The APF is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The APF is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 *                                 http://AudioProcessingFramework.github.com *
 ******************************************************************************/

/// @file
/// Some tools for the use with libsndfile.

#ifndef APF_SNDFILETOOLS_H
#define APF_SNDFILETOOLS_H

#include <sndfile.hh>  // C++ bindings for libsndfile

#include "apf/stringtools.h"

namespace apf
{

/** Load sound file, throw exception if something's wrong
 * @param name file name
 * @param sample_rate expected sample rate
 * @param channels expected number of channels
 * @throw std::logic_error whenever something is wrong
 **/
inline SndfileHandle load_sndfile(const std::string& name, size_t sample_rate
    , size_t channels)
{
  // TODO: argument for read/write?

  if (name == "")
  {
    throw std::logic_error("apf::load_sndfile(): Empty file name!");
  }

  auto handle = SndfileHandle(name, SFM_READ);

#if 0
  // rawHandle() is available since libsndfile version 1.0.24
  if (!handle.rawHandle())
#else
  if (!handle.channels())
#endif
  {
    throw std::logic_error(
        "apf::load_sndfile(): \"" + name + "\" couldn't be loaded!");
  }

  if (sample_rate)
  {
    const size_t true_sample_rate = handle.samplerate();
    if (sample_rate != true_sample_rate)
    {
      throw std::logic_error("apf::load_sndfile(): \"" + name
          + "\" has sample rate " + str::A2S(true_sample_rate) + " instead of "
          + str::A2S(sample_rate) + "!");
    }
  }

  if (channels)
  {
    const size_t true_channels = handle.channels();

    if (channels != true_channels)
    {
      throw std::logic_error("apf::load_sndfile(): \"" + name + "\" has "
          + str::A2S(true_channels) + " channels instead of "
          + str::A2S(channels) + "!");
    }
  }

  return handle;
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
