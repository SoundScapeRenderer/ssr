// Tests for BiQuad and Cascade.

// see also ../performance_tests/biquad_*.cpp

#include "apf/biquad.h"

#include "catch/catch.hpp"

TEST_CASE("BiQuad", "Test BiQuad")
{

// TODO: more tests!

SECTION("basic", "only instantiations and very basic stuff")
{
  auto a = apf::BiQuad<double>();
  auto b = apf::BiQuad<float>();
  (void)b;

  auto c = apf::SosCoefficients<double>(0.1, 0.1, 0.1, 0.1, 0.1);
  a = c;

  CHECK(a.b0 == 0.1);

  auto d = apf::Cascade<apf::BiQuad<double>>(25);
  auto e = apf::Cascade<apf::BiQuad<float>>(25);
}

} // TEST_CASE
