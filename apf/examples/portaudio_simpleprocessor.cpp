// Usage example for the MimoProcessor with PortAudio.

#include <iostream>

#include "apf/stringtools.h"

// First the policies ...
#include "apf/portaudio_policy.h"
#include "apf/cxx_thread_policy.h"
// ... then the SimpleProcessor.
#include "simpleprocessor.h"

using apf::str::S2A;
using apf::str::A2S;

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    std::cerr << "Error: too few arguments!" << std::endl;
    std::cout << "Usage: " << argv[0]
     << " inchannels outchannels samplerate blocksize [device-id]" << std::endl;

    std::cout << "\nList of devices:" << std::endl;
    std::cout << SimpleProcessor::device_info() << std::endl;
    return 42;
  }

  apf::parameter_map e;
  e.set("threads", 2);

  e.set("in_channels", argv[1]);
  e.set("out_channels", argv[2]);
  e.set("sample_rate", argv[3]);
  e.set("block_size", argv[4]);
  e.set("device_id", argv[5]);

  SimpleProcessor engine(e);

  sleep(60);
}
