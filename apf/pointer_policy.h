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

    void audio_callback(size_t n, T* const* in, T* const* out);

    // for now, do nothing:
    bool activate() const { return true; }
    bool deactivate() const { return true; }

    size_t block_size() const { return _block_size; }
    size_t sample_rate() const { return _sample_rate; }

    int in_channels() const { return _next_input_id; }
    int out_channels() const { return _next_output_id; }

  protected:
    explicit pointer_policy(const parameter_map& params = parameter_map())
      : _sample_rate(params.get<size_t>("sample_rate"))
      , _block_size(params.get<size_t>("block_size"))
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

    const size_t _sample_rate;
    const size_t _block_size;

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
pointer_policy<T*>::audio_callback(size_t n, T* const* in, T* const* out)
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
