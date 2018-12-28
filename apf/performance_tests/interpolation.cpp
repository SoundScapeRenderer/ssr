// Performance tests for the interpolation.

#include <cstdlib>  // for random()

#include "apf/pointer_policy.h"
#include "apf/cxx_thread_policy.h"
#include "apf/mimoprocessor.h"
#include "apf/combine_channels.h"  // for apf::CombineChannelsInterpolation
#include "apf/container.h"  // for fixed_matrix
#include "apf/stopwatch.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::pointer_policy<float*>
                    , apf::cxx_thread_policy>
{
  public:
    using Input = DefaultInput;
    class Output;
    class CombineFunction;

    MyProcessor(const apf::parameter_map& p);
};

class MyProcessor::CombineFunction
{
  public:
    CombineFunction(size_t block_size)
      : _block_size(float(block_size))
    {}

    apf::CombineChannelsResult::type select(const Input&)
    {
      _interpolator.set(3.14f, 666.666f, _block_size);
      return apf::CombineChannelsResult::change;  // Always force interpolation
    }

    float operator()(float)
    {
      throw std::logic_error("This is never used!");
      return 0.0f;
    }

    float operator()(float in, float index)
    {
      return in * _interpolator(index);
    }

  private:
    // float is much faster here than int because less casts are necessary
    float _block_size;
    apf::math::linear_interpolator<float> _interpolator;
};

class MyProcessor::Output : public MimoProcessorBase::DefaultOutput
{
  public:
    explicit Output(const Params& p)
      : MimoProcessorBase::DefaultOutput(p)
      , _combine_and_interpolate(this->parent.get_input_list(), *this)
    {}

    APF_PROCESS(Output, MimoProcessorBase::DefaultOutput)
    {
      _combine_and_interpolate.process(
          CombineFunction(this->parent.block_size()));
    }

  private:
    apf::CombineChannelsInterpolation<rtlist_proxy<Input>, Output>
      _combine_and_interpolate;
};

MyProcessor::MyProcessor(const apf::parameter_map& p)
  : MimoProcessorBase(p)
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

  size_t in_channels = 10;
  size_t out_channels = 70;
  size_t block_size = 512;
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
