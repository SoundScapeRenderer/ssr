// Example that shows how to get information out of the realtime thread(s).

#include <iostream>

#include "apf/mimoprocessor.h"
#include "apf/jack_policy.h"
#include "apf/shareddata.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                  , apf::jack_policy
                  , apf::enable_queries>
{
  public:
    MyProcessor()
      : MimoProcessorBase()
      , ch(_fifo, '_')
    {}

    // MyProcessor doesn't process anything, no Process struct needed

    // To be assigned to in the main thread, read from in the audio thread
    apf::SharedData<char> ch;
};

class MyQuery
{
  public:
    MyQuery(MyProcessor& parent)
      : _parent{parent}
    {}

    // This is called in the audio thread
    void query()
    {
      _ch = _parent.ch;
    }

    // This is called in the query thread
    void update()
    {
      std::cout << _ch << std::flush;
    }

  private:
    MyProcessor& _parent;
    char _ch;
};

void sleep(int sec)
{
  std::this_thread::sleep_for(std::chrono::seconds(sec));
}

int main()
{
  MyProcessor processor;
  MyQuery query{processor};
  processor.activate(query, 100 * 1000);
  sleep(2);
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
