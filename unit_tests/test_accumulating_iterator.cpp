#include "apf/iterator.h"  // for accumulating_iterator
#include "iterator_test_macros.h"

#include "catch/catch.hpp"

using ai = apf::accumulating_iterator<int*>;

TEST_CASE("iterators/accumulating_iterator"
    , "Test all functions of accumulating_iterator")
{

ITERATOR_TEST_SECTION_DEFAULT_CTOR(ai)
ITERATOR_TEST_SECTION_BASE(ai, int)
ITERATOR_TEST_SECTION_COPY_ASSIGNMENT(ai, int)
ITERATOR_TEST_SECTION_INCREMENT(ai, int)

SECTION("dereference/increment", "*a, *a++")
{
  int n = 5;
  auto iter = ai(&n);

  *iter = 4;

  CHECK(n == 9);

  *iter++ = 2;

  CHECK(n == 11);
  CHECK(iter.base() == &n+1);

  // NOTE: operator->() is purposely not implemented!
}

SECTION("test make_accumulating_iterator", "namespace-level helper function")
{
  int n = 5;
  auto iter = apf::make_accumulating_iterator(&n);
  CHECK(iter.base() == &n);
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
