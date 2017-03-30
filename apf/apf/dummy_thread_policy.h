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
/// Dummy thread policy class.

#ifndef APF_DUMMY_THREAD_POLICY_H
#define APF_DUMMY_THREAD_POLICY_H

#include <stdexcept>  // for std::logic_error

#ifndef APF_MIMOPROCESSOR_THREAD_POLICY
#define APF_MIMOPROCESSOR_THREAD_POLICY apf::dummy_thread_policy
#endif

#define APF_DUMMY_THREAD_POLICY_ERROR throw std::logic_error( \
    "'dummy_thread_policy' can only be used with a single thread!")

namespace apf
{

/// @c thread_policy without functionality. Can only be used for single-threaded
///   processing.
/// @see MimoProcessor
/// @ingroup apf_policies
class dummy_thread_policy
{
  public:
    using useconds_type = int;

    class Thread;
    template<typename F> struct ScopedThread;
    template<typename F> struct DetachedThread;
    class Lock;
    class Semaphore;

    unsigned default_number_of_threads() { return 1; }

  protected:
     dummy_thread_policy() = default;  ///< Protected ctor.
    ~dummy_thread_policy() = default;  ///< Protected dtor.
};

class dummy_thread_policy::Thread
{
  public:
    using native_handle_type = int;

    void create(void* (*f)(void*), void* data)
    {
      (void)f;  // avoid "unused parameter" warning
      (void)data;
      APF_DUMMY_THREAD_POLICY_ERROR;
    }

    bool join()
    {
      APF_DUMMY_THREAD_POLICY_ERROR;
      return false;
    }

    native_handle_type native_handle() const
    {
      APF_DUMMY_THREAD_POLICY_ERROR;
      return -1;
    }
};

template<typename F>
struct dummy_thread_policy::ScopedThread : Thread
{
  ScopedThread(F, useconds_type)
  {
    APF_DUMMY_THREAD_POLICY_ERROR;
  }
};

template<typename F>
struct dummy_thread_policy::DetachedThread : Thread
{
  explicit DetachedThread(F)
  {
    APF_DUMMY_THREAD_POLICY_ERROR;
  }
};

class dummy_thread_policy::Lock
{
  public:
    Lock() {}

    int lock()
    {
      APF_DUMMY_THREAD_POLICY_ERROR;
      return -1;
    }

    int unlock()
    {
      APF_DUMMY_THREAD_POLICY_ERROR;
      return -1;
    }
};

class dummy_thread_policy::Semaphore
{
  public:
    using value_type = unsigned int;

    explicit Semaphore(value_type = 0)
    {
      APF_DUMMY_THREAD_POLICY_ERROR;
    }

    bool post()
    {
      APF_DUMMY_THREAD_POLICY_ERROR;
      return false;
    }

    bool wait()
    {
      APF_DUMMY_THREAD_POLICY_ERROR;
      return false;
    }
};

}  // namespace apf

#undef APF_DUMMY_THREAD_POLICY_ERROR

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
