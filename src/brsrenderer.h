/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://spatialaudio.net/ssr                           ssr@spatialaudio.net *
 ******************************************************************************/

/// @file
/// Binaural Room Synthesis renderer.

#ifndef SSR_BRSRENDERER_H
#define SSR_BRSRENDERER_H

#include "rendererbase.h"

#include "apf/convolver.h"  // for apf::conv::*
#include "apf/sndfiletools.h"  // for apf::load_sndfile
#include "apf/combine_channels.h"  // for apf::raised_cosine_fade, ...

namespace ssr
{

class BrsRenderer : public SourceToOutput<BrsRenderer, RendererBase>
{
  private:
    using _base = SourceToOutput<BrsRenderer, ssr::RendererBase>;

  public:
    static const char* name() { return "BrsRenderer"; }

    using Input = _base::DefaultInput;
    class Source;
    struct SourceChannel;
    class Output;
    class RenderFunction;

    BrsRenderer(const apf::parameter_map& params)
      : _base(params)
      , _fade(this->block_size())
    {}

    void load_reproduction_setup();

    APF_PROCESS(BrsRenderer, _base)
    {
      this->_process_list(_source_list);
    }

  private:
    apf::raised_cosine_fade<sample_type> _fade;
};

struct BrsRenderer::SourceChannel : apf::has_begin_and_end<sample_type*>
                                  , apf::conv::Output
{
  explicit SourceChannel(const apf::conv::Input& in)
    : apf::conv::Output(in)
  {}

  // out-of-class definition because of cyclic dependencies with Source
  void update();
  void convolve_and_more(sample_type weight);

  apf::CombineChannelsResult::type crossfade_mode;
  sample_type new_weighting_factor;
};

class BrsRenderer::Source : public _base::Source
{
  public:
    Source(const Params& p)
      : _base::Source(p)
      , _weighting_factor(-1.0f)
      , _brtf_index(size_t(-1))
    {
      SndfileHandle ir_file
        = apf::load_sndfile(p.get<std::string>("properties_file")
            , this->parent.sample_rate(), 0);

      size_t no_of_channels = ir_file.channels();

      if (no_of_channels % 2 != 0)
      {
        throw std::logic_error(
            "Number of channels in BRIR file must be a multiple of 2!");
      }

      _angles = no_of_channels / 2;

      size_t size = ir_file.frames();

      using matrix_t = apf::fixed_matrix<sample_type>;

      auto ir_data = matrix_t(size, no_of_channels);

      // TODO: check return value?
      ir_file.readf(ir_data.data(), size);

      size_t block_size = this->parent.block_size();

      auto temp = apf::conv::Transform(block_size);

      size_t partitions = apf::conv::min_partitions(block_size, size);

      _brtf_set.reset(new brtf_set_t(no_of_channels, block_size, partitions));

      auto target = _brtf_set->begin();
      for (const auto& slice: ir_data.slices)
      {
        temp.prepare_filter(slice.begin(), slice.end(), *target++);
      }

      assert(target == _brtf_set->end());

      _convolver_input.reset(new apf::conv::Input(block_size, partitions));

      this->sourcechannels.reserve(2);
      this->sourcechannels.emplace_back(*_convolver_input);
      this->sourcechannels.emplace_back(*_convolver_input);
    }

    APF_PROCESS(Source, _base::Source)
    {
      _convolver_input->add_block(_input.begin());

      _weighting_factor = this->weighting_factor;

      float azi = this->parent.state.reference_orientation.get().azimuth;

      // TODO: get reference offset!

      // get BRTF index from listener orientation
      // (source positions are NOT considered!)
      // 90 degree is in the middle of index 0
      _brtf_index = size_t(apf::math::wrap(
          (azi - 90.0f) * float(_angles) / 360.0f + 0.5f, float(_angles)));

      using namespace apf::CombineChannelsResult;
      auto crossfade_mode = apf::CombineChannelsResult::type();

      // Check on one channel only, filters are always changed in parallel
      bool queues_empty = this->sourcechannels[0].queues_empty();

      if (_weighting_factor.both() == 0)
      {
        crossfade_mode = nothing;
      }
      else if (queues_empty
          && !_weighting_factor.changed()
          && !_brtf_index.changed())
      {
        crossfade_mode = constant;
      }
      else if (_weighting_factor.old() == 0)
      {
        crossfade_mode = fade_in;
      }
      else if (_weighting_factor == 0)
      {
        crossfade_mode = fade_out;
      }
      else
      {
        crossfade_mode = change;
      }

      for (size_t i = 0; i < 2; ++i)
      {
        if (crossfade_mode == nothing || crossfade_mode == fade_in)
        {
          // No need to convolve with old values
        }
        else
        {
          this->sourcechannels[i].convolve_and_more(_weighting_factor.old());
        }

        if (!queues_empty) this->sourcechannels[i].rotate_queues();

        if (_brtf_index.changed())
        {
          // left and right channels are interleaved
          this->sourcechannels[i].set_filter((*_brtf_set)[2 * _brtf_index + i]);
        }

        this->sourcechannels[i].crossfade_mode = crossfade_mode;
        this->sourcechannels[i].new_weighting_factor = _weighting_factor;
      }
      assert(_brtf_index.exactly_one_assignment());
      assert(_weighting_factor.exactly_one_assignment());
    }

  private:
    using brtf_set_t = apf::fixed_vector<apf::conv::Filter>;
    std::unique_ptr<brtf_set_t> _brtf_set;

    apf::BlockParameter<sample_type> _weighting_factor;
    apf::BlockParameter<size_t> _brtf_index;

    std::unique_ptr<apf::conv::Input> _convolver_input;

    size_t _angles;  // Number of angles in BRIR file
};

void BrsRenderer::SourceChannel::update()
{
  this->convolve_and_more(this->new_weighting_factor);
}

void BrsRenderer::SourceChannel::convolve_and_more(sample_type weight)
{
  _begin = this->convolve(weight);
  _end = _begin + this->block_size();
}

class BrsRenderer::RenderFunction
{
  public:
    RenderFunction() : _in(0) {}

    apf::CombineChannelsResult::type select(SourceChannel& in)
    {
      _in = &in;
      return in.crossfade_mode;
    }

    void update()
    {
      assert(_in);
      _in->update();
    }

  private:
    SourceChannel* _in;
};

class BrsRenderer::Output : public _base::Output
{
  public:
    Output(const Params& p)
      : _base::Output(p)
      , _combiner(this->sourcechannels, this->buffer, this->parent._fade)
    {}

    APF_PROCESS(Output, _base::Output)
    {
      _combiner.process(RenderFunction());
    }

  private:
    apf::CombineChannelsCrossfadeCopy<apf::cast_proxy<SourceChannel
      , sourcechannels_t>, buffer_type
      , apf::raised_cosine_fade<sample_type>> _combiner;
};

void
BrsRenderer::load_reproduction_setup()
{
  // TODO: generalize this for all headphone-based renderers!

  // TODO: read settings from proper reproduction system

  auto params = Output::Params();

  const std::string prefix = this->params.get("system_output_prefix", "");

  if (prefix != "")
  {
    // TODO: read target from proper reproduction file
    params.set("connect_to", prefix + "1");
  }
  this->add(params);

  if (prefix != "")
  {
    params.set("connect_to", prefix + "2");
  }
  this->add(params);
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
