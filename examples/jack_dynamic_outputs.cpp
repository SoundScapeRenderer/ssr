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

// A small example of the MimoProcessor with varying JACK output ports.
// This is a stand-alone program.

#include <vector>

#include "apf/mimoprocessor.h"
#include "apf/combine_channels.h"  // for apf::CombineChannelsCopy
#include "apf/jack_policy.h"
#include "apf/dummy_thread_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::jack_policy
                    , apf::dummy_thread_policy>
{
  public:
    using Input = MimoProcessorBase::DefaultInput;

    class Output : public MimoProcessorBase::DefaultOutput
    {
      public:
        explicit Output(const Params& p)
          : MimoProcessorBase::DefaultOutput(p)
          , _combiner(this->parent.get_input_list(), *this)
        {}

        APF_PROCESS(Output, MimoProcessorBase::DefaultOutput)
        {
          _combiner.process(select_all_inputs());
        }

      private:
        struct select_all_inputs
        {
          apf::CombineChannelsResult::type select(const Input&)
          {
            return apf::CombineChannelsResult::constant;
          }
        };

        apf::CombineChannelsCopy<rtlist_proxy<Input>, Output> _combiner;
    };

    MyProcessor(const apf::parameter_map& p)
      : MimoProcessorBase(p)
    {
      this->add<Input>();
    }
};

int main()
{
  int out_channels = 20;

  apf::parameter_map p;
  p.set("threads", 1);
  //p.set("threads", 2);  // not allowed with dummy_thread_policy!
  MyProcessor engine(p);
  engine.activate();

  sleep(2);

  std::vector<MyProcessor::Output*> outputs;

  for (int i = 1; i <= out_channels; ++i)
  {
    MyProcessor::Output::Params op;
    op.set("id", i * 10);
    outputs.push_back(engine.add(op));
    sleep(1);
  }

  sleep(2);

  // remove the outputs one by one ...
  while (outputs.begin() != outputs.end())
  {
    engine.rem(outputs.front());
    engine.wait_for_rt_thread();
    outputs.erase(outputs.begin());
    sleep(1);
  }

  sleep(2);

  engine.deactivate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
