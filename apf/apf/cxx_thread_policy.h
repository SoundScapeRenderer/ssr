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
/// C++ thread policy class.

#ifndef APF_CXX_THREAD_POLICY_H
#define APF_CXX_THREAD_POLICY_H

#ifndef _REENTRANT
#error You need to compile with _REENTRANT defined since this uses threads!
#endif

#ifndef APF_MIMOPROCESSOR_THREAD_POLICY
#define APF_MIMOPROCESSOR_THREAD_POLICY apf::cxx_thread_policy
#endif

#include <thread>
#include <chrono>  // for std::this_thread::sleep_for()
#include <mutex>
#include <condition_variable>

#include "apf/misc.h"  // for NonCopyable

namespace apf
{

/// @c thread_policy using the standard C++ threads library.
/// @see MimoProcessor
/// @ingroup apf_policies
class cxx_thread_policy
{
  public:
    using useconds_type = int;

    class Thread;
    template<typename F> struct ScopedThread;
    template<typename F> struct DetachedThread;
    using Lock = std::mutex;
    class Semaphore;

    unsigned default_number_of_threads()
    {
      return std::thread::hardware_concurrency();
    }

  protected:
     cxx_thread_policy() = default;  ///< Protected ctor.
    ~cxx_thread_policy() = default;  ///< Protected dtor.

  private:
    class ThreadBase;
};

class cxx_thread_policy::ThreadBase
{
  public:
    using native_handle_type = std::thread::native_handle_type;

    ThreadBase(ThreadBase&&)
    {
      // ThreadBase must be movable in order to be stored in a std::vector.
      // We never actually move it, so this should never be called:
      throw std::logic_error("This is just a work-around, don't move!");
    }

    void create(void* (*f)(void*), void* data)
    {
      _thread = std::thread(f, data);
    }

    bool join()
    {
      _thread.join();
      return true;
    }

    native_handle_type native_handle()
    {
      return _thread.native_handle();
    }

  protected:
    ThreadBase() = default;
    ~ThreadBase() = default;

    std::thread _thread;
};

template<typename F>
class cxx_thread_policy::ScopedThread : public ThreadBase, NonCopyable
{
  public:
    ScopedThread(F f, useconds_type usleeptime)
      : _kill_thread(false)
      , _function(f)
      , _usleeptime(usleeptime)
    {
      this->create(&_thread_aux, this);
    }

    ~ScopedThread()
    {
      _kill_thread = true;
      this->join();
    }

  private:
    static void* _thread_aux(void *arg)
    {
      static_cast<ScopedThread*>(arg)->_thread_function();
      return nullptr;
    }

    void _thread_function()
    {
      while (!_kill_thread)
      {
        _function();
        std::this_thread::sleep_for(std::chrono::microseconds(_usleeptime));
      }
    }

    volatile bool _kill_thread;
    F _function;
    useconds_type _usleeptime;
};

template<typename F>
class cxx_thread_policy::DetachedThread : public ThreadBase
{
  public:
    explicit DetachedThread(F f)
      : _function(f)
    {
      this->create(&_thread_aux, this);
      // We cannot get native handle after detaching, so we store it here:
      _native_handle = _thread.native_handle();
      _thread.detach();
    }

    native_handle_type native_handle() { return _native_handle; }

  private:
    static void* _thread_aux(void* arg)
    {
      static_cast<DetachedThread*>(arg)->_thread_function();
      return nullptr;
    }

    void _thread_function()
    {
      for (;;)
      {
        _function();
      }
    }

    F _function;
    native_handle_type _native_handle{0};
};

class cxx_thread_policy::Semaphore : NonCopyable
{
  // implementation stolen from http://stackoverflow.com/a/19659736/500098

  public:
    Semaphore(int count = 0) : _count{count} {}

    Semaphore(Semaphore&&)
    {
      // Semaphore must be movable in order to be stored in a std::vector.
      // We never actually move it, so this should never be called:
      throw std::logic_error("This is just a work-around, don't move!");
    }

    inline void post()
    {
      std::lock_guard<std::mutex> guard{_mtx};
      _count++;
      _cv.notify_one();
    }

    inline void wait()
    {
      std::unique_lock<std::mutex> lock{_mtx};
      _cv.wait(lock, [this]() { return _count > 0; });
      _count--;
    }

  private:
    std::mutex _mtx;
    std::condition_variable _cv;
    int _count;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
