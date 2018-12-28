#include "apf/iterator.h"  // for index_iterator

#include "catch/catch.hpp"

using ii = apf::index_iterator<int>;

TEST_CASE("iterators/index_iterator", "Test all functions of index_iterator")
{

// check if default constructor compiles
ii it;
it = ii();

SECTION("copy ctor, assignment", "X b(a); b=a;")
{
  auto iter1 = ii(42);
  auto iter2 = ii(iter1);
  ii iter3;
  iter3 = iter1;

  CHECK(*iter2 == 42);
  CHECK(*iter3 == 42);
}

SECTION("comparisons", "a == b; a != b, a < b, ...")
{
  auto iter1 = ii(4);
  auto iter2 = ii(4);
  auto iter3 = ii(5);

  CHECK(iter1 == iter2);
  CHECK(iter2 != iter3);
  CHECK_FALSE(iter1 != iter2);
  CHECK_FALSE(iter2 == iter3);

  CHECK(iter1 < iter3);
  CHECK_FALSE(iter1 > iter3);
  CHECK(iter3 > iter1);
  CHECK_FALSE(iter3 < iter1);

  CHECK(iter1 <= iter2);
  CHECK(iter1 <= iter3);
  CHECK(iter2 <= iter1);

  CHECK(iter3 >= iter1);
  CHECK(iter2 >= iter1);
}

SECTION("dereference", "*a; a[]")
{
  auto iter = ii(4);

  CHECK(*iter == 4);

  CHECK(iter[4] == 8);

  // NOTE: operator->() is purposely not implemented!
}

SECTION("increment, decrement", "++a; a++; *a++; --a; a--; *a--")
{
  auto iter1 = ii(0);
  ii iter2;

  CHECK(*iter1++ == 0);

  iter2 = iter1++;
  CHECK(*iter1 == 2);
  CHECK(*iter2 == 1);

  iter2 = ++iter1;
  CHECK(*iter1 == 3);
  CHECK(*iter2 == 3);

  CHECK(*iter1-- == 3);

  iter2 = iter1--;
  CHECK(*iter1 == 1);
  CHECK(*iter2 == 2);

  iter2 = --iter1;
  CHECK(*iter1 == 0);
  CHECK(*iter2 == 0);
}

SECTION("plus, minus", "a + n; a += n; ...")
{
  ii iter1;
  ii iter2;

  CHECK(*(iter1 + 5) == 5);
  CHECK(*(iter1 - 5) == -5);
  CHECK(*(5 + iter1) == 5);

  iter2 = iter1 += 3;
  CHECK(*iter1 == 3);
  CHECK(*iter2 == 3);
  iter2 = iter1 -= 1;
  CHECK(*iter1 == 2);
  CHECK(*iter2 == 2);

  iter2 += 5;
  CHECK((iter2 - iter1) == 5);
  CHECK((iter1 - iter2) == -5);
}

SECTION("test make_index_iterator", "namespace-level helper function")
{
  CHECK(*apf::make_index_iterator(5) == 5);
}

SECTION("unsigned", "see what happens with unsigned types")
{
  apf::index_iterator<unsigned int> iter1;
  apf::index_iterator<unsigned int> iter2(2);

  //CHECK((iter1 - iter2) == -2);

  //--iter1;
  //CHECK(*iter1 == -1);

  // This is dangerous (because errors may remain unnoticed)!
  // Stay away from unsigned types!
}

} // TEST_CASE
