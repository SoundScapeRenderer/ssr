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

// A small (static) example of the MimoProcessor with the fixed_matrix class.
// This is a stand-alone program.

#include <iostream>
#include <cassert>  // for assert()

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/container.h"  // for fixed_matrix

class MatrixProcessor : public apf::MimoProcessor<MatrixProcessor
                        , apf::jack_policy
                        , apf::posix_thread_policy>
{
  public:
    using matrix_t = apf::fixed_matrix<sample_type>;
    using channel_iterator = matrix_t::channel_iterator;
    using slice_iterator = matrix_t::slice_iterator;
    using Channel = matrix_t::Channel;
    using Slice = matrix_t::Slice;

    using Input = MimoProcessorBase::DefaultInput;

    class m1_channel;
    class m2_channel;
    class m3_slice;
    class Output;

    explicit MatrixProcessor(const apf::parameter_map& p);

    ~MatrixProcessor()
    {
      this->deactivate();
      _m3_list.clear();
      _m2_list.clear();
      _m1_list.clear();
    }

    APF_PROCESS(MatrixProcessor, MimoProcessorBase)
    {
      _process_list(_m1_list);
      _process_list(_m2_list);
      _process_list(_m3_list);
    }

  private:
    /// make sure blocksize is divisible by parts.
    static int _get_parts(int x, int blocksize)
    {
      int parts = x;
      while (blocksize % parts != 0) parts /= 2;
      return parts;
    }

    const int _channels, _blocksize, _parts, _part_length, _part_channels;
    matrix_t _m1, _m2, _m3;
    rtlist_t _m1_list, _m2_list, _m3_list;
};

class MatrixProcessor::m1_channel : public ProcessItem<m1_channel>
{
  public:
    struct Params
    {
      Params() : input(nullptr), part(0), part_size(0) {}
      Channel channel;
      const Input* input;
      int part, part_size;
    };

    class Setup
    {
      public:
        Setup(int parts, int part_length, const rtlist_proxy<Input>& input_list)
          : _part(0)
          , _parts(parts)
          , _part_length(part_length)
          , _input(input_list.begin())
        {}

        m1_channel* operator()(const Channel& channel)
        {
          Params p;
          p.channel = channel;
          p.input = &*_input;
          p.part_size = _part_length;
          p.part = _part;

          ++_part;
          if (_part >= _parts)
          {
            _part = 0;
            ++_input;
          }
          return new m1_channel(p);
        }

      private:
        int _part;
        const int _parts, _part_length;
        rtlist_proxy<Input>::iterator _input;
    };

    APF_PROCESS(m1_channel, ProcessItem<m1_channel>)
    {
      assert(_input != nullptr);
      auto begin = _input->begin() + _part * _part_size;
      std::copy(begin, begin + _part_size, _channel.begin());
    }

  private:
    m1_channel(const Params& p)
      : _channel(p.channel)
      , _input(p.input)
      , _part(p.part)
      , _part_size(p.part_size)
    {}

    Channel _channel;
    const Input* const _input;
    const int _part, _part_size;
};

class MatrixProcessor::m2_channel : public ProcessItem<m2_channel>
{
  public:
    struct Params { Channel channel; Slice input; };

    static m2_channel* create(const Channel& channel, const Slice& input)
    {
      Params temp;
      temp.channel = channel;
      temp.input   = input;
      return new m2_channel(temp);
    }

    APF_PROCESS(m2_channel, ProcessItem<m2_channel>)
    {
      std::copy(_input.begin(), _input.end(), _channel.begin());
    }

  private:
    m2_channel(const Params& p) : _channel(p.channel) , _input(p.input) {}

    Channel _channel;
    Slice _input;
};

class MatrixProcessor::m3_slice : public ProcessItem<m3_slice>
{
  public:
    struct Params { Slice slice; Channel input; };

    static m3_slice* create(const Slice& slice, const Channel& input)
    {
      Params temp;
      temp.slice = slice;
      temp.input = input;
      return new m3_slice(temp);
    }

    APF_PROCESS(m3_slice, ProcessItem<m3_slice>)
    {
      std::copy(_input.begin(), _input.end(), _slice.begin());
    }

  private:
    m3_slice(const Params& p) : _slice(p.slice), _input(p.input) {}

    Slice _slice;
    Channel _input;
};

class MatrixProcessor::Output : public MimoProcessorBase::DefaultOutput
{
  public:
    struct Params : MimoProcessorBase::DefaultOutput::Params
    {
      std::list<Channel> channel_list;
    };

    explicit Output(const Params& p)
      : MimoProcessorBase::DefaultOutput(p)
      , _channel_list(p.channel_list)
    {}

    APF_PROCESS(Output, MimoProcessorBase::DefaultOutput)
    {
      auto out = this->begin();

      for (const auto& ch: _channel_list)
      {
        out = std::copy(ch.begin(), ch.end(), out);
      }
      assert(out = this->end());
    }

  private:
    std::list<Channel> _channel_list;
};

MatrixProcessor::MatrixProcessor(const apf::parameter_map& p)
  : MimoProcessorBase(p)
  , _channels(p.get<int>("channels"))  // if no channels -> exception!
  , _blocksize(this->block_size())
  , _parts(_get_parts(16, _blocksize))
  , _part_length(_blocksize / _parts)
  , _part_channels(_channels * _parts)
  , _m1(_part_channels, _part_length)
  , _m2(_part_length,   _part_channels)
  , _m3(_part_channels, _part_length)
  , _m1_list(_fifo)
  , _m2_list(_fifo)
  , _m3_list(_fifo)
{
  std::cout << "channels: " << _channels << ", parts: " << _parts
    << ", blocksize: " << _blocksize << std::endl;

  std::cout << "Creating Matrix with "
    << _part_channels << " channels and "
    << _part_length << " slices." << std::endl;

  // first, set parameters for all inputs ...
  Input::Params ip;
  for (int i = 1; i <= _channels; ++i)
  {
    ip.set("id", i);
    this->add(ip);
  }

  // m1: input channels are split up in more (and smaller) channels

  m1_channel::Setup m1_setup(_parts, _part_length, this->get_input_list());
  for (const auto& ch: _m1.channels)
  {
    _m1_list.add(m1_setup(ch));
  }

  // m2: reading slices from first matrix and writing to channels of second
  // matrix (= transpose matrix)

  std::list<m2_channel*> m2_temp;
  std::transform(_m2.channels.begin(), _m2.channels.end(), _m1.slices.begin()
      , back_inserter(m2_temp), m2_channel::create);
  _m2_list.add(m2_temp.begin(), m2_temp.end());

  // m3: reading channels, writing slices

  std::list<m3_slice*> m3_temp;
  std::transform(_m3.slices.begin(), _m3.slices.end(), _m2.channels.begin()
      , back_inserter(m3_temp), m3_slice::create);
  _m3_list.add(m3_temp.begin(), m3_temp.end());

  // set parameters for all outputs ...
  Output::Params op;
  op.parent = this;
  auto next_channel = _m3.channels.begin();
  for (int i = 1; i <= _channels; ++i)
  {
    op.set("id", i);
    op.channel_list.clear();
    for (int j = 0; j < _parts; ++j)
    {
      op.channel_list.push_back(*next_channel++);
    }
    this->add(op);
  }

  this->activate();
}

int main()
{
  apf::parameter_map p;
  p.set("channels", 2);
  //p.set("channels", 120);
  p.set("threads", 2);
  MatrixProcessor engine(p);
  sleep(60);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
