// A small example of the MimoProcessor with varying JACK input ports.
// This is a stand-alone program.

#include "apf/mimoprocessor.h"
#include "apf/combine_channels.h"  // for apf::CombineChannels
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
          float weight = 1.0f/static_cast<float>(
              this->parent.get_input_list().size());
          _combiner.process(simple_predicate(weight));
        }

      private:
        class simple_predicate
        {
          public:
            explicit simple_predicate(float weight) : _weight(weight) {}

            // trivial, all inputs are used; no crossfade/interpolation
            apf::CombineChannelsResult::type select(const Input&)
            {
              return apf::CombineChannelsResult::constant;
            }

            float operator()(float in) { return in * _weight; }

          private:
            float _weight;
        };

        apf::CombineChannels<rtlist_proxy<Input>, Output> _combiner;
    };
};

void sleep(int sec)
{
  std::this_thread::sleep_for(std::chrono::seconds(sec));
}

int main()
{
  int in_channels = 20;

  MyProcessor engine;
  engine.add<MyProcessor::Output>();
  engine.activate();

  sleep(2);

  std::vector<MyProcessor::Input*> inputs;

  for (int i = 1; i <= in_channels; ++i)
  {
    MyProcessor::Input::Params p;
    p.set("id", i * 10);
    p.set("connect_to", "system:capture_1");
    inputs.push_back(engine.add(p));
    sleep(1);
  }

  sleep(2);

  // remove the inputs one by one ...
  while (inputs.begin() != inputs.end())
  {
    engine.rem(inputs.front());
    engine.wait_for_rt_thread();
    inputs.erase(inputs.begin());
    sleep(1);
  }

  sleep(2);

  engine.deactivate();
}
