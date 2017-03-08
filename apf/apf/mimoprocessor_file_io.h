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
/// Helper function for multichannel soundfile reading/writing

#include <sndfile.hh>  // C++ interface to libsndfile
#include <iostream>

#include "stopwatch.h"
#include "apf/container.h"  // for fixed_matrix

namespace apf
{

/// Use MimoProcessor-based object with multichannel audio file input and output
/// @param processor Object derived from MimoProcessor
/// @param infilename Input audio file name
/// @param outfilename Output audio file name (will be overwritten if it exists)
template<typename Processor>
int mimoprocessor_file_io(Processor& processor
    , const std::string& infilename
    , const std::string& outfilename)
{
  std::cout << "Opening file \"" << infilename << "\" ..." << std::endl;

  auto in = SndfileHandle(infilename, SFM_READ);

  if (int err = in.error())
  {
    std::cout << in.strError() << std::endl;
    return err;
  }

  if (in.samplerate() != static_cast<int>(processor.sample_rate()))
  {
    std::cout << "Samplerate mismatch!" << std::endl;
    return 42;
  }

  if (in.channels() != processor.in_channels())
  {
    std::cout << "Input channel mismatch!" << std::endl;
    return 666;
  }

  auto out = SndfileHandle(outfilename, SFM_WRITE
      , in.format(), processor.out_channels(), in.samplerate());

  if (int err = out.error())
  {
    std::cout << out.strError() << std::endl;
    return err;
  }

  auto format_info = SF_FORMAT_INFO();
  format_info.format = in.format();
  in.command(SFC_GET_FORMAT_INFO, &format_info, sizeof(format_info));

  std::cout << "format: " << format_info.name << std::endl;

  std::cout << "frames: " << in.frames()
    << " (" << in.frames()/in.samplerate() << " seconds)" << std::endl;
  std::cout << "channels: " << in.channels() << std::endl;
  std::cout << "samplerate: " << in.samplerate() << std::endl;

  auto blocksize = processor.block_size();

  // these matrices are used for de-interleaving and interleaving
  fixed_matrix<float> m_in(blocksize, size_t(in.channels()));
  fixed_matrix<float> m_in_transpose(size_t(in.channels()), blocksize);
  fixed_matrix<float> m_out(blocksize, size_t(processor.out_channels()));
  fixed_matrix<float> m_out_transpose(
      size_t(processor.out_channels()), blocksize);

  processor.activate();

  StopWatch watch("processing");

  sf_count_t actual_frames = 0;
  while ((actual_frames = in.readf(m_in.data()
          , static_cast<sf_count_t>(blocksize))) != 0)
  {
    m_in_transpose.set_channels(m_in.slices);

    processor.audio_callback(blocksize
        , m_in_transpose.get_channel_ptrs()
        , m_out_transpose.get_channel_ptrs());

    m_out.set_channels(m_out_transpose.slices);

    out.writef(m_out.data(), actual_frames);
  }

  //out.writeSync();  // write cache buffers to disk

  processor.deactivate();

  return 0;
}

}  // namespace apf

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
