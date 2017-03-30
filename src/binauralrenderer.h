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
/// Binaural renderer.

#ifndef SSR_BINAURALRENDERER_H
#define SSR_BINAURALRENDERER_H

#include "rendererbase.h"
#include "apf/iterator.h"  // for apf::cast_proxy, apf::make_cast_proxy()
#include "apf/convolver.h"  // for apf::conv::*
#include "apf/container.h"  // for apf::fixed_matrix
#include "apf/sndfiletools.h"  // for apf::load_sndfile
#include "apf/combine_channels.h"  // for apf::raised_cosine_fade, ...

namespace ssr
{

// TODO: derive from HeadphoneRenderer?
class BinauralRenderer : public SourceToOutput<BinauralRenderer, RendererBase>
{
  private:
    using _base = SourceToOutput<BinauralRenderer, ssr::RendererBase>;

  public:
    static const char* name() { return "BinauralRenderer"; }

    class SourceChannel;
    class Source;
    class Output;
    class RenderFunction;

    BinauralRenderer(const apf::parameter_map& params)
      : _base(params)
      , _fade(this->block_size())
      , _partitions(0)
    {}

    void load_reproduction_setup();

    APF_PROCESS(BinauralRenderer, _base)
    {
      this->_process_list(_source_list);
    }

  private:
    using hrtf_set_t = apf::fixed_vector<apf::conv::Filter>;

    void _load_hrtfs(const std::string& filename, size_t size);

    static bool _cmp_abs(sample_type left, sample_type right)
    {
      return std::abs(left) < std::abs(right);
    }

    apf::raised_cosine_fade<sample_type> _fade;
    size_t _partitions;
    size_t _angles;  // Number of angles in HRIR file
    std::unique_ptr<hrtf_set_t> _hrtfs;
    std::unique_ptr<apf::conv::Filter> _neutral_filter;
};

class BinauralRenderer::SourceChannel : public apf::conv::Output
                                      , public apf::has_begin_and_end<float*>
{
  public:
    SourceChannel(const apf::conv::Input& input)
      : apf::conv::Output(input)
      , temporary_hrtf(input.block_size(), input.partitions())
      , _block_size(input.block_size())
    {}

    void convolve_and_more(sample_type weight)
    {
      _begin = this->convolve(weight);
      _end = _begin + _block_size;
    }

    void update()
    {
      this->convolve_and_more(this->weight);
    }

    apf::conv::Filter temporary_hrtf;

    sample_type weight;
    apf::CombineChannelsResult::type crossfade_mode;

  private:
    const size_t _block_size;
};

void
BinauralRenderer::_load_hrtfs(const std::string& filename, size_t size)
{
  auto hrir_file = apf::load_sndfile(filename, this->sample_rate(), 0);

  const size_t no_of_channels = hrir_file.channels();

  if (no_of_channels % 2 != 0)
  {
    throw std::logic_error("Number of channels must be a multiple of 2!");
  }

  _angles = no_of_channels / 2;

  // TODO: handle size > hrir_file.frames()

  if (size == 0) size = hrir_file.frames();

  // Deinterleave channels and transform to FFT domain

  auto transpose = apf::fixed_matrix<float>(size, no_of_channels);

  size = hrir_file.readf(transpose.data(), size);

  _partitions = apf::conv::min_partitions(this->block_size(), size);

  auto temp = apf::conv::Transform(this->block_size());

  _hrtfs.reset(new hrtf_set_t(no_of_channels, this->block_size(), _partitions));

  auto target = _hrtfs->begin();
  for (const auto& slice: transpose.slices)
  {
    temp.prepare_filter(slice.begin(), slice.end(), *target++);
  }

  // prepare neutral filter (dirac impulse) for interpolation around the head

  // get index of absolute maximum in first channel (frontal direcion, left)
  apf::fixed_matrix<sample_type>::slice_iterator maximum
    = std::max_element(transpose.slices.begin()->begin()
      , transpose.slices.begin()->end(), _cmp_abs);

  int index = std::distance(transpose.slices.begin()->begin(), maximum);

  auto impulse = apf::fixed_vector<sample_type>(index + 1);
  impulse.back() = 1;

  _neutral_filter.reset(new apf::conv::Filter(this->block_size()
        , impulse.begin(), impulse.end()));
  // Number of partitions may be different from _hrtfs!
}

class BinauralRenderer::RenderFunction
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

class BinauralRenderer::Output : public _base::Output
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

void BinauralRenderer::load_reproduction_setup()
{
  // TODO: read settings from proper reproduction system

  try
  {
    _load_hrtfs(this->params["hrir_file"], this->params.get("hrir_size", 0));
  }
  catch (const std::logic_error& e)
  {
    throw std::logic_error("Error loading HRIR file: " + std::string(e.what()));
  }

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

class BinauralRenderer::Source : public apf::conv::Input, public _base::Source
{
  private:
    void _process();

  public:
    Source(const Params& p)
      // TODO: assert that p.parent != 0?
      : apf::conv::Input(p.parent->block_size(), p.parent->_partitions)
      , _base::Source(p, 2, *this)
      , _hrtf_index(size_t(-1))
      , _interp_factor(-1.0f)
      , _weight(0.0f)
    {}

    APF_PROCESS(Source, _base::Source)
    {
      _process();
    }

  private:
    apf::BlockParameter<size_t> _hrtf_index;
    apf::BlockParameter<float> _interp_factor;
    apf::BlockParameter<float> _weight;
};

void BinauralRenderer::Source::_process()
{
  float interp_factor = 0.0f;

  this->add_block(_input.begin());

  auto ref_pos = _input.parent.state.reference_position
    + _input.parent.state.reference_offset_position;
  auto ref_ori = _input.parent.state.reference_orientation
    + _input.parent.state.reference_offset_orientation;

  float source_distance = (this->position - ref_pos).length();

  if (this->weighting_factor != 0 && source_distance < 0.5f 
        && this->model != ::Source::plane)
  {
    interp_factor = 1.0f - 2 * source_distance;
  }

  _interp_factor = interp_factor;  // Assign (once!) to BlockParameter
  _weight = this->weighting_factor;  // ... same here

  float angles = _input.parent._angles;

  // calculate relative orientation of sound source
  auto rel_ori = -ref_ori;

  if (this->model == ::Source::plane)
  {
    // plane wave orientation points into direction of propagation
    // +180 degree has to be applied to select the hrtf correctly
    rel_ori += this->orientation + Orientation(180);
  }
  else
  {
    rel_ori += (this->position - ref_pos).orientation();
  }

  _hrtf_index = size_t(apf::math::wrap(
      rel_ori.azimuth * angles / 360.0f + 0.5f, angles));

  using namespace apf::CombineChannelsResult;
  auto crossfade_mode = apf::CombineChannelsResult::type();

  // Check on one channel only, filters are always changed in parallel
  bool queues_empty = this->sourcechannels[0].queues_empty();

  bool hrtf_changed = _hrtf_index.changed() || _interp_factor.changed();

  if (_weight.both() == 0)
  {
    crossfade_mode = nothing;
  }
  else if (queues_empty && !_weight.changed() && !hrtf_changed)
  {
    crossfade_mode = constant;
  }
  else if (_weight == 0)
  {
    crossfade_mode = fade_out;
  }
  else if (_weight.old() == 0)
  {
    crossfade_mode = fade_in;
  }
  else
  {
    crossfade_mode = change;
  }

  for (size_t i = 0; i < 2; ++i)
  {
    auto& channel = this->sourcechannels[i];

    if (crossfade_mode == nothing || crossfade_mode == fade_in)
    {
      // No need to convolve
    }
    else
    {
      channel.convolve_and_more(_weight.old());
    }

    if (!queues_empty) channel.rotate_queues();

    if (hrtf_changed)
    {
      // left and right channels are interleaved
      auto& hrtf = (*_input.parent._hrtfs)[2 * _hrtf_index + i];

      if (_interp_factor == 0)
      {
        channel.set_filter(hrtf);
      }
      else
      {
        // Interpolate between selected HRTF and neutral filter (Dirac)
        apf::conv::transform_nested(hrtf
            , *_input.parent._neutral_filter, channel.temporary_hrtf
            , [this] (sample_type one, sample_type two)
              {
                return (1.0f - _interp_factor) * one + _interp_factor * two;
              });
        this->sourcechannels[i].set_filter(channel.temporary_hrtf);
      }
    }

    channel.crossfade_mode = crossfade_mode;
    channel.weight = _weight;
  }

  assert(_hrtf_index.exactly_one_assignment());
  assert(_interp_factor.exactly_one_assignment());
  assert(_weight.exactly_one_assignment());
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
