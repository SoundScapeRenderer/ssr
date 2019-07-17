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
/// Ambisonics Amplitude Panning renderer.

#ifndef SSR_AAPRENDERER_H
#define SSR_AAPRENDERER_H

#include "ssr_global.h"
#include "loudspeakerrenderer.h"
#include "apf/combine_channels.h"

namespace ssr
{

class AapRenderer : public SourceToOutput<AapRenderer, LoudspeakerRenderer>
{
  private:
    using _base = SourceToOutput<AapRenderer, ssr::LoudspeakerRenderer>;

  public:
    static const char* name() { return "AAP-Renderer"; }

    class Source;
    class SourceChannel;
    class Output;
    class RenderFunction;

    explicit AapRenderer(const apf::parameter_map& params)
      : _base(params)
      , _ambisonics_order(params.get("ambisonics_order", 0))
      , _in_phase_rendering(params.get("in_phase", true))
    {
      SSR_VERBOSE((_in_phase_rendering ? "U" : "Not u")
          << "sing in-phase rendering.");
    }

    APF_PROCESS(AapRenderer, _base)
    {
      _process_list(_source_list);
    }

    void load_reproduction_setup();

  private:
    int _ambisonics_order;
    bool _in_phase_rendering;
};

class AapRenderer::Source : public _base::Source
{
  public:
    Source(const Params& p)
      : _base::Source(p, p.parent->get_output_list().size(), this)
    {}

    bool get_output_levels(sample_type* first, sample_type* last) const;
};

class AapRenderer::SourceChannel
{
  public:
    explicit SourceChannel(const Source* s)
      : source(*s)
    {}

    const Source& source;

    using iterator = decltype(source.begin());

    iterator begin() const { return source.begin(); }
    iterator end() const { return source.end(); }

    apf::BlockParameter<sample_type> stored_weight;
};


bool
AapRenderer::Source::get_output_levels(sample_type* first
    , sample_type* last) const
{
  assert(
      static_cast<sourcechannels_t::size_type>(std::distance(first, last))
      == this->sourcechannels.size());
  (void)last;

  for (const auto& channel: this->sourcechannels)
  {
    *first = channel.stored_weight;
    ++first;
  }

  return true;
}

class AapRenderer::RenderFunction
{
  public:
    RenderFunction(const Output& out) : _out(out) {}

    apf::CombineChannelsResult::type select(SourceChannel& in);

    sample_type operator()(sample_type in)
    {
      return in * _weight;
    }

    sample_type operator()(sample_type in, sample_type index)
    {
      return in * _interpolator(index);
    }

  private:
    sample_type _weight;
    apf::math::linear_interpolator<sample_type> _interpolator;

    const Output& _out;
};

class AapRenderer::Output : public _base::Output
{
  public:
    Output(const Params& p)
      : _base::Output(p)
      , _combiner(this->sourcechannels, this->buffer)
    {
      // TODO: add delay line (in some base class?)

      // TODO: amplitude correction for misplaced loudspeakers?
      //_weight = loudspeaker_distance / farthest_loudspeaker_distance;
    }

    APF_PROCESS(Output, _base::Output)
    {
      _combiner.process(RenderFunction(*this));
    }

  private:
    apf::CombineChannelsInterpolation<apf::cast_proxy<SourceChannel
      , sourcechannels_t>, buffer_type> _combiner;
};

void
AapRenderer::load_reproduction_setup()
{
  // TODO: find a way to avoid overwriting load_reproduction_setup()

  _base::load_reproduction_setup();

  // TODO: avoid code duplication between VBAP and AAP renderers!
  // TODO: move some stuff to base class?

  // TODO: check somehow if loudspeaker setup is reasonable?

  // TODO: get loudspeaker delays from setup?
  // delay_samples = size_t(delay * sample_rate + 0.5f)

  int normal_loudspeakers = 0;

  for (const auto& out: rtlist_proxy<Output>(this->get_output_list()))
  {
    if (out.model == LegacyLoudspeaker::subwoofer)
    {
      // TODO: something
    }
    else  // loudspeaker type == normal
    {
      ++normal_loudspeakers;
    }
  }

  if (normal_loudspeakers < 1)
  {
    throw std::logic_error("No loudspeakers found!");
  }

  if (!_ambisonics_order)
  {
    _ambisonics_order = (normal_loudspeakers - 1) / 2;
  }

  assert(_ambisonics_order > 0);

  SSR_VERBOSE("Using Ambisonics order " << _ambisonics_order << ".");

  // TODO: more things?
}

apf::CombineChannelsResult::type
AapRenderer::RenderFunction::select(SourceChannel& in)
{
  // TODO: take loudspeaker weight into account (for misplaced loudspeakers)?

  using apf::math::deg2rad;

  float two_times_order = 2 * _out.parent._ambisonics_order;

  auto weighting_factor = sample_type();

  if (_out.model == LegacyLoudspeaker::normal)
  {
    // WARNING: The reference offset is currently broken!

    float alpha_0  = deg2rad((_out.position).orientation().azimuth);
    float theta_pw = deg2rad(((Position(in.source.position) -
            Position(_out.parent.state.reference_position)).orientation()
          - Orientation(_out.parent.state.reference_rotation)).azimuth);

    // TODO: wrap angles?

    if (_out.parent._in_phase_rendering)
    {
      weighting_factor = std::pow(std::cos((alpha_0 - theta_pw) / 2)
          , two_times_order);
    }
    else
    {
      // check numerical stability
      if (std::abs(std::sin((alpha_0 - theta_pw) / 2)) < 0.0001f)
      {
        weighting_factor = 1;
      }
      else
      {
        weighting_factor
          = std::sin((two_times_order + 1) * (alpha_0 - theta_pw) / 2) /
           ((two_times_order + 1) * std::sin((alpha_0 - theta_pw) / 2));
      }
    }
  }
  else
  {
    // TODO: subwoofer gets weighting factor 1.0?
    weighting_factor = 1;
  }

  // Apply source volume, mute, ...
  weighting_factor *= in.source.weighting_factor;

  in.stored_weight = weighting_factor;

  auto old_weight = in.stored_weight.old();

  using namespace apf::CombineChannelsResult;

  if (old_weight == 0 && weighting_factor == 0)
  {
    return nothing;
  }
  else if (old_weight == weighting_factor)
  {
    _weight = weighting_factor;
    return constant;
  }
  else
  {
    _interpolator.set(old_weight, weighting_factor, _out.parent.block_size());
    return change;
  }
}

}  // namespace ssr

#endif
