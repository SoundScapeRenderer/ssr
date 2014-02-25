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
/// POSIX thread policy class.

#ifndef APF_POSIX_THREAD_POLICY_H
#define APF_POSIX_THREAD_POLICY_H

#ifndef _REENTRANT
#error You need to compile with _REENTRANT defined since this uses threads!
#endif

#ifndef APF_MIMOPROCESSOR_THREAD_POLICY
#define APF_MIMOPROCESSOR_THREAD_POLICY apf::posix_thread_policy
#endif

// Unnamed semaphores are not implemented on Mac OS X, so we use named
// semaphores with an auto-generated name.
// That's a hack.
// But it was easier than to use some OSX-specific stuff.
// If you want to use proper unnamed semaphores, define APF_UNNAMED_SEMAPHORES
// TODO: proper synchronisation for OSX, go back to unnamed for Linux.
#ifndef APF_UNNAMED_SEMAPHORES
#define APF_PSEUDO_UNNAMED_SEMAPHORES
#endif

#include <stdexcept>  // for std::runtime_error
#include <cstring>  // for std::strerror()
#include <pthread.h>
#include <semaphore.h>
#include <cerrno>
#include <unistd.h>  // for usleep()
#include <thread>  // for std::thread::hardware_concurrency()

#ifdef APF_PSEUDO_UNNAMED_SEMAPHORES
#include <fcntl.h>  // for O_CREAT, O_EXCL
#include "apf/stringtools.h"  // for apf::str::A2S()
#endif

#include "apf/misc.h"  // for NonCopyable

namespace apf
{

/// @c thread_policy using the POSIX thread library.
/// @see MimoProcessor
/// @ingroup apf_policies
class posix_thread_policy
{
  public:
    using useconds_type = useconds_t;

    template<typename F> class ScopedThread;
    template<typename F> class DetachedThread;
    class Lock;  // TODO: read-write lock?
    class Semaphore;

    unsigned default_number_of_threads()
    {
      // POSIX doesn't have it's own way to do it, so we borrow it from C++11:
      return std::thread::hardware_concurrency();
    }

  protected:
     posix_thread_policy() = default;  ///< Protected ctor.
    ~posix_thread_policy() = default;  ///< Protected dtor.

  private:
    class ThreadBase;
};

class posix_thread_policy::ThreadBase
{
  public:
    using native_handle_type = pthread_t;

    void create(void* (*f)(void*), void* data)
    {
      if (pthread_create(&_thread_id, 0, f, data))
      {
        throw std::runtime_error("Can't create thread!");
      }
    }

    bool join() { return !pthread_join(_thread_id, 0); }

    native_handle_type native_handle() const { return _thread_id; }

  protected:
    ThreadBase() = default;
    ~ThreadBase() = default;

  private:
    native_handle_type _thread_id;
};

template<typename F>
class posix_thread_policy::ScopedThread : public ThreadBase, NonCopyable
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
      static_cast<ScopedThread*>(arg)->_thread();
      return nullptr;
    }

    void _thread()
    {
      while (!_kill_thread)
      {
        _function();
        usleep(_usleeptime);
      }
    }

    volatile bool _kill_thread;
    F _function;
    useconds_type _usleeptime;
};

template<typename F>
class posix_thread_policy::DetachedThread : public ThreadBase
{
  public:
    explicit DetachedThread(F f)
      : _function(f)
    {
      this->create(&_thread_aux, this);
      pthread_detach(this->native_handle());  // return value is ignored!
    }

  private:
    static void* _thread_aux(void* arg)
    {
      static_cast<DetachedThread*>(arg)->_thread();
      return nullptr;
    }

    void _thread()
    {
      for (;;)
      {
        _function();
      }
    }

    F _function;
};

/** Inner type Lock.
 * Wrapper class for a mutex.
 **/
class posix_thread_policy::Lock : NonCopyable
{
  public:
    // TODO: parameter: initial lock state?
    Lock()
    {
      if (pthread_mutex_init(&_lock, nullptr))
      {
        throw std::runtime_error("Can't init mutex. (impossible !!!)");
      }
    }

    // TODO: change return type to bool?
    int   lock() { return pthread_mutex_lock(  &_lock); }
    int unlock() { return pthread_mutex_unlock(&_lock); }

    // TODO: trylock?

  private:
    pthread_mutex_t _lock;
};

class posix_thread_policy::Semaphore : NonCopyable
{
  public:
    using value_type = unsigned int;

    explicit Semaphore(value_type value = 0)
#ifdef APF_PSEUDO_UNNAMED_SEMAPHORES
      // Create a unique dummy name from object pointer
      : _name("/apf_" + apf::str::A2S(this))
      , _sem_ptr(sem_open(_name.c_str(), O_CREAT | O_EXCL, 0600, value))
    {
      if (!_sem_ptr)
#else
      : _sem_ptr(&_semaphore)
    {
      if (sem_init(_sem_ptr, 0, value))
#endif
      {
        throw std::runtime_error("Error initializing Semaphore! ("
            + std::string(std::strerror(errno)) + ")");
      }
    }

    Semaphore(Semaphore&&) = default;

    ~Semaphore()
    {
#ifdef APF_PSEUDO_UNNAMED_SEMAPHORES
      sem_unlink(_name.c_str());  // Only for named semaphores
#else
      sem_destroy(_sem_ptr);  // Only for unnamed semaphores
#endif
    }

    bool post() { return sem_post(_sem_ptr) == 0; }
    bool wait() { return sem_wait(_sem_ptr) == 0; }

  private:
#ifdef APF_PSEUDO_UNNAMED_SEMAPHORES
    const std::string _name;
#else
    sem_t _semaphore;
#endif
    sem_t* const _sem_ptr;
};

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
