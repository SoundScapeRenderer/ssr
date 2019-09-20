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
/// Multi-threaded MIMO (multiple input, multiple output) processor.

#ifndef APF_MIMOPROCESSOR_H
#define APF_MIMOPROCESSOR_H

#include <cassert>  // for assert()
#include <mutex>
#include <stdexcept>  // for std::logic_error

#include "apf/rtlist.h"
#include "apf/parameter_map.h"
#include "apf/misc.h"  // for NonCopyable
#include "apf/iterator.h" // for *_iterator, make_*_iterator(), cast_proxy_const
#include "apf/container.h" // for fixed_vector
#include "apf/threadtools.h" // for ScopedThread

#define APF_MIMOPROCESSOR_TEMPLATES template<typename Derived, typename interface_policy, typename query_policy>
#define APF_MIMOPROCESSOR_BASE MimoProcessor<Derived, interface_policy, query_policy>

/** Macro to create a @c Process struct and a corresponding member function.
 * @param name Name of the containing class
 * @param parent Parent class (must have an inner class @c Process).
 *   The class apf::MimoProcessor::ProcessItem can be used.
 *
 * Usage examples:
 *                                                                         @code
 * // In 'public' section of Output:
 * APF_PROCESS(Output, MimoProcessorBase::Output)
 * {
 *   // do something here, you have access to members of Output
 * }
 *
 * // In 'public' section of MyItem:
 * APF_PROCESS(MyItem, ProcessItem<MyItem>)
 * {
 *   // do something here, you have access to members of MyItem
 *   // MyItem has to be publicly derived from ProcessItem<MyItem>
 * }
 *                                                                      @endcode
 **/
#define APF_PROCESS(name, parent) \
struct Process : parent::Process { \
  explicit Process(name& ctor_arg) : parent::Process(ctor_arg) { \
    ctor_arg.APF_PROCESS_internal(); } }; \
void APF_PROCESS_internal()

namespace apf
{

// the default implementation does nothing
template<typename interface_policy, typename native_handle_type>
struct thread_traits
{
  static void set_priority(const interface_policy&, native_handle_type) {}
};

class enable_queries
{
  protected:
    enable_queries()
      // NB: 1 should be sufficient, but it doesn't seem to work:
      : _query_fifo(2)
    {}

    template<typename F>
    void start_query_thread(F& query_function, int usleeptime)
    {
      _query_thread = std::make_unique<QueryThread>(_query_fifo, usleeptime);

      // CAUTION: this must be called after interface_policy::activate()!
      // If not, an infinite recursion happens!
      _new_query(query_function);
    }

    // This is supposed to be called from the audio thread
    void process_query_commands()
    {
      _query_fifo.process_commands();
    }

    void stop_query_thread()
    {
      _keep_running = false;
      // Stop query thread:
      _query_thread.reset();
      // Empty query fifo in case there is still some cleanup to do:
      _query_fifo.deactivate();
    }

  private:
    class CleanupFunction
    {
      public:
        CleanupFunction(CommandQueue& fifo) : _fifo(fifo) {};
        void operator()() { _fifo.cleanup_commands(); }

      private:
        CommandQueue& _fifo;
    };

    struct QueryThread : ScopedThread<CleanupFunction>
    {
      QueryThread(CommandQueue& fifo, int usleeptime)
        : ScopedThread<CleanupFunction>(CleanupFunction(fifo), usleeptime)
      {}
    };

    template<typename F>
    class QueryCommand : public CommandQueue::Command
    {
      public:
        QueryCommand(F& query_function, enable_queries& parent)
          : _query_function(query_function)
          , _parent(parent)
        {}

        // This is called in the audio thread
        virtual void execute()
        {
          _query_function.query();
        }

        // This is called in the query thread
        virtual void cleanup()
        {
          if (_parent._keep_running)
          {
            _query_function.update();
            _parent._new_query(_query_function);  // "recursive" call!
          }
        }

      private:
        F& _query_function;
        enable_queries& _parent;
    };

    template<typename F>
    void _new_query(F& query_function)
    {
      _query_fifo.push(new QueryCommand<F>(query_function, *this));
    }

    std::atomic<bool> _keep_running{true};
    std::unique_ptr<QueryThread> _query_thread;
    CommandQueue _query_fifo;
};

class disable_queries
{
  protected:
    void process_query_commands() {}
    void stop_query_thread() {}
};

/** Multi-threaded multiple-input-multiple-output (MIMO) processor.
 * Derive your own class from MimoProcessor and also use it as first template
 * argument. This is called the "Curiously Recurring Template Pattern" (CRTP).
 * The rest of the template arguments are \ref apf_policies ("Policy-based
 * Design").
 *
 * @tparam Derived Your derived class -> CRTP!
 * @tparam interface_policy Policy class. You can use existing policies (e.g.
 *   jack_policy, pointer_policy<T*>) or write your own policy class.
 *
 * Example: @ref MimoProcessor
 **/
template<typename Derived
  , typename interface_policy
  , typename query_policy = disable_queries>
class MimoProcessor : public interface_policy
                    , public query_policy
                    , public CRTP<Derived>
                    , NonCopyable
{
  public:
    using typename interface_policy::sample_type;

    class Input;
    class Output;
    class DefaultInput;
    class DefaultOutput;

    /// Abstract base class for list items.
    struct Item : NonCopyable
    {
      virtual ~Item() = default;

      /// to be overwritten in the derived class
      virtual void process() = 0;
    };

    /** Base class for items which have a @c Process class.
     * Usage:
     *                                                                     @code
     * class MyItem : public ProcessItem<MyItem>
     * {
     *   public:
     *     APF_PROCESS(MyItem, ProcessItem<MyItem>)
     *     {
     *       // do something here, you have access to members of MyItem
     *     }
     * };
     *                                                                  @endcode
     * Multiple layers of inheritance are possible, but ProcessItem must
     * be instantiated with the most derived class!
     **/
    template<typename X>
    struct ProcessItem : Item, public CRTP<X>
    {
      struct Process { explicit Process(ProcessItem&) {} };

      virtual void process()
      {
        typename X::Process(this->derived());
      }
    };

    using rtlist_t = RtList<Item*>;

    /// Proxy class for accessing an RtList.
    /// @note This is for read-only access. Write access is only allowed in the
    ///   process() member function from within the object itself.
    template<typename T>
    struct rtlist_proxy : cast_proxy_const<T, rtlist_t>
    {
      rtlist_proxy(const rtlist_t& l) : cast_proxy_const<T, rtlist_t>(l) {}
    };

    /// Lock is released when it goes out of scope.
    class ScopedLock : NonCopyable
    {
      public:
        explicit ScopedLock(std::mutex& obj)
          : _obj(obj)
        {
          _obj.lock();
        }

        ~ScopedLock() { _obj.unlock(); }

      private:
        std::mutex& _obj;
    };

    bool activate()
    {
      _fifo.reactivate();  // no return value
      return interface_policy::activate();
    }

    /// This is only available when enable_queries is used
    template<typename F>
    bool activate(F& query_function, int usleeptime)
    {
      auto result = this->activate();
      this->start_query_thread(query_function, usleeptime);
      return result;
    }

    bool deactivate()
    {
      this->stop_query_thread();

      if (!interface_policy::deactivate()) return false;

      // All audio threads should be stopped now.

      // Inputs/Outputs push commands in their destructors -> we need a loop.
      do
      {
        // Exceptionally, this is called from the non-realtime thread:
        _fifo.process_commands();
        _fifo.cleanup_commands();
      }
      while (_fifo.commands_available());
      // The queue should be empty now.
      if (!_fifo.deactivate()) throw std::logic_error("Bug: FIFO not empty!");
      return true;

      // The lists can now be manipulated safely from the non-realtime thread.
    }

    void wait_for_rt_thread() { _fifo.wait(); }

    template<typename X>
    X* add()
    {
      return this->add(typename X::Params());
    }

    // TODO: find a way to get the outer type automatically
    template<typename P>
    typename P::outer* add(const P& p)
    {
      using X = typename P::outer;
      auto temp = p;
      temp.parent = &this->derived();
      return static_cast<X*>(_add_helper(new X(temp)));
    }

    void rem(Input* in) { _input_list.rem(in); }
    void rem(Output* out) { _output_list.rem(out); }

    const rtlist_t& get_input_list() const { return _input_list; }
    const rtlist_t& get_output_list() const { return _output_list; }

    const parameter_map params;

  protected:
    using MimoProcessorBase = APF_MIMOPROCESSOR_BASE;

    using rtlist_iterator = typename rtlist_t::iterator;
    using rtlist_const_iterator = typename rtlist_t::const_iterator;

    struct Process { Process(Derived&) {} };

    explicit MimoProcessor(const parameter_map& params = parameter_map());

    /// Protected non-virtual destructor
    ~MimoProcessor()
    {
      this->deactivate();
      _input_list.clear();
      _output_list.clear();
    }

    void _process_list(rtlist_t& l);
    void _process_list(rtlist_t& l1, rtlist_t& l2);

    CommandQueue _fifo;

  private:
    class WorkerThread : NonCopyable
    {
      public:
        WorkerThread(unsigned thread_number, MimoProcessor& parent)
          : _thread_number(thread_number)
          , _parent(parent)
          , _thread(std::thread(&WorkerThread::_thread_function, this))
        {
          // Set thread priority from interface_policy, if available
          thread_traits<interface_policy
            , std::thread::native_handle_type>::set_priority(parent
                , _thread.native_handle());
        }

        WorkerThread(WorkerThread&& other)
          : _parent{other._parent}
        {
          // WorkerThread must be movable to be stored in a std::vector.
          // We never actually move it, so this should never be called:
          throw std::logic_error("This is just a work-around, don't move!");
        }

        ~WorkerThread()
        {
          _keep_running.store(false, std::memory_order_release);
          this->cont_semaphore.post();
          _thread.join();
        }

        Semaphore cont_semaphore{0};
        Semaphore wait_semaphore{0};

      private:
        void _thread_function()
        {
          for (;;)
          {
            // wait for main audio thread
            this->cont_semaphore.wait();

            if (!_keep_running.load(std::memory_order_acquire)) { break; }

            _parent._process_selected_items_in_current_list(_thread_number);

            // report to main audio thread
            this->wait_semaphore.post();
          }
        }

        std::atomic<bool> _keep_running{true};
        unsigned _thread_number;
        MimoProcessor& _parent;

        std::thread _thread;  // Thread must be initialized after semaphores
    };

    class Xput;

    // This is called from the interface_policy
    virtual void process()
    {
      _fifo.process_commands();
      _process_list(_input_list);
      typename Derived::Process(this->derived());
      _process_list(_output_list);
      this->process_query_commands();
    }

    void _process_current_list_in_main_thread();
    void _process_selected_items_in_current_list(unsigned thread_number);

    Input* _add_helper(Input* in) { return _input_list.add(in); }
    Output* _add_helper(Output* out) { return _output_list.add(out); }

    // TODO: make "volatile"?
    rtlist_t* _current_list;

    /// Number of threads (main thread plus worker threads)
    const unsigned _num_threads;

    fixed_vector<WorkerThread> _thread_data;

    rtlist_t _input_list, _output_list;
};

/// @throw std::logic_error if CommandQueue cannot be deactivated.
APF_MIMOPROCESSOR_TEMPLATES
APF_MIMOPROCESSOR_BASE::MimoProcessor(const parameter_map& params_)
  : interface_policy(params_)
  , query_policy()
  , params(params_)
  , _fifo(params.get("fifo_size", size_t(1024)))
  , _current_list(nullptr)
  , _num_threads(params.get("threads", std::thread::hardware_concurrency()))
  , _input_list(_fifo)
  , _output_list(_fifo)
{
  assert(_num_threads > 0);

  // deactivate FIFO for non-realtime initializations
  if (!_fifo.deactivate()) throw std::logic_error("Bug: FIFO not empty!");

  // Create N-1 worker threads.  NOTE: Number 0 is reserved for the main thread.
  _thread_data.reserve(_num_threads - 1);
  for (unsigned i = 1; i < _num_threads; ++i)
  {
    _thread_data.emplace_back(i, *this);
  }
}

APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_process_list(rtlist_t& l)
{
  _current_list = &l;
  _process_current_list_in_main_thread();
}

APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_process_list(rtlist_t& l1, rtlist_t& l2)
{
  // TODO: extend for more than two lists?

  // Note: this was not conforming to C++03.
  // According to C++03 iterators to the spliced elements are invalidated!
  // In C++11 this was fixed.
  // see http://stackoverflow.com/q/143156

  // see also http://stackoverflow.com/q/7681376

  auto temp = l2.begin();
  l2.splice(temp, l1);  // join lists: "L2 = L1 + L2"
  _process_list(l2);
  l1.splice(l1.end(), l2, l2.begin(), temp);  // restore original lists

  // not exception-safe (original lists are not restored), but who cares?
}

APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_process_selected_items_in_current_list(
    unsigned thread_number)
{
  assert(_current_list);

  unsigned n = 0;
  for (auto& i: *_current_list)
  {
    if (thread_number == n++ % _num_threads)
    {
      assert(i);
      i->process();
    }
  }
}

APF_MIMOPROCESSOR_TEMPLATES
void
APF_MIMOPROCESSOR_BASE::_process_current_list_in_main_thread()
{
  assert(_current_list);
  if (_current_list->empty()) return;

  // wake all threads
  for (auto& it: _thread_data) it.cont_semaphore.post();

  _process_selected_items_in_current_list(0);

  // wait for worker threads
  for (auto& it: _thread_data) it.wait_semaphore.wait();
}

APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::Xput : public APF_MIMOPROCESSOR_BASE::Item
{
  public:
    // Parameters for an Input or Output.
    // You can add your own parameters by deriving from it.
    struct Params : parameter_map
    {
      Params() : parent(nullptr) {}
      Derived* parent;

      Params& operator=(const parameter_map& p)
      {
        this->parameter_map::operator=(p);
        return *this;
      }
    };

    Derived& parent;

  protected:
    /// Protected Constructor.
    /// @throw std::logic_error if parent == NULL
    explicit Xput(const Params& p)
      : parent(*(p.parent
            ? p.parent
            : throw std::logic_error("Bug: In/Output: parent == 0!")))
    {}
};

/// %Input class.
APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::Input : public APF_MIMOPROCESSOR_BASE::Xput
                                    , public interface_policy::Input
                                    , public CRTP<typename Derived::Input>
{
  public:
    struct Params : APF_MIMOPROCESSOR_BASE::Xput::Params
    {
      using Xput::Params::operator=;
      using outer = typename Derived::Input;  // see add()
    };

    struct Process { Process(Input&) {} };

    explicit Input(const Params& p)
      : Xput(p)
      , interface_policy::Input(*p.parent, p)
    {}

  private:
    virtual void process()
    {
      this->fetch_buffer();
      typename Derived::Input::Process(this->derived());
    }
};

/// %Input class with begin() and end().
APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::DefaultInput : public APF_MIMOPROCESSOR_BASE::Input
{
  public:
    using Params = typename Input::Params;
    using iterator = typename Input::iterator;

    explicit DefaultInput(const Params& p) : Input(p) {}

    iterator begin() const { return this->buffer.begin(); }
    iterator   end() const { return this->buffer.end(); }
};

/// %Output class.
APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::Output : public APF_MIMOPROCESSOR_BASE::Xput
                                     , public interface_policy::Output
                                     , public CRTP<typename Derived::Output>
{
  public:
    struct Params : APF_MIMOPROCESSOR_BASE::Xput::Params
    {
      using Xput::Params::operator=;
      using outer = typename Derived::Output;  // see add()
    };

    struct Process { Process(Output&) {} };

    explicit Output(const Params& p)
      : Xput(p)
      , interface_policy::Output(*p.parent, p)
    {}

  private:
    virtual void process()
    {
      this->fetch_buffer();
      typename Derived::Output::Process(this->derived());
    }
};

/// %Output class with begin() and end().
APF_MIMOPROCESSOR_TEMPLATES
class APF_MIMOPROCESSOR_BASE::DefaultOutput : public APF_MIMOPROCESSOR_BASE::Output
{
  public:
    using Params = typename Output::Params;
    using iterator = typename Output::iterator;

    DefaultOutput(const Params& p) : Output(p) {}

    iterator begin() const { return this->buffer.begin(); }
    iterator   end() const { return this->buffer.end(); }
};

}  // namespace apf

#undef APF_MIMOPROCESSOR_TEMPLATES
#undef APF_MIMOPROCESSOR_BASE

#endif
