// See also test_*_iterator.cpp

#include "apf/iterator.h"

#include "catch/catch.hpp"

TEST_CASE("iterators/has_begin_and_end", "Test has_begin_and_end")
{

int a[] = { 1, 2, 3 };

SECTION("default constructor", "")
{
  apf::has_begin_and_end<int*> range;
  CHECK(range.begin() == range.end());
}

SECTION("constructor with begin and end", "... and assignment op")
{
  auto range = apf::has_begin_and_end<int*>(a, a+3);
  CHECK(range.begin() == a);
  CHECK(range.end() == a + 3);
  apf::has_begin_and_end<int*> range2;
  range2 = range;
  CHECK(range2.begin() == a);
  CHECK(range2.end() == a + 3);
}

SECTION("constructor with begin and length", "... and copy ctor")
{
  auto range = apf::has_begin_and_end<int*>(a, 3);
  CHECK(range.begin() == a);
  CHECK(range.end() == a + 3);
  auto range2 = apf::has_begin_and_end<int*>(range);
  CHECK(range2.begin() == a);
  CHECK(range2.end() == a + 3);
}

SECTION("subscript operator", "")
{
  auto range = apf::has_begin_and_end<int*>(a, 3);
  CHECK(range[1] == 2);
  range[1] = 42;
  CHECK(range[1] == 42);
}

SECTION("const subscript operator", "")
{
  const int* b = a;
  auto range = apf::has_begin_and_end<const int*>(b, 3);
  CHECK(range[1] == 2);
  //range[1] = 42;  // doesn't work (as expected)
}

} // TEST_CASE

#include <list>

TEST_CASE("bidirectional iterator", "")
{

std::list<int> l;
l.push_back(1);
l.push_back(2);
l.push_back(3);

SECTION("bidirectional iterator", "")
{
  auto range
    = apf::has_begin_and_end<std::list<int>::iterator>(l.begin(), l.end());
  CHECK(range.begin() == l.begin());
  CHECK(range.end() == l.end());
  CHECK(*range.begin() == 1);
  //CHECK(range[1] == 2);  // doesn't work (as expected)
}

} // TEST_CASE
