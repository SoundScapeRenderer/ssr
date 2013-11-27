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

// Example that shows how to get information out of the realtime thread(s).

#include <iostream>

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/shareddata.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::jack_policy
                    , apf::posix_thread_policy
                    , apf::enable_queries>
{
  public:
    MyProcessor()
      : MimoProcessorBase()
      , ch(_fifo, '_')
      , _query(*this)
      , _query_thread(QueryThread(_query_fifo), 1000*1000 / this->block_size())
    {}

    // MyProcessor doesn't process anything, no Process struct needed

    void start_querying()
    {
      this->new_query(_query);
    }

    apf::SharedData<char> ch;

  private:
    class my_query
    {
      public:
        my_query(MyProcessor& parent)
          : _parent(parent)
        {}

        void query()
        {
          _ch = _parent.ch;
        }

        void update()
        {
          std::cout << _ch << std::flush;
        }

      private:
        MyProcessor& _parent;
        char _ch;
    } _query;

    ScopedThread<QueryThread> _query_thread;
};

int main()
{
  MyProcessor processor;
  processor.activate();
  processor.start_querying();
  sleep(3);
  processor.ch = '*';
  sleep(1);
  processor.ch = '+';
  sleep(1);
  processor.ch = '#';
  sleep(1);
  processor.ch = '.';
  sleep(1);
  processor.deactivate();
  std::cout << std::endl;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
