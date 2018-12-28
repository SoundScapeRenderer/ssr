#include "apf/iterator.h"  // for cast_iterator
#include "iterator_test_macros.h"

#include "catch/catch.hpp"

struct dummy_type {};
using ci = apf::cast_iterator<dummy_type, int*>;

struct Base {};

struct Derived : Base
{
  Derived(int n) : _n(n) {}

  int _n;
};

// The iterator requirements are based on
// http://www.cplusplus.com/reference/std/iterator/RandomAccessIterator

TEST_CASE("iterators/cast_iterator", "Test all functions of cast_iterator")
{

// First, the straightforward functions:

ITERATOR_TEST_SECTION_DEFAULT_CTOR(ci)
ITERATOR_TEST_SECTION_BASE(ci, int)
ITERATOR_TEST_SECTION_COPY_ASSIGNMENT(ci, int)
ITERATOR_TEST_SECTION_EQUALITY(ci, int)
ITERATOR_TEST_SECTION_INCREMENT(ci, int)
ITERATOR_TEST_SECTION_DECREMENT(ci, int)
ITERATOR_TEST_SECTION_PLUS_MINUS(ci, int)
ITERATOR_TEST_SECTION_LESS(ci, int)

// now the "special" operators * and -> and []:

SECTION("dereference", "*a; a->m; a[n]")
{
  auto d1 = Derived(42);
  Base* pb = &d1;

  auto iter = apf::cast_iterator<Derived, Base**>(&pb);

  Derived d2 = *iter;
  Derived d3 = iter[0];

  CHECK(d2._n == 42);
  CHECK(iter->_n == 42);
  CHECK(d3._n == 42);

  iter->_n = 23;
  CHECK(d1._n == 23);

  (*iter)._n = 42;
  CHECK(d1._n == 42);

  iter[0]._n = 666;
  CHECK(d1._n == 666);
}

SECTION("dereference and increment", "*a++")
{
  auto d = Derived(42);
  Base* pb = &d;

  auto iter = apf::cast_iterator<Derived, Base**>(&pb);

  Derived d2 = *iter++;

  CHECK(d2._n == 42);

  CHECK(iter.base() == &pb+1);
}

SECTION("offset dereference", "a[n]")
{
  auto d = Derived(42);

  Base* b[] = { nullptr, nullptr, &d };

  auto iter = apf::cast_iterator<Derived, Base**>(b);

  Derived d2 = iter[2];
  CHECK(d2._n == 42);
}

SECTION("test make_cast_iterator", "namespace-level helper function")
{
  int n = 5;
  auto iter = apf::make_cast_iterator<dummy_type>(&n);
  CHECK(iter.base() == &n);
}

using vb = std::vector<Base*>;
vb b;
b.push_back(new Derived(1));
b.push_back(new Derived(2));
b.push_back(new Derived(3));

SECTION("test cast_proxy", "")
{
  auto p = apf::cast_proxy<Derived, vb>(b);

  CHECK(p.size() == 3);
  CHECK(p.begin()->_n == 1);
  CHECK((*p.begin())._n == 1);
  CHECK((p.begin() + 3) == p.end());

  p.begin()->_n = 42;
  CHECK(p.begin()->_n == 42);

  CHECK(apf::make_cast_proxy<Derived>(b).size() == 3);
}

SECTION("test cast_proxy_const", "")
{
  auto p = apf::cast_proxy_const<Derived, vb>(b);

  CHECK(p.size() == 3);
  CHECK(p.begin()->_n == 1);
  CHECK((*p.begin())._n == 1);
  CHECK((p.begin() + 3) == p.end());

  //p.begin()->_n = 42;  // compile-time error (as expected)

  CHECK(apf::make_cast_proxy_const<Derived>(b).size() == 3);
}

} // TEST_CASE
