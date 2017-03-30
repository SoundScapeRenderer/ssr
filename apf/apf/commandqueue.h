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
/// Command queue.

#ifndef APF_COMMANDQUEUE_H
#define APF_COMMANDQUEUE_H

#include <unistd.h> // for usleep()
#include <cassert>  // for assert()

#include "apf/lockfreefifo.h"

namespace apf
{

/** Manage command queue from non-realtime thread to realtime thread.
 * Commands can be added in the non-realtime thread with push().
 *
 * Commands are executed when process_commands() is called from the realtime
 * thread.
 **/
class CommandQueue : NonCopyable
{
  public:
    /// Abstract base class for realtime commands.
    /// These commands are passed through queues into the realtime thread and
    /// after execution back to the non-realtime thread for cleanup.
    struct Command : NonCopyable
    {
      /// Empty virtual destructor.
      virtual ~Command() {}

      /// The actual implementation of the command. This is called from the
      /// realtime thread. Overwritten in the derived class.
      virtual void execute() = 0;

      /// Cleanup of resources. This is called from the non-realtime thread.
      /// Overwritten in the derived class.
      virtual void cleanup() = 0;
    };

    /// Dummy command to synchronize with non-realtime thread.
    class WaitCommand : public Command
    {
      public:
        /// Constructor. @param done is set to @b true when cleanup() is called.
        WaitCommand(bool& done) : _done(done) {}

      private:
        virtual void execute() { }
        virtual void cleanup() { _done = true; }

        bool& _done;
    };

    /// @name Functions to be called from the non-realtime thread
    /// If there are multiple non-realtime threads, access has to be locked!
    //@{

    /// Constructor.
    /// @param size maximum number of commands in queue.
    explicit CommandQueue(size_t size)
      : _in_fifo(size)
      , _out_fifo(size)
      , _active(true)
    {}

    /// Destructor.
    /// @attention Commands in the cleanup queue are cleaned up, but commands in
    /// the process queue are ignored and their memory is not freed!
    ~CommandQueue()
    {
      this->cleanup_commands();
      // TODO: warning if process queue is not empty?
      // TODO: if inactive -> process commands (if active -> ???)
    }

    inline void push(Command* cmd);

    inline void wait();

    /// Clean up all commands in the cleanup-queue.
    /// @note This function must be called from the non-realtime thread.
    void cleanup_commands()
    {
      Command* cmd;
      while ((cmd = _out_fifo.pop()) != nullptr) { _cleanup(cmd); }
    }

    // TODO: avoid return value?
    /// Deactivate queue; process following commands in the non-realtime thread.
    /// @return @b true on success
    /// @note The queue must be empty. If not, the queue is @em not deactivated
    ///   and @b false is returned.
    inline bool deactivate()
    {
      this->cleanup_commands();
      if (_in_fifo.empty()) _active = false;
      return !_active;
    }

    /// Re-activate queue. @see deactivate().
    inline void reactivate()
    {
      this->cleanup_commands();
      assert(_in_fifo.empty());
      _active = true;
    }

    //@}

    /// @name Functions to be called from the realtime thread
    //@{

    /// Execute all commands in the queue.
    /// After execution, the commands are queued for cleanup in the non-realtime
    /// thread.
    /// @note This function must be called from the realtime thread.
    void process_commands()
    {
      Command* cmd;
      while ((cmd = _in_fifo.pop()) != nullptr)
      {
        cmd->execute();
        bool result = _out_fifo.push(cmd);
        // If _out_fifo is full, cmd is not cleaned up!
        // This is very unlikely to happen (if not impossible).
        assert(result && "Error in _out_fifo.push()!");
        (void)result;  // avoid "unused-but-set-variable" warning
      }
    }

    /// Check if commands are available.
    /// @return @b true if commands are available.
    bool commands_available() const
    {
      return !_in_fifo.empty();
    }

    //@}

  private:
    /// Clean up and delete a command @p cmd
    void _cleanup(Command* cmd)
    {
      assert(cmd != nullptr);
      cmd->cleanup();
      delete cmd;
    }

    /// Queue of commands to execute in realtime thread
    LockFreeFifo<Command*> _in_fifo;
    /// Queue of executed commands to delete in non-realtime thread
    LockFreeFifo<Command*> _out_fifo;

    bool _active;  ///< default: true
};

/** Push a command to be executed in the realtime thread.
 * The command will be cleaned up when it comes back from the
 * realtime thread.
 * If the CommandQueue is inactive, the command is not queued but executed and
 * cleaned up immediately.
 * @param cmd The command to be executed. 
 **/
void CommandQueue::push(Command* cmd)
{
  if (!_active)
  {
    cmd->execute();
    _cleanup(cmd);
    return;
  }

  // First remove all commands from _out_fifo.
  // This ensures that it's not going to be full which would block 
  // process_commands() and its calling realtime thread.  
  this->cleanup_commands();

  // Now push the command on _in_fifo; if the FIFO is full: retry, retry, ...
  while (!_in_fifo.push(cmd))
  {
    // We don't really know if that ever happens, so we abort in debug-mode:
    assert(false && "Error in _in_fifo.push()!");
    // TODO: avoid this usleep()?
    usleep(50);
  }
}

/** Wait for realtime thread.
 * Push an empty command and wait for its return.
 **/
void CommandQueue::wait()
{
  bool done = false;
  this->push(new WaitCommand(done));

  this->cleanup_commands();
  while (!done)
  {
    // TODO: avoid this usleep()?
    usleep(50);
    this->cleanup_commands();
  }
}

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
