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

// Usage example for the MimoProcessor with PortAudio.

#include <iostream>

#include "apf/stringtools.h"

// First the policies ...
#include "apf/portaudio_policy.h"
#include "apf/posix_thread_policy.h"
// ... then the SimpleProcessor.
#include "simpleprocessor.h"

using apf::str::S2A;
using apf::str::A2S;

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    std::cerr << "Error: too few arguments!" << std::endl;
    std::cout << "Usage: " << argv[0]
     << " inchannels outchannels samplerate blocksize [device-id]" << std::endl;

    std::cout << "\nList of devices:" << std::endl;
    std::cout << SimpleProcessor::device_info() << std::endl;
    return 42;
  }

  apf::parameter_map e;
  e.set("threads", 2);

  e.set("in_channels", argv[1]);
  e.set("out_channels", argv[2]);
  e.set("sample_rate", argv[3]);
  e.set("block_size", argv[4]);
  e.set("device_id", argv[5]);

  SimpleProcessor engine(e);

  sleep(60);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
