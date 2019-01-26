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
/// Generic renderer.

#ifndef SSR_GENERICRENDERER_H
#define SSR_GENERICRENDERER_H

#include "loudspeakerrenderer.h"

#include "apf/convolver.h"  // for apf::conv::*
#include "apf/sndfiletools.h"  // for apf::load_sndfile
#include "apf/combine_channels.h"  // for apf::raised_cosine_fade, ...

namespace ssr
{

class GenericRenderer : public SourceToOutput<GenericRenderer
                                                          , LoudspeakerRenderer>
{
  private:
    using _base = SourceToOutput<GenericRenderer, ssr::LoudspeakerRenderer>;

  public:
    static const char* name() { return "GenericRenderer"; }

    using Input = _base::DefaultInput;
    class Source;
    struct SourceChannel;
    class Output;
    class RenderFunction;

    GenericRenderer(const apf::parameter_map& params)
      : _base(params)
      , _fade(this->block_size())
    {}

    APF_PROCESS(GenericRenderer, _base)
    {
      this->_process_list(_source_list);
    }

  private:
    apf::raised_cosine_fade<sample_type> _fade;
};

struct GenericRenderer::SourceChannel : apf::has_begin_and_end<sample_type*>
{
  template<typename In>
  explicit SourceChannel(const Source& s, In first, In last);

  // out-of-class definition because of cyclic dependencies with Source
  void update();
  void convolve(sample_type weight);

  const Source& source;

  apf::conv::StaticOutput convolver;
};

class GenericRenderer::Source : public _base::Source
{
  public:
    explicit Source(const Params& p)
      : _base::Source(p)
      , _weighting_factor()
    {
      using matrix_t = apf::fixed_matrix<sample_type>;

      size_t outputs = this->parent.get_output_list().size();

      auto ir_file = apf::load_sndfile(
          p.get<std::string>("properties-file"), this->parent.sample_rate()
          , outputs);

      size_t size = ir_file.frames();

      auto ir_data = matrix_t(size, outputs);

      // TODO: check return value?
      ir_file.readf(ir_data.data(), size);

      size_t block_size = this->parent.block_size();

      _convolver.reset(new apf::conv::Input(block_size
            , apf::conv::min_partitions(block_size, size)));

      this->sourcechannels.reserve(outputs);

      for (matrix_t::slices_iterator slice = ir_data.slices.begin()
          ; slice != ir_data.slices.end()
          ; slice++)
      {
        this->sourcechannels.emplace_back(*this, slice->begin(), slice->end());
      }
    }

    APF_PROCESS(Source, _base::Source)
    {
      _weighting_factor = this->weighting_factor;

      _convolver->add_block(_input.begin());

      assert(_weighting_factor.exactly_one_assignment());
    }

    apf::BlockParameter<sample_type> _weighting_factor;

    std::unique_ptr<apf::conv::Input> _convolver;
};

template<typename In>
GenericRenderer::SourceChannel::SourceChannel(const Source& s
    , In first, In last)
  : source(s)
  // TODO: assert s._convolver != 0?
  , convolver(*s._convolver, first, last)
{}

void GenericRenderer::SourceChannel::update()
{
  this->convolve(this->source._weighting_factor);
}

void GenericRenderer::SourceChannel::convolve(sample_type weight)
{
  // TODO: check if convolver == 0?
  _begin = this->convolver.convolve(weight);
  _end = _begin + this->convolver.block_size();
}

class GenericRenderer::RenderFunction
{
  public:
    RenderFunction() : _in(0) {}

    apf::CombineChannelsResult::type select(SourceChannel& in)
    {
      _in = & in;

      const auto& factor = in.source._weighting_factor;

      using namespace apf::CombineChannelsResult;

      if (factor.both() == 0) return nothing;

      if (factor.old() == 0) return fade_in;

      in.convolve(factor.old());

      if (factor == 0) return fade_out;

      if (!factor.changed()) return constant;

      return change;
    }

    void update()
    {
      assert(_in);
      _in->update();
    }

  private:
    SourceChannel* _in;
};

class GenericRenderer::Output : public _base::Output
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

}  // namespace ssr

#endif
