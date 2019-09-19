// Example for crossfade and/or parameter interpolation (?)

#include <iostream>
#include <cassert>  // for assert()

#include "apf/mimoprocessor.h"
#include "apf/combine_channels.h"  // for apf::raised_cosine_fade, apf::Combine*
#include "apf/jack_policy.h"
#include "apf/shareddata.h"
#include "apf/math.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor, apf::jack_policy>
{
  public:
    class Input;
    class CombineFunction;
    class Output;

    MyProcessor();
    ~MyProcessor() { this->deactivate(); }

    enum { CROSSFADE, INTERPOLATION } mode;

    apf::SharedData<float> volume;

  private:
    apf::raised_cosine_fade<float> _fade;
};

class MyProcessor::Input : public MimoProcessorBase::DefaultInput
{
  public:
    explicit Input(const Params& p)
      : MimoProcessorBase::DefaultInput(p)
      , weight(this->parent.volume)
    {}

    APF_PROCESS(Input, MimoProcessorBase::DefaultInput)
    {
      // In real-life applications, this will be more complicated:
      this->weight = this->parent.volume;

      assert(this->weight.exactly_one_assignment());
    }

    apf::BlockParameter<float> weight;
};

class MyProcessor::CombineFunction
{
  public:
    CombineFunction(size_t block_size) : _block_size(float(block_size)) {}

    apf::CombineChannelsResult::type select(const Input& in)
    {
      using namespace apf::CombineChannelsResult;

      _weight = in.weight;
      _old_weight = in.weight.old();

      if (_weight != _old_weight)
      {
        _interpolator.set(_old_weight, _weight, _block_size);
        return change;
      }

      if (_weight != 0.0f) return constant;

      return nothing;
    }

    float operator()(float in)
    {
      return in * _weight;
    }

    float operator()(float in, apf::fade_out_tag)
    {
      return in * _old_weight;
    }

    float operator()(float in, float index)
    {
      return in * _interpolator(index);
    }

    void update()
    {
      // This is called between fade-out and fade-in
    }

  private:
    const float _block_size;
    float _weight, _old_weight;
    apf::math::linear_interpolator<float> _interpolator;
};

class MyProcessor::Output : public MimoProcessorBase::DefaultOutput
{
  public:
    explicit Output(const Params& p)
      : MimoProcessorBase::DefaultOutput(p)
      , _combine_and_interpolate(this->parent.get_input_list(), *this)
      , _combine_and_crossfade(this->parent.get_input_list(), *this
          , this->parent._fade)
      , _combine_function(this->parent.block_size())
    {}

    APF_PROCESS(Output, MimoProcessorBase::DefaultOutput)
    {
      switch (this->parent.mode)
      {
        case INTERPOLATION:
          _combine_and_interpolate.process(_combine_function);
          break;

        case CROSSFADE:
          _combine_and_crossfade.process(_combine_function);
          break;
      }
    }

  private:
    apf::CombineChannelsInterpolation<rtlist_proxy<Input>, Output>
      _combine_and_interpolate;
    apf::CombineChannelsCrossfade<rtlist_proxy<Input>, Output
      , apf::raised_cosine_fade<float>> _combine_and_crossfade;
    CombineFunction _combine_function;
};

MyProcessor::MyProcessor()
  : MimoProcessorBase()
  , mode(CROSSFADE)
  , volume(_fifo, 1.0f)
  , _fade(this->block_size())
{
  // Let's create 2 inputs ...
  this->add<Input>();
  this->add<Input>();

  // ... and 1 output, OK?
  this->add<Output>();

  std::cout << "following keys are supported:\n"
    " +<enter> to increment volume\n"
    " -<enter> to decrement volume\n"
    " 0<enter> to mute\n"
    " 1<enter> to unmute\n"
    " c<enter> to switch to crossfade mode\n"
    " i<enter> to switch to interpolation mode\n"
    " q<enter> to quit\n"
    " press and hold the <enter> key to continuously toggle the mute state\n"
    << std::endl;
  std::cout << "current mode: crossfade" << std::endl;

  this->activate();
}

int main()
{
  MyProcessor processor;

  std::string input;
  auto volume = 1.0f;

  for (;;)
  {
    std::cout << volume << std::endl;
    processor.volume = volume;

    std::getline(std::cin, input);
    if (input == "+")
    {
      volume = apf::math::dB2linear(apf::math::linear2dB(volume+0.01f) + 1);
    }
    else if (input == "-")
    {
      volume = apf::math::dB2linear(apf::math::linear2dB(volume) - 1);
    }
    else if (input == "0")
    {
      volume = 0.0f;
    }
    else if (input == "1")
    {
      volume = 1.0f;
    }
    else if (input == "")
    {
      if (volume > 0.0001f)
      {
        volume = 0.0f;
      }
      else
      {
        volume = 1.0f;
      }
    }
    else if (input == "c")
    {
      processor.mode = MyProcessor::CROSSFADE;
      std::cout << "current mode: crossfade" << std::endl;
    }
    else if (input == "i")
    {
      processor.mode = MyProcessor::INTERPOLATION;
      std::cout << "current mode: interpolation" << std::endl;
    }
    else if (input == "q")
    {
      break;
    }
    else
    {
      std::cout << "What? Type q to quit!" << std::endl;
    }
  }
}
