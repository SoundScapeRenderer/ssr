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

    explicit SharedData(CommandQueue& fifo, X&& def = X())
      : _fifo(fifo)
      , _data(std::forward<X>(def))
    {}

    /// Get contained data. Use this if the conversion operator cannot be used.
    const X& get() const { return _data; }

    operator const X&() const { return this->get(); }

    void operator=(const X& rhs)
    {
      _fifo.push(new SetCommand(&_data, rhs));
    }

    void operator=(X&& rhs)
    {
      _fifo.push(new SetCommand(&_data, std::forward<X>(rhs)));
    }

    void set_from_rt_thread(X&& data)
    {
      _data = std::forward<X>(data);
    }

    friend bool
    operator==(const SharedData& lhs, const X& rhs) { return lhs._data == rhs; }
    friend bool
    operator==(const X& lhs, const SharedData& rhs) { return lhs == rhs._data; }
    friend bool
    operator!=(const SharedData& lhs, const X& rhs) { return lhs._data != rhs; }
    friend bool
    operator!=(const X& lhs, const SharedData& rhs) { return lhs != rhs._data; }

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

    SetCommand(X* pointer, X&& data)
      : _pointer(pointer)
      , _data(std::forward<X>(data))
    {
      assert(pointer != nullptr);
    }

  private:
    virtual void execute()
    {
      using std::swap;
      swap(*_pointer, _data);
    }

    virtual void cleanup() {}

    X* _pointer;
    X _data;  ///< copy of data or moved-to data
};

}  // namespace apf

#endif
