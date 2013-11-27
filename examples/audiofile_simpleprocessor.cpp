/******************************************************************************
 * Copyright © 2012-2013 Institut für Nachrichtentechnik, Universität Rostock *
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

// Usage example for the MimoProcessor reading from and writing to multichannel
// audio files.

#include "apf/mimoprocessor_file_io.h"

// First the policies ...
#include "apf/pointer_policy.h"
#include "apf/posix_thread_policy.h"
// ... then the SimpleProcessor.
#include "simpleprocessor.h"

int main(int argc, char *argv[])
{
  const size_t blocksize = 65536;

  if (argc < 4)
  {
    std::cerr << "Error: too few arguments!" << std::endl;
    std::cout << "Usage: " << argv[0]
      << " infilename outfilename outchannels [threads]" << std::endl;
    return 42;
  }

  std::string infilename = argv[1];
  std::string outfilename = argv[2];

  apf::parameter_map e;
  e.set("threads", (argc >= 5) ? argv[4] : "1");

  SndfileHandle in(infilename, SFM_READ);
  e.set("in_channels", in.channels());
  e.set("out_channels", apf::str::S2RV<int>(argv[3]));

  e.set("block_size", blocksize);
  e.set("sample_rate", in.samplerate());

  SimpleProcessor engine(e);

  return mimoprocessor_file_io(engine, infilename , outfilename);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
