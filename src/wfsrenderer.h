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
/// Wave Field Synthesis renderer.

#ifndef SSR_WFSRENDERER_H
#define SSR_WFSRENDERER_H

#include "loudspeakerrenderer.h"
#include "ssr_global.h"

#include "apf/convolver.h"  // for apf::conv::...
#include "apf/blockdelayline.h"  // for NonCausalBlockDelayLine
#include "apf/sndfiletools.h"  // for apf::load_sndfile
#include "apf/combine_channels.h"  // for apf::raised_cosine_fade, ...

// TODO: make more flexible option:
#define WEIGHTING_OLD
//#define WEIGHTING_DELFT

namespace ssr
{

class WfsRenderer : public SourceToOutput<WfsRenderer, LoudspeakerRenderer>
{
  private:
    using _base = SourceToOutput<WfsRenderer, ssr::LoudspeakerRenderer>;

  public:
    static const char* name() { return "WFS-Renderer"; }

    class Input;
    class Source;
    class SourceChannel;
    class Output;
    class RenderFunction;

    WfsRenderer(const apf::parameter_map& params)
      : _base(params)
      , _fade(this->block_size())
      , _max_delay(this->params.get("delayline_size", 0))
      , _initial_delay(this->params.get("initial_delay", 0))
    {
      // TODO: compute "ideal" initial delay?
      // TODO: check if given initial delay is sufficient?

      // TODO: make option --prefilter=none?

      // TODO: get pre-filter from reproduction setup!
      // TODO: allow alternative files for different sample rates

      SndfileHandle prefilter;
      try
      {
        prefilter = apf::load_sndfile(
            this->params.get("prefilter_file", ""), this->sample_rate(), 1);
      }
      catch (const std::logic_error& e)
      {
        throw std::logic_error(
            "Error loading WFS pre-equalization filter file: "
            + std::string(e.what()));
      }

      size_t size = prefilter.frames();

      auto ir = apf::fixed_vector<sample_type>(size);

      size = prefilter.readf(ir.data(), size);

      // TODO: warning if size changed?
      // TODO: warning if size == 0?

      _pre_filter.reset(new apf::conv::Filter(this->block_size()
            , ir.begin(), ir.end()));
    }

    APF_PROCESS(WfsRenderer, _base)
    {
      this->_process_list(_source_list);
    }

  private:
    apf::raised_cosine_fade<sample_type> _fade;
    std::unique_ptr<apf::conv::Filter> _pre_filter;

    size_t _max_delay, _initial_delay;
};

class WfsRenderer::Input : public _base::Input
{
  public:
    friend class Source;  // give access to _delayline

    Input(const Params& p)
      : _base::Input(p)
      // TODO: check if _pre_filter != 0!
      , _convolver(*this->parent._pre_filter)
      , _delayline(this->parent.block_size(), this->parent._max_delay
          , this->parent._initial_delay)
    {}

    APF_PROCESS(Input, _base::Input)
    {
      _convolver.add_block(this->buffer.begin());
      _delayline.write_block(_convolver.convolve());
    }

  private:
    apf::conv::StaticConvolver _convolver;
    apf::NonCausalBlockDelayLine<sample_type> _delayline;
};

class WfsRenderer::SourceChannel : public apf::has_begin_and_end<
                          apf::NonCausalBlockDelayLine<sample_type>::circulator>
{
  public:
    SourceChannel(const Source& s)
      : crossfade_mode(0)
      , weighting_factor(0.0f)
      , delay(0)
      , source(s)
    {}

    void update();

    int crossfade_mode;
    apf::BlockParameter<sample_type> weighting_factor;
    apf::BlockParameter<int> delay;

    const Source& source;

    // TODO: avoid making those public:
    using apf::has_begin_and_end<apf::NonCausalBlockDelayLine<sample_type>
      ::circulator>::_begin;
    using apf::has_begin_and_end<apf::NonCausalBlockDelayLine<sample_type>
      ::circulator>::_end;
};

class WfsRenderer::RenderFunction
{
  public:
    RenderFunction(const Output& out) : _in(0), _out(out) {}

    apf::CombineChannelsResult::type select(SourceChannel& in);

    sample_type operator()(sample_type in)
    {
      return in * _new_factor;
    }

    sample_type operator()(sample_type in, apf::fade_out_tag)
    {
      return in * _old_factor;
    }

    void update()
    {
      assert(_in);
      _in->update();
    }

  private:
    sample_type _old_factor, _new_factor;

    SourceChannel* _in;
    const Output& _out;
};

class WfsRenderer::Output : public _base::Output
{
  public:
    friend class Source;  // to be able to see _sourcechannels

    Output(const Params& p)
      : _base::Output(p)
      , _combiner(this->sourcechannels, this->buffer, this->parent._fade)
    {}

    APF_PROCESS(Output, _base::Output)
    {
      _combiner.process(RenderFunction(*this));
    }

  private:
    apf::CombineChannelsCrossfade<apf::cast_proxy<SourceChannel
      , sourcechannels_t>, buffer_type
      , apf::raised_cosine_fade<sample_type>> _combiner;
};

class WfsRenderer::Source : public _base::Source
{
  private:
    void _process();

  public:
    Source(const Params& p)
      : _base::Source(p, p.parent->get_output_list().size(), *this)
      , delayline(p.input->_delayline)
    {}

    APF_PROCESS(Source, _base::Source)
    {
      _process();
    }

    bool get_output_levels(sample_type* first, sample_type* last) const
    {
      assert(size_t(std::distance(first, last)) == this->sourcechannels.size());

      auto channel = this->sourcechannels.begin();

      for ( ; first != last; ++first)
      {
        *first = channel->weighting_factor;
        ++channel;
      }
      return true;
    }

    const apf::NonCausalBlockDelayLine<sample_type>& delayline;

    //private:
    bool _focused;
};

void WfsRenderer::Source::_process()
{
  if (this->model == ::Source::plane)
  {
    // do nothing, focused-ness is irrelevant for plane waves
    _focused = false;
  }
  else
  {
    _focused = true;
    for (const auto& out: rtlist_proxy<Output>(_input.parent.get_output_list()))
    {
      // subwoofers have to be ignored!
      if (out.model == Loudspeaker::subwoofer) continue;

      // TODO: calculate with inner product

      // angle (modulo) between the line connecting source<->loudspeaker
      // and the loudspeaker orientation

      // TODO: avoid getting reference 2 times (see select())
      auto ls = DirectionalPoint(out);
      auto ref = DirectionalPoint(out.parent.state.reference_position
          , out.parent.state.reference_orientation);
      ls.transform(ref);

      auto a = apf::math::wrap(angle(ls.position - this->position
            , ls.orientation), 2 * apf::math::pi<sample_type>());

      auto halfpi = apf::math::pi<sample_type>()/2;

      if (a < halfpi || a > 3 * halfpi)
      {
        // if at least one loudspeaker "turns its back" to the source, the
        // source is considered non-focused
        _focused = false;
        break;
      }
    }
  }

  // TODO: active sources?
}

void WfsRenderer::SourceChannel::update()
{
  _begin = this->source.delayline.get_read_circulator(this->delay);
  _end = _begin + source.parent.block_size();
}

apf::CombineChannelsResult::type
WfsRenderer::RenderFunction::select(SourceChannel& in)
{
  _in = &in;

  // define a restricted area around loudspeakers to avoid division by zero:
  const float safety_radius = 0.01f; // 1 cm

  // TODO: move reference calculation to WfsRenderer::Process?
  auto ref = DirectionalPoint(_out.parent.state.reference_position
      , _out.parent.state.reference_orientation);

  // TODO: this is actually wrong!
  // We use it to be compatible with the (also wrong) GUI implementation.
  auto ref_off = ref;
  ref_off.transform(DirectionalPoint(
        _out.parent.state.reference_offset_position
        , _out.parent.state.reference_offset_orientation));

  sample_type weighting_factor = 1;
  float float_delay = 0;

  auto ls = Loudspeaker(_out);
  auto src_pos = in.source.position;

  // TODO: shortcut if in.source.weighting_factor == 0

  // Transform loudspeaker position according to reference and offset
  ls.transform(ref);

  float reference_distance = (ls.position - ref_off.position).length();

  float source_ls_distance = (ls.position - src_pos).length();

  switch (in.source.model) // check if point source or plane wave or ...
  {
    case ::Source::point:
      if (ls.model == Loudspeaker::subwoofer)
      {
        // the delay is calculated to be correct on the reference position
        // delay can be negative!
        float_delay = (src_pos - ref_off.position).length()
          - reference_distance;

        // setting the subwoofer amplitude to 1 is the unwritten standard 
        // (cf. AAP renderer)
        weighting_factor = 1.0f;

        break; // step out of switch
      }

      float_delay = source_ls_distance;
      assert(float_delay >= 0);

      float denominator;
      if (float_delay < safety_radius) denominator = std::sqrt(safety_radius);
      else denominator = std::sqrt(float_delay);

      // TODO: does this really do the right thing?
      weighting_factor = cos(angle(ls.position - src_pos,
            ls.orientation)) / denominator;

      if (weighting_factor < 0.0f)
      {
        // negative weighting factor is only valid for focused sources
        if (in.source._focused)
        {
          // loudspeaker selection:

          // this could also be done by using the cosine function instead of
          // inner product

          // calculate inner product of those two vectors:
          // this = source
          auto lhs = ls.position - src_pos;
          auto rhs = ref_off.position - src_pos;

          // rhs will be zero if src_pos is exactly at the reference position
          // This would would lead to the inner product being zero.
          // lhs can't be zero (checked before to avoid infinite gain)
          if (rhs.x == 0.0f && rhs.y == 0.0f)
          {
            rhs.y = -0.001;
          }

          // TODO: write inner product function in Position class
          if ((lhs.x * rhs.x + lhs.y * rhs.y) < 0.0f)
          {
            // if the inner product is less than zero, the source is more or
            // less between the loudspeaker and the reference
            float_delay = -float_delay;
            weighting_factor = -weighting_factor;

#if defined(WEIGHTING_OLD)
            (void)source_ls_distance;  // avoid "unused variable" warning
#elif defined(WEIGHTING_DELFT)
            // limit to a maximum of 2.0
            weighting_factor *= std::min(2.0f, std::sqrt(source_ls_distance
                / (reference_distance + source_ls_distance)));
#endif
          }
          else
          {
            // ignored focused point source
            weighting_factor = 0;
            break;
          }
        }
        else // non-focused and weighting_factor < 0
        {
          // ignored non-focused point source
          weighting_factor = 0;
          break;
        }
      }
      else if(weighting_factor > 0.0f) // positive weighting factor
      {
        if (!in.source._focused)
        {
          // non-focused point source

#if defined(WEIGHTING_OLD)
#elif defined(WEIGHTING_DELFT)
          // WARNING: division by zero is possible!
          weighting_factor *= std::sqrt(source_ls_distance
              / (reference_distance + source_ls_distance));
#endif
        }
        else // focused
        {
          // ignored focused point source
          break;
        }
      }
      else
      {
        // this should never happen: Weighting factor is 0 or +-Inf or NaN!
        break;
      }
      break;

    case ::Source::plane:
      if (ls.model == Loudspeaker::subwoofer)
      {
        weighting_factor = 1.0f; // TODO: is this correct?
        // the delay is calculated to be correct on the reference position
        // delay can be negative!
        float_delay
          = DirectionalPoint(in.source.position, in.source.orientation)
          .plane_to_point_distance(ref_off.position) - reference_distance;
        break; // step out of switch
      }

      // weighting factor is determined by the cosine of the angle
      // difference between plane wave direction and loudspeaker direction
      weighting_factor = cos(angle(in.source.orientation, ls.orientation));
      // check if loudspeaker is active for this source
      if (weighting_factor < 0)
      {
        // ignored plane wave
        weighting_factor = 0;
        break;
      }

      float_delay = DirectionalPoint(in.source.position, in.source.orientation)
        .plane_to_point_distance(ls.position);

      if (float_delay < 0.0)
      {
        // "focused" plane wave
      }
      else // positive delay
      {
        // plane wave
      }
      break;

    default:
      //WARNING("Unknown source model");
      break;
  } // switch source model

#if defined(WEIGHTING_OLD)
  if (in.source.model == ::Source::point)
  {
    // compensate for inherent distance decay (approx. 1/sqrt(r))
    // no compensation closer to 0.5 m to the reference
    // this is the same operation for focused and non-focused sources
    // exclude subwoofers as there is no inherent amplitude decay
    if (ls.model != Loudspeaker::subwoofer)
    {
      weighting_factor *= 
        std::sqrt(std::max((src_pos - ref_off.position).length(), 0.5f)); 
    }
  }
#elif defined(WEIGHTING_DELFT)
// TODO: Undo default distance attenuation
#endif
  
  // apply the gain factor of the current source
  weighting_factor *= in.source.weighting_factor;

  // apply tapering
  weighting_factor *= ls.weight;

  assert(weighting_factor >= 0.0f);

  // delay in seconds
  float_delay *= c_inverse;
  // delay in samples
  float_delay *= _out.parent.sample_rate();

  // TODO: check for negative delay and print an error if > initial_delay

  // TODO: do proper rounding
  // TODO: enable interpolated reading from delay line.
  int int_delay = static_cast<int>(float_delay + 0.5f);

  if (in.source.delayline.delay_is_valid(int_delay))
  {
    in.delay = int_delay;
    in.weighting_factor = weighting_factor;
  }
  else
  {
    // TODO: some sort of warning message?

    in.delay = 0;
    in.weighting_factor = 0;
  }

  assert(in.weighting_factor.exactly_one_assignment());
  assert(in.delay.exactly_one_assignment());

  _old_factor = in.weighting_factor.old();
  _new_factor = in.weighting_factor;

  using namespace apf::CombineChannelsResult;
  auto crossfade_mode = apf::CombineChannelsResult::type();

  if (_old_factor == 0 && _new_factor == 0)
  {
    crossfade_mode = nothing;
  }
  else if (_old_factor == _new_factor && !in.delay.changed())
  {
    crossfade_mode = constant;
  }
  else if (_old_factor == 0)
  {
    crossfade_mode = fade_in;
  }
  else if (_new_factor == 0)
  {
    crossfade_mode = fade_out;
  }
  else
  {
    crossfade_mode = change;
  }

  if (crossfade_mode == nothing || crossfade_mode == fade_in)
  {
    // No need to read the delayline here
  }
  else
  {
    in._begin = in.source.delayline.get_read_circulator(in.delay.old());
    in._end = in._begin + _out.parent.block_size();
  }

  return crossfade_mode;
}

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
