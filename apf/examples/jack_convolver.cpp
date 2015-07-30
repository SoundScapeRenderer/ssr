// Example for the Convolver.

#include <iostream>
#include <sndfile.hh>

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/cxx_thread_policy.h"
#include "apf/shareddata.h"
#include "apf/convolver.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::jack_policy
                    , apf::cxx_thread_policy>
{
  public:
    using Input = MimoProcessorBase::DefaultInput;

    template<typename In>
    MyProcessor(In first, In last);

    ~MyProcessor() { this->deactivate(); }

    struct Output : MimoProcessorBase::DefaultOutput
    {
      Output(const Params& p) : MimoProcessorBase::DefaultOutput(p) {}

      // Deactivate process() function, fetch_buffer() is called earlier!
      virtual void process() {}
    };

    APF_PROCESS(MyProcessor, MimoProcessorBase)
    {
      _convolver.add_block(_input->begin());

      if (!_convolver.queues_empty()) _convolver.rotate_queues();

      if (this->reverb != _old_reverb)
      {
        if (this->reverb)
        {
          _convolver.set_filter(_filter);
        }
        else
        {
          _convolver.set_filter(_dirac);
        }
        _old_reverb = this->reverb;
      }
      float* result = _convolver.convolve();

      // This is necessary because _output is used before _output_list is
      // processed:
      _output->fetch_buffer();

      std::copy(result, result + this->block_size(), _output->begin());
    }

    apf::SharedData<bool> reverb;

  private:
    bool _old_reverb;

    Input* _input;
    Output* _output;

    apf::conv::Filter _filter;
    apf::conv::Convolver _convolver;

    apf::conv::Filter _dirac;
};

template<typename In>
MyProcessor::MyProcessor(In first, In last)
  : MimoProcessorBase()
  , reverb(_fifo, true)
  , _old_reverb(false)
  , _filter(this->block_size(), first, last)
  , _convolver(this->block_size(), _filter.partitions())
  , _dirac(this->block_size(), 1)
{
  // Load Dirac
  float one = 1.0f;
  _convolver.prepare_filter(&one, (&one)+1, _dirac);

  _input = this->add<Input>();
  _output = this->add<Output>();

  std::cout << "Press <enter> to switch and q to quit" << std::endl;
  this->activate();
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <IR filename>" << std::endl;
    return 1;
  }

  SndfileHandle in(argv[1], SFM_READ);

  if (in.error()) throw std::runtime_error(in.strError());

  if (in.channels() != 1)
  {
    throw std::runtime_error("Only mono files are supported!");
  }

  std::vector<float> ir(size_t(in.frames()));

  if (in.readf(&ir[0], in.frames()) != in.frames())
  {
    throw std::runtime_error("Couldn't load audio file!");
  }

  MyProcessor processor(ir.begin(), ir.end());

  if (in.samplerate() != int(processor.sample_rate()))
  {
    throw std::runtime_error("Samplerate mismatch!");
  }

  std::string input;
  bool reverb = true;

  for (;;)
  {
    std::getline(std::cin, input);
    if (input == "")
    {
      if (reverb)
      {
        processor.reverb = false;
        reverb = false;
        std::cout << "filter off" << std::endl;
      }
      else
      {
        processor.reverb = true;
        reverb = true;
        std::cout << "filter on" << std::endl;
      }
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

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
