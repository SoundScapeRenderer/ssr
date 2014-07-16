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

// Usage example for the MimoProcessor with JACK.

#include <iostream>

#include "apf/stringtools.h"

// First the policies ...
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"
// ... then the SimpleProcessor.
#include "simpleprocessor.h"

using apf::str::S2A;
using apf::str::A2S;

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    std::cerr << "Error: too few arguments!" << std::endl;
    std::cout << "Usage: " << argv[0]
      << " inchannels inportprefix outchannels [outportprefix]" << std::endl;
    return 42;
  }

  apf::parameter_map e;
  e.set("name", "my_engine");
  e.set("threads", 2);

  e.set("in_channels", argv[1]);
  e.set("in_port_prefix", argv[2]);
  e.set("out_channels", argv[3]);
  if (argc > 4) e.set("out_port_prefix", argv[4]);
  else          e.set("out_port_prefix", "system:playback_");

  SimpleProcessor engine(e);

  sleep(2);

  SimpleProcessor::Input::Params p3;
  p3.set("port_name", "another_port_just_for_fun");
  engine.add(p3);

  sleep(60);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
