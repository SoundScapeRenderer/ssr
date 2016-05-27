// Tests for combinations of different iterators

#include "apf/iterator.h"  // for *_iterator

#include "catch/catch.hpp"

struct three_halves
{
  float operator()(int in) { return static_cast<float>(in) * 1.5f; }
};

using ii = apf::index_iterator<int>;
using fii = apf::transform_iterator<ii, three_halves>;
using si = apf::stride_iterator<ii>;
using fsi = apf::transform_iterator<si, three_halves>;

TEST_CASE("iterators/combinations", "Test combinations of iterators")
{

SECTION("index_iterator + transform_iterator", "")
{
  auto iter = fii(apf::make_index_iterator(2), three_halves());
  CHECK(*iter == 3.0f);
}

SECTION("index_iterator + stride_iterator + transform_iterator", "")
{
  auto iter = fsi(si(apf::make_index_iterator(2), 2), three_halves());
  CHECK(*iter == 3.0f);
  ++iter;
  CHECK(*iter == 6.0f);

  auto iter2 = fsi(si(apf::make_index_iterator(2), -2), three_halves());
  CHECK(*iter2 == 3.0f);
  ++iter2;
  CHECK(*iter2 == 0.0f);
}


} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
