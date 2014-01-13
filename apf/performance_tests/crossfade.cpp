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

// Performance tests for the crossfade.

#include <cstdlib>  // for random()

#include "apf/pointer_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/mimoprocessor.h"
#include "apf/combine_channels.h"  // for apf::raised_cosine_fade, Combine...
#include "apf/container.h"  // for apf::fixed_matrix
#include "apf/stopwatch.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::pointer_policy<float*>
                    , apf::posix_thread_policy>
{
  public:
    using Input = DefaultInput;
    class Output;
    class CombineFunction;

    MyProcessor(const apf::parameter_map& p);

  private:
    apf::raised_cosine_fade<float> _fade;
};

class MyProcessor::CombineFunction
{
  public:
    apf::CombineChannelsResult::type select(const Input&)
    {
      return apf::CombineChannelsResult::change;  // Always force crossfade
    }

    float operator()(float in, apf::fade_out_tag)
    {
      return in * 0.5f;
    }

    float operator()(float in)
    {
      return in * 3.14f;
    }

    void update() {}  // Unused. Call will be optimized away.
};

class MyProcessor::Output : public MimoProcessorBase::DefaultOutput
{
  public:
    explicit Output(const Params& p)
      : MimoProcessorBase::DefaultOutput(p)
      , _combine_and_crossfade(this->parent.get_input_list(), *this
          , this->parent._fade)
    {}

    APF_PROCESS(Output, MimoProcessorBase::DefaultOutput)
    {
      _combine_and_crossfade.process(CombineFunction());
    }

  private:
    apf::CombineChannelsCrossfade<rtlist_proxy<Input>, Output
      , apf::raised_cosine_fade<float>> _combine_and_crossfade;
};

MyProcessor::MyProcessor(const apf::parameter_map& p)
  : MimoProcessorBase(p)
  , _fade(this->block_size())
{
  for (int i = 0; i < p.get<int>("in_channels"); ++i)
  {
    this->add<Input>();
  }

  for (int i = 0; i < p.get<int>("out_channels"); ++i)
  {
    this->add<Output>();
  }
}

int main()
{
  // TODO: check for input arguments

  int in_channels = 10;
  int out_channels = 70;
  int block_size = 512;
  int repetitions = 1000;
  int threads = 1;

  apf::fixed_matrix<float> m_in(in_channels, block_size);
  apf::fixed_matrix<float> m_out(out_channels, block_size);

  // WARNING: this is not really a meaningful audio signal:
  std::generate(m_in.begin(), m_in.end(), random);

  apf::parameter_map p;
  p.set("in_channels", in_channels);
  p.set("out_channels", out_channels);
  p.set("block_size", block_size);
  p.set("sample_rate", 44100);  // Not really relevant in this case
  p.set("threads", threads);

  MyProcessor processor(p);

  processor.activate();

  {
    apf::StopWatch watch("processing");
    for (int i = 0; i < repetitions; ++i)
    {
      processor.audio_callback(block_size
          , m_in.get_channel_ptrs(), m_out.get_channel_ptrs());
    }
  }

  processor.deactivate();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
