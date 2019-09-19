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
/// Utility classes for the use with std::thread.

#ifndef APF_THREADTOOLS_H
#define APF_THREADTOOLS_H

#include <thread>
#include <chrono>  // for std::this_thread::sleep_for()
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "apf/misc.h"  // for NonCopyable

namespace apf
{

template<typename F>
class ScopedThread : NonCopyable
{
  public:
    ScopedThread(F f, int usleeptime)
      : _keep_running(true)
      , _function(f)
      , _sleeptime(std::chrono::microseconds(usleeptime))
      , _thread(std::thread(&ScopedThread::_thread_function, this))
    {}

    ~ScopedThread()
    {
      _keep_running.store(false, std::memory_order_release);
      _thread.join();
    }

  private:
    void _thread_function()
    {
      while (_keep_running.load(std::memory_order_acquire))
      {
        _function();
        std::this_thread::sleep_for(_sleeptime);
      }
    }

    std::atomic<bool> _keep_running;
    F _function;
    std::chrono::microseconds _sleeptime;
    std::thread _thread;
};

class Semaphore : NonCopyable
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
