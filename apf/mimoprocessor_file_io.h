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

  int blocksize = processor.block_size();

  // these matrices are used for de-interleaving and interleaving
  fixed_matrix<float> m_in(blocksize, in.channels());
  fixed_matrix<float> m_in_transpose(in.channels(), blocksize);
  fixed_matrix<float> m_out(blocksize, processor.out_channels());
  fixed_matrix<float> m_out_transpose(processor.out_channels(), blocksize);

  processor.activate();

  StopWatch watch("processing");

  size_t actual_frames = 0;
  while ((actual_frames = in.readf(m_in.data(), blocksize)) != 0)
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
