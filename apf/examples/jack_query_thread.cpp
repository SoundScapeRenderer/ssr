// Example that shows how to get information out of the realtime thread(s).

#include <iostream>

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/cxx_thread_policy.h"
#include "apf/shareddata.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::jack_policy
                    , apf::cxx_thread_policy
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
