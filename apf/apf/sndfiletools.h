/******************************************************************************
 Copyright (c) 2012-2016 Institut für Nachrichtentechnik, Universität Rostock
 Copyright (c) 2006-2012 Quality & Usability Lab
                         Deutsche Telekom Laboratories, TU Berlin

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*******************************************************************************/

// https://AudioProcessingFramework.github.io/

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
