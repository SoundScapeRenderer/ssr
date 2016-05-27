#include "apf/iterator.h"  // for dual_iterator

#include "catch/catch.hpp"

TEST_CASE("iterators/dual_iterator", "Test all functions of dual_iterator")
{

using di = apf::dual_iterator<int*, int*>;

int x[] = { 0, 0 };
int y[] = { 0, 0 };

SECTION("default ctor", "")
{
  di one;
  (void)one;  // avoid "unused variable" warning
  di();
}

SECTION("copy ctor, assignment", "X b(a); b=a;")
{
  auto iter1 = di(x, y);
  auto iter2 = di(iter1);
  auto iter3 = iter1;  // same as above
  (void)iter2;
  (void)iter3;
  di iter4;
  iter4 = iter1;

  // TODO: actually CHECK something?
}

SECTION("dereference/increment", "*a, *a++")
{
  auto iter = di(x, y);

  *iter++ = 1;

  CHECK(x[0] == 1);
  CHECK(y[0] == 1);

  *iter = 2;

  CHECK(x[1] == 2);
  CHECK(y[1] == 2);
}

SECTION("(un)equality", "... and pre/post-increment")
{
  auto iter1 = di(x, y);
  auto iter2 = di(x, y);

  ++iter1;

  CHECK_FALSE(iter1 == iter2);
  CHECK(iter1 != iter2);

  iter2++;

  CHECK(iter1 == iter2);
  CHECK_FALSE(iter1 != iter2);
}

SECTION("test make_dual_iterator", "... and assignment")
{
  auto iter = apf::make_dual_iterator(x, y);
  *iter = 1;
  CHECK(*x == 1);
  CHECK(*y == 1);
}

SECTION("dual_iterator assign from std::pair", "")
{
  int i = 5;
  double d = 6;
  auto iter = apf::make_dual_iterator(&i, &d);
  *iter = std::make_pair(2, 3.0);
  CHECK(i == 2);
  CHECK(d == 3.0);
}

SECTION("dual_iterator assign to std::pair", "")
{
  int i = 5;
  double d = 6;
  auto iter = apf::make_dual_iterator(&i, &d);
  // The pair types don't have to match exactly:
  std::pair<long int, long double> p = *iter;
  CHECK(p.first == 5);
  CHECK(p.second == 6.0);
}

SECTION("dereference ... and do stuff", "+=")
{
  auto iter = apf::make_dual_iterator(x, y);

  *iter += 5;
  CHECK(*x == 5);
  CHECK(*y == 5);

  *iter += std::make_pair(4, 2);
  CHECK(*x == 9);
  CHECK(*y == 7);
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
