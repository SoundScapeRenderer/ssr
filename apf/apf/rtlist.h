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
/// A (under certain circumstances) realtime-safe list.

#ifndef APF_RTLIST_H
#define APF_RTLIST_H

#include <list>
#include <algorithm>  // for std::find()

#include "apf/commandqueue.h"

namespace apf
{

template<typename T> class RtList; // no implementation, use <T*>!

/** A list for realtime access and non-realtime modification.
 * It is normally used by a realtime thread and a non-realtime thread at the
 * same time.
 *
 * The list is created and modified (using add(), rem(), ...) by the
 * non-realtime thread. These functions are not safe for multiple non-realtime
 * threads; access has to be locked in this case.
 *
 * Before the realtime thread can access the list elements, it has to call
 * CommandQueue::process_commands() to synchronize.
 *
 * TODO: more information about which functions are allowed in which thread.
 **/
template<typename T>
class RtList<T*> : NonCopyable
{
  public:
    using list_t = typename std::list<T*>;
    using value_type = typename list_t::value_type;
    using size_type = typename list_t::size_type;
    using iterator = typename list_t::iterator;
    using const_iterator = typename list_t::const_iterator;

    class AddCommand;  // no implementation, use <T*>!
    class RemCommand;  // no implementation, use <T*>!
    class ClearCommand;  // no implementation, use <T*>!

    // Default constructor is not allowed!

    /// Constructor.
    /// @param fifo the CommandQueue
    explicit RtList(CommandQueue& fifo)
      : _fifo(fifo)
    {}

    /// Destructor.
    /// Elements can be removed - via rem() or clear() - before
    /// the destructor is called. But if not, they are deleted here.
    ~RtList()
    {
      for (auto& delinquent: _the_actual_list) delete delinquent;
      _the_actual_list.clear();
    }

    /// Add an element to the list.
    /// @param item Pointer to the list item
    /// @return the same pointer
    /// @note Ownership is passed to the list!
    template<typename X>
    X* add(X* item)
    {
      _fifo.push(new AddCommand(_the_actual_list, item));
      return item;
    }

    /// Add a range of elements to the list.
    /// @param first Begin of range to be added
    /// @param last End of range to be added
    /// @note Ownership is passed to the list!
    template<typename ForwardIterator>
    void add(ForwardIterator first, ForwardIterator last)
    {
      _fifo.push(new AddCommand(_the_actual_list, first, last));
    }

    /// Remove an element from the list.
    void rem(T* to_rem)
    {
      _fifo.push(new RemCommand(_the_actual_list, to_rem));
    }

    /// Remove a range of elements from the list.
    /// @param first Iterator to the first item
    /// @param last Past-the-end iterator
    template<typename ForwardIterator>
    void rem(ForwardIterator first, ForwardIterator last)
    {
      _fifo.push(new RemCommand(_the_actual_list, first, last));
    }

    /// Remove all elements from the list.
    void clear()
    {
      _fifo.push(new ClearCommand(_the_actual_list));
    }

    /// Splice another RtList into the RtList. @see @c std::list::splice()
    void splice(iterator position, RtList& x)
    {
      assert(&_fifo == &x._fifo);
      _the_actual_list.splice(position, x._the_actual_list);
    }

    /// Splice a part of another RtList into the RtList.
    void splice(iterator position, RtList& x, iterator first, iterator last)
    {
      assert(&_fifo == &x._fifo);
      _the_actual_list.splice(position, x._the_actual_list, first, last);
    }

    ///@{ @name Functions to be called from the realtime thread
    iterator       begin()       { return _the_actual_list.begin(); }
    const_iterator begin() const { return _the_actual_list.begin(); }
    iterator       end()         { return _the_actual_list.end(); }
    const_iterator end()   const { return _the_actual_list.end(); }
    bool           empty() const { return _the_actual_list.empty(); }
    size_type      size()  const { return _the_actual_list.size(); }
    ///@}

  private:
    CommandQueue& _fifo;
    list_t _the_actual_list;
};

/// Command to add an element to a list.
template<typename T>
class RtList<T*>::AddCommand : public CommandQueue::Command
{
  public:
    /// Constructor to add a single item.
    /// @param dst_list List to which the element will be added
    /// @param element Pointer to the element that will be added
    AddCommand(list_t& dst_list, T* element)
      : _splice_list(1, element) // make list with one element
      , _dst_list(dst_list)
    {
      assert(element != nullptr);
    }

    /// Constructor to add a bunch of items at once.
    /// @param dst_list List to which the elements will be added
    /// @param first Iterator to the first item to be added
    /// @param last Past-the-end iterator
    template<typename InputIterator>
    AddCommand(list_t& dst_list, InputIterator first, InputIterator last)
      : _splice_list(first, last)
      , _dst_list(dst_list)
    {}

    virtual void execute()
    {
      _dst_list.splice(_dst_list.end(), _splice_list);
    }

    // Empty function, because no cleanup is necessary
    virtual void cleanup() {}

  private:
    list_t _splice_list;  // List of elements to be added
    list_t& _dst_list;  // Destination list
};

/// Command to remove an element from a list.
template<typename T>
class RtList<T*>::RemCommand : public CommandQueue::Command
{
  public:
    /// Constructor to remove a single item.
    /// @param dst_list List from which the item will be removed
    /// @param delinquent Pointer to the item which will be removed
    RemCommand(list_t& dst_list, T* delinquent)
      : _dst_list(dst_list)
      , _delinquents(1, delinquent)
    {}

    /// Constructor to remove a bunch of items at once.
    /// @param dst_list List from which the elements will be removed
    /// @param first Iterator to first item to be removed
    /// @param last Past-the-end iterator
    template<typename InputIterator>
    RemCommand(list_t& dst_list, InputIterator first, InputIterator last)
      : _dst_list(dst_list)
      , _delinquents(first, last)
    {}

    /// The actual implementation of the command.
    /// @throw std::logic_error if item(s) is/are not found
    virtual void execute()
    {
      for (auto& i: _delinquents)
      {
        auto delinquent = std::find(_dst_list.begin(), _dst_list.end(), i);
        if (delinquent != _dst_list.end())
        {
          // Note: destruction order is reverse
          _splice_list.splice(_splice_list.begin(), _dst_list, delinquent);
        }
        else
        {
          throw std::logic_error("RemCommand: Item not found!");
        }
      }
    }

    // this might be dangerous/unexpected.
    // but i would need a synchronized Command, which waited
    // for the operation to complete, otherwise.
    virtual void cleanup()
    {
      for (auto& delinquent: _splice_list) delete delinquent;
      _splice_list.clear();
    }

  private:
    list_t _splice_list;  // List of elements to be removed
    list_t& _dst_list;  // Destination list
    list_t _delinquents;  // Temporary list of elements to be removed
};

/// Command to remove all elements from a list.
template<typename T>
class RtList<T*>::ClearCommand : public CommandQueue::Command
{
  public:
    /// Constructor.
    /// @param dst_list List from which all elements will be removed
    ClearCommand(list_t& dst_list)
      : _dst_list(dst_list)
    {}

    virtual void execute()
    {
      _delinquents.swap(_dst_list);
    }

    virtual void cleanup()
    {
      for (auto& delinquent: _delinquents) delete delinquent;
      _delinquents.clear();
    }

  private:
    list_t _delinquents;  ///< List of elements to be removed
    list_t& _dst_list;  ///< Destination list
};

}  // namespace apf

#endif
