// A small example of the MimoProcessor with varying JACK output ports.
// This is a stand-alone program.

#include <vector>

#include "apf/mimoprocessor.h"
#include "apf/combine_channels.h"  // for apf::CombineChannelsCopy
#include "apf/jack_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor, apf::jack_policy>
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
};

void sleep(int sec)
{
  std::this_thread::sleep_for(std::chrono::seconds(sec));
}

int main()
{
  int out_channels = 20;

  MyProcessor engine;
  engine.add<MyProcessor::Input>();
  engine.activate();

  sleep(2);

  std::vector<MyProcessor::Output*> outputs;

  for (int i = 1; i <= out_channels; ++i)
  {
    MyProcessor::Output::Params p;
    p.set("id", i * 10);
    p.set("connect_to", "system:playback_1");
    outputs.push_back(engine.add(p));
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
