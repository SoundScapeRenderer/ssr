/******************************************************************************
 Copyright (c) 2012-2016 Institut für Nachrichtentechnik, Universität Rostock
 Copyright (c) 2006-2012 Quality & Usability Lab
                         Deutsche Telekom Laboratories, TU Berlin

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*******************************************************************************/

// https://AudioProcessingFramework.github.io/

/// @file
/// PortAudio policy for MimoProcessor's interface_policy.

#ifndef APF_PORTAUDIO_POLICY_H
#define APF_PORTAUDIO_POLICY_H

#include <portaudio.h>
#include <cassert>  // for assert()

#include "apf/parameter_map.h"
#include "apf/stringtools.h"
#include "apf/iterator.h"  // for has_begin_and_end

#ifndef APF_MIMOPROCESSOR_INTERFACE_POLICY
#define APF_MIMOPROCESSOR_INTERFACE_POLICY apf::portaudio_policy
#endif

namespace apf
{

/// @c interface_policy using PortAudio.
/// @see MimoProcessor
/// @ingroup apf_policies
class portaudio_policy
{
  public:
    using sample_type = float;
    class Input;
    class Output;

    struct portaudio_error : std::runtime_error
    {
      portaudio_error(PaError error)
        : std::runtime_error("PortAudio: "+std::string(Pa_GetErrorText(error)))
      {}
    };

    // const PaStreamInfo *    Pa_GetStreamInfo (PaStream *stream)

    static std::string device_info()
    {
      auto err = Pa_Initialize();
      if (err != paNoError) throw portaudio_error(err);

      std::string result;
      for (int i = 0; i < Pa_GetDeviceCount(); ++i)
      {
        result = result + "Device ID " + str::A2S(i) + ": "
          + Pa_GetDeviceInfo(i)->name + "\n";
      }
      return result;
    }

    bool activate()
    {
      // the original definition causes a warning message (old-style cast).
      // 32bit float, non-interleaved
      unsigned long sample_format = 0x00000001 | 0x80000000;

      auto inputParameters = PaStreamParameters();
      auto outputParameters = PaStreamParameters();

      inputParameters.channelCount = _next_input_id;
      inputParameters.device = _device_id;
      inputParameters.hostApiSpecificStreamInfo = nullptr;
      inputParameters.sampleFormat = sample_format;
      inputParameters.suggestedLatency
        = 0; //Pa_GetDeviceInfo(_device_id)->defaultLowInputLatency ;

      outputParameters.channelCount = _next_output_id;
      outputParameters.device = _device_id;
      outputParameters.hostApiSpecificStreamInfo = nullptr;
      outputParameters.sampleFormat = sample_format;
      outputParameters.suggestedLatency
        = 0; //Pa_GetDeviceInfo(_device_id)->defaultLowOutputLatency ;

      auto err = Pa_OpenStream(&_stream, &inputParameters, &outputParameters
          , _sample_rate, _block_size, 0, _pa_callback, this);

      if (err != paNoError) throw portaudio_error(err);

      err = Pa_StartStream(_stream);
      if (err != paNoError) throw portaudio_error(err);
      return true;
    }

    bool deactivate()
    {
      auto err = Pa_StopStream(_stream);
      if (err != paNoError) throw portaudio_error(err);
      return true;
    }

    unsigned long block_size() const { return _block_size; }
    int sample_rate() const { return _sample_rate; }

    int in_channels() const { return _next_input_id; }
    int out_channels() const { return _next_output_id; }

    // this is just temporary!
    // TODO: find a clean solution regarding audio and thread policies
    int get_real_time_priority() const { return -1; }

  protected:
    /// Constructor
    explicit portaudio_policy(const parameter_map& p = parameter_map())
      : _sample_rate(p.get<int>("sample_rate"))
      , _block_size(p.get<unsigned long>("block_size"))
      , _device_id(p.get("device_id", 0))
      , _next_input_id(0)
      , _next_output_id(0)
    {
      auto err = Pa_Initialize();
      if (err != paNoError) throw portaudio_error(err);
    }

    /// Protected destructor
    ~portaudio_policy()
    {
      Pa_CloseStream(_stream);  // ignore errors
      Pa_Terminate();  // ignore errors
    }

  private:
    static int _pa_callback(const void *input, void *output
        , unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo
        , PaStreamCallbackFlags statusFlags, void *userData)
    {
      (void)timeInfo;  // not used
      (void)statusFlags;  // not used

      return static_cast<portaudio_policy*>(userData)->pa_callback(input
          , output, frameCount);
    }

    int pa_callback(const void *input, void *output, unsigned long frameCount)
    {
      (void)frameCount;
      assert(frameCount == this->block_size());

      _in = static_cast<sample_type* const*>(input);
      _out = static_cast<sample_type* const*>(output);

      this->process();

      return paContinue;
      // possible return values: paContinue, paComplete, paAbort
    }

    virtual void process() = 0;

    /// Generate next higher input ID.
    /// @warning This function is \b not re-entrant!
    int get_next_input_id() { return _next_input_id++; }

    /// @see get_next_input_id()
    int get_next_output_id() { return _next_output_id++; }

    const int _sample_rate;
    const unsigned long _block_size;
    const int _device_id;

    int _next_input_id;
    int _next_output_id;
    sample_type* const* _in;
    sample_type* const* _out;

    PaStream* _stream;
};

class portaudio_policy::Input
{
  public:
    using iterator = sample_type const*;

    struct buffer_type : has_begin_and_end<iterator> { friend class Input; };

    void fetch_buffer()
    {
      this->buffer._begin = _parent._in[_id];
      this->buffer._end   = this->buffer._begin + _parent.block_size();
    }

    buffer_type buffer;

  protected:
    Input(portaudio_policy& parent, const parameter_map&)
      : _parent(parent)
      , _id(_parent.get_next_input_id())
    {}

    ~Input() = default;

  private:
    portaudio_policy& _parent;
    const int _id;
};

class portaudio_policy::Output
{
  public:
    using iterator = sample_type*;

    struct buffer_type : has_begin_and_end<iterator> { friend class Output; };

    void fetch_buffer()
    {
      this->buffer._begin = _parent._out[_id];
      this->buffer._end   = this->buffer._begin + _parent.block_size();
    }

    buffer_type buffer;

  protected:
    Output(portaudio_policy& parent, const parameter_map&)
      : _parent(parent)
      , _id(_parent.get_next_output_id())
    {}

    ~Output() = default;

  private:
    portaudio_policy& _parent;
    const int _id;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
