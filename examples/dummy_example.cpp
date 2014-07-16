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

// This example is used in the Doxygen documentation to MimoProcessor.

#include "apf/mimoprocessor.h"
#include "apf/pointer_policy.h"
#include "apf/dummy_thread_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::pointer_policy<float*>, apf::dummy_thread_policy>
{
  public:
    using Input = MimoProcessorBase::DefaultInput;

    class MyIntermediateThing : public ProcessItem<MyIntermediateThing>
    {
      public:
        // you can create other classes and use them in their own RtList, as
        // long as they are derived from ProcessItem<YourClass> and have a
        // Process class publicly derived from ProcessItem<YourClass>::Process.

        // This can be facilitated with this macro call:
        APF_PROCESS(MyIntermediateThing, ProcessItem<MyIntermediateThing>)
        {
          // do your processing here!
        }
    };

    class Output : public MimoProcessorBase::DefaultOutput
    {
      public:
        explicit Output(const Params& p)
          : MimoProcessorBase::DefaultOutput(p)
        {}

        APF_PROCESS(Output, MimoProcessorBase::DefaultOutput)
        {
          // this->buffer.begin() and this->buffer.end(): access to audio data
        }
    };

    MyProcessor(const apf::parameter_map& p)
      : MimoProcessorBase(p)
      , _intermediate_list(_fifo)
    {
      this->add<Input>();
      _intermediate_list.add(new MyIntermediateThing());
      this->add<Output>();
      this->activate();
    }

    ~MyProcessor() { this->deactivate(); }

    APF_PROCESS(MyProcessor, MimoProcessorBase)
    {
      // input/output lists are processed automatically before/after this:
      _process_list(_intermediate_list);
    }

  private:
    rtlist_t _intermediate_list;
};

int main()
{
  // For now, this does nothing, we just want it to compile ...
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
