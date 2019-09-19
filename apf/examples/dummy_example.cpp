// This example is used in the Doxygen documentation to MimoProcessor.

#include "apf/mimoprocessor.h"
#include "apf/pointer_policy.h"

class MyProcessor : public apf::MimoProcessor<MyProcessor
                    , apf::pointer_policy<float*>>
{
  public:
    using Input = MimoProcessorBase::DefaultInput;

    class MyIntermediateThing : public ProcessItem<MyIntermediateThing>
    {
      public:
        // you can create other classes and use them in their own RtList, as
        // long as they are derived from ProcessItem<YourClass> and have a
        // Process class publicly derived from ProcessItem<YourClass>::Process.

        // This can be facilitated with this macro call:
        APF_PROCESS(MyIntermediateThing, ProcessItem<MyIntermediateThing>)
        {
          // do your processing here!
        }
    };

    class Output : public MimoProcessorBase::DefaultOutput
    {
      public:
        explicit Output(const Params& p)
          : MimoProcessorBase::DefaultOutput(p)
        {}

        APF_PROCESS(Output, MimoProcessorBase::DefaultOutput)
        {
          // this->buffer.begin() and this->buffer.end(): access to audio data
        }
    };

    MyProcessor(const apf::parameter_map& p)
      : MimoProcessorBase(p)
      , _intermediate_list(_fifo)
    {
      this->add<Input>();
      _intermediate_list.add(new MyIntermediateThing());
      this->add<Output>();
      this->activate();
    }

    ~MyProcessor() { this->deactivate(); }

    APF_PROCESS(MyProcessor, MimoProcessorBase)
    {
      // input/output lists are processed automatically before/after this:
      _process_list(_intermediate_list);
    }

  private:
    rtlist_t _intermediate_list;
};

int main()
{
  // For now, this does nothing, we just want it to compile ...
}
