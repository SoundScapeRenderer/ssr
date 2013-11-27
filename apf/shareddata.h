/******************************************************************************
 * Copyright © 2012-2013 Institut für Nachrichtentechnik, Universität Rostock *
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

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
