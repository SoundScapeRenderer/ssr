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
/// Shared data.

#ifndef APF_SHAREDDATA_H
#define APF_SHAREDDATA_H

#include "apf/commandqueue.h"

namespace apf
{

template<typename X>
class SharedData
{
  public:
    class SetCommand;

    explicit SharedData(CommandQueue& fifo, const X& def = X())
      : _fifo(fifo)
      , _data(def)
    {
      // TODO: use reserve() for std::strings and std::vectors? using traits?
      // the size can be given as third (optional) argument.
      // see src/scene.h for container_traits.
      // ... or maybe use swap() instead of assignment?
    }

    /// Get contained data. Use this if the conversion operator cannot be used.
    const X& get() const { return _data; }

    operator const X&() const { return this->get(); }

    void operator=(const X& rhs)
    {
      _fifo.push(new SetCommand(&_data, rhs));
    }

  private:
    CommandQueue& _fifo;
    X _data;
};

template<typename X>
class SharedData<X>::SetCommand : public CommandQueue::Command
{
  public:
    SetCommand(X* pointer, const X& data)
      : _pointer(pointer)
      , _data(data)
    {
      assert(pointer != nullptr);
    }

  private:
    virtual void execute() { *_pointer = _data; }
    virtual void cleanup() {}

    X* _pointer;
    X _data;  ///< copy of data!
};

}  // namespace apf

#endif
