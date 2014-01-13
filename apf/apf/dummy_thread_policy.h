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
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
