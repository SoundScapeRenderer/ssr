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

// Example showing how to connect JACK ports.

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
      , apf::jack_policy, apf::posix_thread_policy>
{
  public:
    MyProcessor()
    {
      Input::Params in_params;

      in_params["port_name"] = "no_initial_connection";
      this->add(in_params);

      in_params["port_name"] = "initial_connection";
      in_params["connect_to"] = "system:capture_1";
      this->add(in_params);

      Output::Params out_params;

      out_params["port_name"] = "connect_before_activate";
      this->add(out_params);

      out_params["port_name"] = "connect_after_activate";
      this->add(out_params);

      out_params["port_name"] = "port with spaces";
      this->add(out_params);
    }
};

int main()
{
  MyProcessor processor;

  processor.connect_ports("MimoProcessor:connect_before_activate"
      , "system:playback_1");

  sleep(5);

  processor.activate();

  sleep(2);

  processor.connect_ports("MimoProcessor:connect_after_activate"
      , "system:playback_1");

  processor.connect_ports("MimoProcessor:port with spaces"
      , "system:playback_2");

  sleep(30);
  processor.deactivate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
