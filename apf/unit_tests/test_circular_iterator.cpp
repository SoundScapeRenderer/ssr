#include "apf/iterator.h"  // for circular_iterator
#include "iterator_test_macros.h"
#include "catch/catch.hpp"

using ci = apf::circular_iterator<int*>;

TEST_CASE("iterators/circular_iterator/1"
    , "Test all straightforward functions of circular_iterator")
{

ITERATOR_TEST_SECTION_BASE(ci, int)
ITERATOR_TEST_SECTION_DEFAULT_CTOR(ci)
ITERATOR_TEST_SECTION_COPY_ASSIGNMENT(ci, int)
ITERATOR_TEST_SECTION_DEREFERENCE(ci, int, 5)
ITERATOR_TEST_SECTION_EQUALITY(ci, int)

// NOTE: comparison operators (except == and !=) don't make sense!

} // TEST_CASE

TEST_CASE("iterators/circular_iterator/2"
    , "Test all non-trivial functions of circular_iterator")
{

int a[] = { 0, 1, 2 };
ci iter1(&a[0], &a[3]);
ci iter2(&a[0], &a[3], &a[1]);
ci iter3(&a[0]);  // "useless" constructor
ci iter4(&a[0], &a[3], &a[3]);  // wrapping, current == end -> current = begin

SECTION("special constructors", "")
{
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[1]);
  CHECK(iter3.base() == &a[0]);
  CHECK(iter4.base() == &a[0]);

  // expected error:
  //ci iter5(&a[0], &a[0]);  // assertion, begin == end
}

SECTION("increment", "++a; a++")
{
  CHECK(iter1.base() == &a[0]);
  iter2 = ++iter1;
  CHECK(iter1.base() == &a[1]);
  CHECK(iter2.base() == &a[1]);
  iter2 = ++iter1;
  CHECK(iter1.base() == &a[2]);
  CHECK(iter2.base() == &a[2]);
  iter2 = ++iter1;
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[0]);
  iter2 = ++iter1;
  CHECK(iter1.base() == &a[1]);
  CHECK(iter2.base() == &a[1]);
  iter2 = ++iter1;
  CHECK(iter1.base() == &a[2]);
  CHECK(iter2.base() == &a[2]);
  iter2 = ++iter1;
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[0]);

  iter2 = iter1++;
  CHECK(iter1.base() == &a[1]);
  CHECK(iter2.base() == &a[0]);
  iter2 = iter1++;
  CHECK(iter1.base() == &a[2]);
  CHECK(iter2.base() == &a[1]);
  iter2 = iter1++;
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[2]);
  iter2 = iter1++;
  CHECK(iter1.base() == &a[1]);
  CHECK(iter2.base() == &a[0]);
  iter2 = iter1++;
  CHECK(iter1.base() == &a[2]);
  CHECK(iter2.base() == &a[1]);
  iter2 = iter1++;
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[2]);
}

SECTION("decrement", "--a; a--")
{
  CHECK(iter1.base() == &a[0]);
  iter2 = --iter1;
  CHECK(iter1.base() == &a[2]);
  CHECK(iter2.base() == &a[2]);
  iter2 = --iter1;
  CHECK(iter1.base() == &a[1]);
  CHECK(iter2.base() == &a[1]);
  iter2 = --iter1;
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[0]);
  iter2 = --iter1;
  CHECK(iter1.base() == &a[2]);
  CHECK(iter2.base() == &a[2]);
  iter2 = --iter1;
  CHECK(iter1.base() == &a[1]);
  CHECK(iter2.base() == &a[1]);
  iter2 = --iter1;
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[0]);

  iter2 = iter1--;
  CHECK(iter1.base() == &a[2]);
  CHECK(iter2.base() == &a[0]);
  iter2 = iter1--;
  CHECK(iter1.base() == &a[1]);
  CHECK(iter2.base() == &a[2]);
  iter2 = iter1--;
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[1]);
  iter2 = iter1--;
  CHECK(iter1.base() == &a[2]);
  CHECK(iter2.base() == &a[0]);
  iter2 = iter1--;
  CHECK(iter1.base() == &a[1]);
  CHECK(iter2.base() == &a[2]);
  iter2 = iter1--;
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[1]);
}

SECTION("plus/minus", "a + n; n + a; a - n; a - b; a += n; a -= n")
{
  CHECK((iter1 + -9).base() == &a[0]);
  CHECK((iter1 + -8).base() == &a[1]);
  CHECK((iter1 + -7).base() == &a[2]);
  CHECK((iter1 + -6).base() == &a[0]);
  CHECK((iter1 + -5).base() == &a[1]);
  CHECK((iter1 + -4).base() == &a[2]);
  CHECK((iter1 + -3).base() == &a[0]);
  CHECK((iter1 + -2).base() == &a[1]);
  CHECK((iter1 + -1).base() == &a[2]);
  CHECK((iter1 +  0).base() == &a[0]);
  CHECK((iter1 +  1).base() == &a[1]);
  CHECK((iter1 +  2).base() == &a[2]);
  CHECK((iter1 +  3).base() == &a[0]);
  CHECK((iter1 +  4).base() == &a[1]);
  CHECK((iter1 +  5).base() == &a[2]);
  CHECK((iter1 +  6).base() == &a[0]);
  CHECK((iter1 +  7).base() == &a[1]);
  CHECK((iter1 +  8).base() == &a[2]);
  CHECK((iter1 +  9).base() == &a[0]);

  CHECK((iter1 -  9).base() == &a[0]);
  CHECK((iter1 -  8).base() == &a[1]);
  CHECK((iter1 -  7).base() == &a[2]);
  CHECK((iter1 -  6).base() == &a[0]);
  CHECK((iter1 -  5).base() == &a[1]);
  CHECK((iter1 -  4).base() == &a[2]);
  CHECK((iter1 -  3).base() == &a[0]);
  CHECK((iter1 -  2).base() == &a[1]);
  CHECK((iter1 -  1).base() == &a[2]);
  CHECK((iter1 -  0).base() == &a[0]);
  CHECK((iter1 - -1).base() == &a[1]);
  CHECK((iter1 - -2).base() == &a[2]);
  CHECK((iter1 - -3).base() == &a[0]);
  CHECK((iter1 - -4).base() == &a[1]);
  CHECK((iter1 - -5).base() == &a[2]);
  CHECK((iter1 - -6).base() == &a[0]);
  CHECK((iter1 - -7).base() == &a[1]);
  CHECK((iter1 - -8).base() == &a[2]);
  CHECK((iter1 - -9).base() == &a[0]);

  CHECK((0 + iter1).base() == &a[0]);
  CHECK((1 + iter1).base() == &a[1]);
  CHECK((2 + iter1).base() == &a[2]);
  CHECK((3 + iter1).base() == &a[0]);
  CHECK((4 + iter1).base() == &a[1]);
  CHECK((5 + iter1).base() == &a[2]);
  CHECK((6 + iter1).base() == &a[0]);
  CHECK((7 + iter1).base() == &a[1]);
  CHECK((8 + iter1).base() == &a[2]);
  CHECK((9 + iter1).base() == &a[0]);

  CHECK((ci(&a[0], &a[3], &a[0]) - ci(&a[0], &a[3])) == 0);
  CHECK((ci(&a[0], &a[3], &a[1]) - ci(&a[0], &a[3])) == 1);
  CHECK((ci(&a[0], &a[3], &a[2]) - ci(&a[0], &a[3])) == 2);

  // all differences are positive!
  CHECK((ci(&a[0], &a[3]) - ci(&a[0], &a[3], &a[0])) == 0);
  CHECK((ci(&a[0], &a[3]) - ci(&a[0], &a[3], &a[1])) == 2);
  CHECK((ci(&a[0], &a[3]) - ci(&a[0], &a[3], &a[2])) == 1);

  iter2 = (iter1 += 0);
  CHECK(iter1.base() == &a[0]);
  CHECK(iter2.base() == &a[0]);
  iter1 += 2;
  CHECK(iter1.base() == &a[2]);
  iter1 += 2;
  CHECK(iter1.base() == &a[1]);
  iter1 -= 2;
  CHECK(iter1.base() == &a[2]);
  iter1 -= 2;
  CHECK(iter1.base() == &a[0]);

  CHECK((iter3 + 666).base() == &a[0]);
}

SECTION("offset dereference", "a[n]")
{
  CHECK(iter1[-5] == 1);
  CHECK(iter1[-4] == 2);
  CHECK(iter1[-3] == 0);
  CHECK(iter1[-2] == 1);
  CHECK(iter1[-1] == 2);
  CHECK(iter1[ 0] == 0);
  CHECK(iter1[ 1] == 1);
  CHECK(iter1[ 2] == 2);
  CHECK(iter1[ 3] == 0);
  CHECK(iter1[ 4] == 1);
  CHECK(iter1[ 5] == 2);

  // can we also assign?
  iter1[-3] = 42;
  CHECK(a[0] == 42);
}

SECTION("make_circular_iterator", "")
{
  iter1 = apf::make_circular_iterator(&a[0], &a[3]);
  CHECK(iter1.base() == &a[0]);
  iter1 = apf::make_circular_iterator(&a[0], &a[3], &a[2]);
  CHECK(iter1.base() == &a[2]);
  iter1 = apf::make_circular_iterator(&a[0], &a[3], &a[3]);
  CHECK(iter1.base() == &a[0]);
}

} // TEST_CASE

#include <list>

TEST_CASE("iterators/circular_iterator/3"
    , "Test if it also works with a bidirectional iterator")
{

  std::list<int> l = {0, 1, 2};
  apf::circular_iterator<std::list<int>::iterator> it(l.begin(), l.end());

  CHECK(*it == 0);
  --it;
  CHECK(*it == 2);
  it--;
  CHECK(*it == 1);
  ++it;
  CHECK(*it == 2);
  it++;
  CHECK(*it == 0);

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
