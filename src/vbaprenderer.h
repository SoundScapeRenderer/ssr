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
/// Vector Base Amplitude Panning renderer.

#ifndef SSR_VBAPRENDERER_H
#define SSR_VBAPRENDERER_H

#include "loudspeakerrenderer.h"
#include "apf/combine_channels.h"

namespace ssr
{

/** Vector Base Amplitude Panning Renderer.
 * The loudspeaker weights are calculated according the the vector base
 * formulation proposed by Ville Pulkki in "Virtual Sound Source Positioning
 * Using Vector Base Amplitude Panning", Journal of the Audio Engineering
 * Society (JAES), Vol.45(6), June 1997.\par
 * The speaker weights are calculated as follows:
 * \f$ \displaystyle g_{left} = \frac{\cos \phi \sin \phi_0 +
 * \sin \phi \cos \phi_0}{2 \cos \phi_0 \sin \phi_0} \f$
 * \f$ \displaystyle g_{right} = \frac{\cos \phi \sin \phi_0 -
 * \sin \phi \cos \phi_0}{2 \cos \phi_0 \sin \phi_0} \f$
 * \par
 * whereby \f$ \displaystyle \phi\f$ denotes blah, blah
 **/
class VbapRenderer : public LoudspeakerRenderer<VbapRenderer>
{
  private:
    using _base = ssr::LoudspeakerRenderer<VbapRenderer>;

  public:
    static const char* name() { return "VBAP-Renderer"; }

    class Source;
    class Output;
    class RenderFunction;

    explicit VbapRenderer(const apf::parameter_map& params)
      : _base(params)
      , _max_angle(params.get("vbap_max_angle", apf::math::deg2rad(180.0)))
      , _overhang_angle(
          params.get("vbap_overhang_angle", apf::math::deg2rad(30.0)))
      , _overhang_func(2 * _overhang_angle)
      , _reference_offset_position(this->state.reference_offset_position.get())
    {}

    void load_reproduction_setup();

    APF_PROCESS(VbapRenderer, _base)
    {
      // WARNING: The reference offset is currently broken!
      // To make it work, we have to fiddle a bit.
      auto temp = this->state.reference_offset_position.get();
      temp.rotate(-90.0);
      _reference_offset_position = temp;

      // TODO: once the reference offset is implemented correctly, do only this:
      //_reference_offset_position = this->state.reference_offset_position();

      if (_reference_offset_position.changed())
      {
        // TODO: check if reference is 'inside' the array?

        _update_angles();

        // The (circular) order will always be the same in a convex array.
        // However, angles may be wrapped around 0 and 2*pi.
        // Only in this case the list has to be re-sorted.
        if (_sorted_loudspeakers.back() < _sorted_loudspeakers.front())
        {
          _sort_loudspeakers();
        }

        _update_valid_sections();
      }

      _absolute_reference_offset_position
        = Position(_reference_offset_position).rotate(
            this->state.reference_orientation)
        + this->state.reference_position;

      _process_list(_source_list);
    }

  private:
    struct LoudspeakerEntry
    {
      // Note: This is non-explicit to allow comparison with angle
      LoudspeakerEntry(float angle_, bool valid_section_ = false
          , const Output* output_ = nullptr)
        : angle(angle_)
        , valid_section(valid_section_)
        , ls_ptr(output_)
      {}

      // Comparison operator is needed for std::sort() and std::upper_bound()
      friend
      bool operator<(const LoudspeakerEntry& lhs, const LoudspeakerEntry& rhs)
      {
        return lhs.angle < rhs.angle;
      }

      float angle;  // radians
      bool valid_section;  // Is the angle to the next loudspeaker < _max_angle?
      const Output* ls_ptr;
    };

    struct LoudspeakerWeight
    {
      const Output* ls_ptr = nullptr;
      float weight = 0.0;
    };

    void _update_angles();
    void _sort_loudspeakers();
    void _update_valid_sections();

    float _max_angle, _overhang_angle;

    apf::math::raised_cosine<float> _overhang_func;

    std::vector<LoudspeakerEntry> _sorted_loudspeakers;

    apf::BlockParameter<Position> _reference_offset_position;
    Position _absolute_reference_offset_position;
};

class VbapRenderer::Source : public _base::Source
{
  public:
    Source(const Params& p)
      : _base::Source(p)
    {}

    APF_PROCESS(Source, _base::Source)
    {
      // NOTE: reference_offset_orientation doesn't affect rendering

      float incidence_angle = apf::math::wrap_two_pi(apf::math::deg2rad(
            ((this->position
              - this->parent._absolute_reference_offset_position).orientation()
             - this->parent.state.reference_orientation).azimuth));

      auto l_begin = this->parent._sorted_loudspeakers.begin();
      auto l_end = this->parent._sorted_loudspeakers.end();

      auto second = apf::make_circular_iterator(l_begin, l_end
          , std::upper_bound(l_begin, l_end, incidence_angle));

      auto first = second;

      --first;

      auto weights = _calculate_loudspeaker_weights(incidence_angle
          , *first, *second);

      // Apply source volume, mute, ...
      weights.first.weight *= this->weighting_factor;
      weights.second.weight *= this->weighting_factor;

      this->loudspeaker_weights = weights;

      assert(this->loudspeaker_weights.first.exactly_one_assignment());
      assert(this->loudspeaker_weights.second.exactly_one_assignment());
    }

    bool get_output_levels(sample_type* first, sample_type* last) const
    {
      assert(size_t(std::distance(first, last))
          == parent.get_output_list().size());
      auto current = first;

      for (const auto& out: rtlist_proxy<Output>(parent.get_output_list()))
      {
        // TODO: handle subwoofers!

        if (this->loudspeaker_weights.first.get().ls_ptr == &out)
        {
          *current = this->loudspeaker_weights.first.get().weight;
        }
        else if (this->loudspeaker_weights.second.get().ls_ptr == &out)
        {
          *current = this->loudspeaker_weights.second.get().weight;
        }
        else
        {
          *current = 0;
        }

        ++current;
      }
      assert(current == last);
      (void)last;

      return true;
    }

  private:
    std::pair<LoudspeakerWeight, LoudspeakerWeight>
    _calculate_loudspeaker_weights(float angle
          , const LoudspeakerEntry& first, const LoudspeakerEntry& second);

  public:
    std::pair<apf::BlockParameter<LoudspeakerWeight>
            , apf::BlockParameter<LoudspeakerWeight>> loudspeaker_weights;
};

class VbapRenderer::RenderFunction
{
  public:
    RenderFunction(const Output& out) : _out(out) {}

    apf::CombineChannelsResult::type select(const Source& in);

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

class VbapRenderer::Output : public _base::Output
{
  public:
    Output(const Params& p)
      : _base::Output(p)
      , _combiner(this->parent._source_list, this->buffer)
    {
      // TODO: handle loudspeaker delays?
      // TODO: optional delay line?
    }

    APF_PROCESS(Output, _base::Output)
    {
      _combiner.process(RenderFunction(*this));
    }

  private:
    apf::CombineChannelsInterpolation<rtlist_proxy<Source>, buffer_type>
      _combiner;
};

void
VbapRenderer::load_reproduction_setup()
{
  // TODO: find a way to avoid overwriting load_reproduction_setup()

  _base::load_reproduction_setup();

  // TODO: check somehow if loudspeaker setup is reasonable?

  // TODO: get loudspeaker delays from setup?
  // delay_samples = size_t(delay * sample_rate + 0.5f)

  for (const auto& out: rtlist_proxy<Output>(this->get_output_list()))
  {
    if (out.model == Loudspeaker::subwoofer)
    {
      // TODO: put subwoofers in separate list? (for get_output_levels())
    }
    else  // loudspeaker type == normal
    {
      _sorted_loudspeakers.emplace_back(0, false, &out);
    }
  }

  if (_sorted_loudspeakers.size() < 1)
  {
    throw std::logic_error("No loudspeakers found!");
  }

  _update_angles();
  _sort_loudspeakers();
  _update_valid_sections();
}

apf::CombineChannelsResult::type
VbapRenderer::RenderFunction::select(const Source& in)
{
  const auto& ls = in.loudspeaker_weights;

  assert(ls.first.get().ls_ptr != ls.second.get().ls_ptr
      || ls.first.get().ls_ptr == nullptr);

  auto get_weight = [this] (const LoudspeakerWeight& first
      , const LoudspeakerWeight& second)
  {
    float weight = 0;
    if (first.ls_ptr == &_out) { weight = first.weight; }
    else if (second.ls_ptr == &_out) { weight = second.weight; }
    return weight;
  };

  auto old_weight = get_weight(ls.first.old(), ls.second.old());
  auto new_weight = get_weight(ls.first, ls.second);

  using namespace apf::CombineChannelsResult;

  if (old_weight == 0 && new_weight == 0)
  {
    return nothing;
  }
  else if (old_weight == new_weight)
  {
    _weight = new_weight;
    return constant;
  }
  else
  {
    _interpolator.set(old_weight, new_weight, in.parent.block_size());
    return change;
  }
}

void
VbapRenderer::_update_angles()
{
  for (auto& ls: _sorted_loudspeakers)
  {
    // NOTE: reference_offset_orientation doesn't affect rendering

    ls.angle = apf::math::wrap_two_pi(apf::math::deg2rad((ls.ls_ptr->position
        - _reference_offset_position).orientation().azimuth));
  }
}

void
VbapRenderer::_sort_loudspeakers()
{
  std::sort(_sorted_loudspeakers.begin(), _sorted_loudspeakers.end());
}

void
VbapRenderer::_update_valid_sections()
{
  for (auto ls = _sorted_loudspeakers.begin()
      ; ls != _sorted_loudspeakers.end()
      ; ++ls)
  {
    auto next_ls = ls;
    if (++next_ls == _sorted_loudspeakers.end())
    {
      // If there is only one loudspeaker, it will always be invalid.
      next_ls = _sorted_loudspeakers.begin();
      ls->valid_section = (ls->angle + _max_angle) >= (next_ls->angle
          + apf::math::deg2rad(360.0));
    }
    else
    {
      ls->valid_section = (ls->angle + _max_angle) >= next_ls->angle;
    }
  }
}

std::pair<VbapRenderer::LoudspeakerWeight, VbapRenderer::LoudspeakerWeight>
VbapRenderer::Source::_calculate_loudspeaker_weights(float source_angle
    , const LoudspeakerEntry& first, const LoudspeakerEntry& second)
{
  using namespace apf::math;

  // Constructed with defaults: nullptr/0.0
  std::pair<LoudspeakerWeight, LoudspeakerWeight> weights;

  if (first.valid_section)
  {
    // phi_0: angle from halfway between loudspeakers to left loudspeaker
    // phi: angle from halfway between loudspeakers to source

    float phi_0 = wrap_two_pi(second.angle - first.angle) / 2.0f;

    float phi = wrap_two_pi(source_angle - first.angle - phi_0);

    float num1 = std::cos(phi) * std::sin(phi_0);
    float num2 = std::sin(phi) * std::cos(phi_0);
    float den  = 2 * std::cos(phi_0) * std::sin(phi_0);

    weights.second.weight = (num1 + num2) / den;
    weights.first.weight = (num1 - num2) / den;

    weights.second.ls_ptr = second.ls_ptr;
    weights.first.ls_ptr = first.ls_ptr;
  }
  else
  {
    float max_overhang = this->parent._overhang_angle;
    const auto& overhang_func = this->parent._overhang_func;

    float overhang = wrap_two_pi(source_angle - first.angle);

    if (overhang < max_overhang)
    {
      weights.first.weight = overhang_func(overhang);
      weights.first.ls_ptr = first.ls_ptr;
    }

    overhang = wrap_two_pi(second.angle - source_angle);

    if (overhang < max_overhang)
    {
      weights.second.weight = overhang_func(overhang);
      weights.second.ls_ptr = second.ls_ptr;
    }
  }
  return weights;
}

}  // namespace ssr

#endif
