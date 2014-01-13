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

/// @file
/// C policy (= pointer based) for MimoProcessor's audio_interface.

#ifndef APF_POINTER_POLICY_H
#define APF_POINTER_POLICY_H

#include <cassert>  // for assert()
#include "apf/parameter_map.h"
#include "apf/iterator.h"  // for has_begin_and_end

#ifndef APF_MIMOPROCESSOR_SAMPLE_TYPE
#define APF_MIMOPROCESSOR_SAMPLE_TYPE float
#endif
#ifndef APF_MIMOPROCESSOR_INTERFACE_POLICY
#define APF_MIMOPROCESSOR_INTERFACE_POLICY apf::pointer_policy<APF_MIMOPROCESSOR_SAMPLE_TYPE*>
#endif

namespace apf
{

template<typename T> class pointer_policy;  // no implementation, use <T*>!

/// @c interface_policy which uses plain pointers.
/// @see MimoProcessor
/// @ingroup apf_policies
template<typename T>
class pointer_policy<T*>
{
  public:
    using sample_type = T;

    class Input;
    class Output;

    void audio_callback(int n, T* const* in, T* const* out);

    // for now, do nothing:
    bool activate() const { return true; }
    bool deactivate() const { return true; }

    int block_size() const { return _block_size; }
    int sample_rate() const { return _sample_rate; }

    int in_channels() const { return _next_input_id; }
    int out_channels() const { return _next_output_id; }

  protected:
    explicit pointer_policy(const parameter_map& params = parameter_map())
      : _sample_rate(params.get<int>("sample_rate"))
      , _block_size(params.get<int>("block_size"))
      , _next_input_id(0)
      , _next_output_id(0)
      , _in(0)
      , _out(0)
    {}

    virtual ~pointer_policy() = default;

  private:
    virtual void process() = 0;

    /// Generate next higher input ID.
    /// @warning This function is \b not re-entrant!
    int get_next_input_id() { return _next_input_id++; }

    /// @see get_next_input_id()
    int get_next_output_id() { return _next_output_id++; }

    const int _sample_rate;
    const int _block_size;

    int _next_input_id;
    int _next_output_id;
    T* const* _in;
    T* const* _out;
};

/** This has to be called for each audio block.
 * @attention You must make sure that there is enough memory available for input
 *   and output data. Inputs and outputs can be added, but @p in and @p out must
 *   be enlarged accordingly.
 * @warning @p in and @p out can only grow bigger, if inputs/outputs are @em
 *   removed, the corresponding pointer of @p in/@p out must remain at its
 *   place!
 * @param n block size
 * @param in pointer to an array of pointers to input channels
 * @param out pointer to an array of pointers to output channels
 **/
template<typename T>
void
pointer_policy<T*>::audio_callback(int n, T* const* in, T* const* out)
{
  assert(n == this->block_size());
  (void)n;  // avoid "unused parameter" warning

  _in = in;
  _out = out;
  this->process();
}

template<typename T>
class pointer_policy<T*>::Input
{
  public:
    using iterator = T const*;

    struct buffer_type : has_begin_and_end<iterator> { friend class Input; };

    void fetch_buffer()
    {
      this->buffer._begin = _parent._in[_id];
      this->buffer._end   = this->buffer._begin + _parent.block_size();
    }

    buffer_type buffer;

  protected:
    Input(pointer_policy& parent, const parameter_map&)
      : _parent(parent)
      , _id(_parent.get_next_input_id())
    {}

    ~Input() = default;

  private:
    Input(const Input&); Input& operator=(const Input&);  // deactivated

    pointer_policy& _parent;
    const int _id;
};

template<typename T>
class pointer_policy<T*>::Output
{
  public:
    using iterator = T*;

    struct buffer_type : has_begin_and_end<iterator> { friend class Output; };

    void fetch_buffer()
    {
      this->buffer._begin = _parent._out[_id];
      this->buffer._end   = this->buffer._begin + _parent.block_size();
    }

    buffer_type buffer;

  protected:
    Output(pointer_policy& parent, const parameter_map&)
      : _parent(parent)
      , _id(_parent.get_next_output_id())
    {}

    ~Output() = default;

  private:
    Output(const Output&); Output& operator=(const Output&);  // deactivated

    pointer_policy& _parent;
    const int _id;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
