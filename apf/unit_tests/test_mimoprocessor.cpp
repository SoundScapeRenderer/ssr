#include "apf/mimoprocessor.h"

#include "catch/catch.hpp"

#include "apf/pointer_policy.h"

struct DummyProcessor :
  public apf::MimoProcessor<DummyProcessor, apf::pointer_policy<float*>>
{
  DummyProcessor(const apf::parameter_map& p)
    : apf::MimoProcessor<DummyProcessor, apf::pointer_policy<float*>>(p)
  {}

  void process() {}
};

TEST_CASE("MimoProcessor", "Test MimoProcessor")
{

SECTION("compilation", "does it compile?")
{
  apf::parameter_map p;
  // some strange values:
  p.set("sample_rate", 1000);
  p.set("block_size", 33);
  DummyProcessor dummy(p);
}

// TODO: more tests!

} // TEST_CASE MimoProcessor
