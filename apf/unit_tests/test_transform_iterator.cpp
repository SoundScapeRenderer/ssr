#include "apf/iterator.h"  // for transform_iterator
#include "iterator_test_macros.h"

#include "catch/catch.hpp"

// Requirements for input iterators:
// http://www.cplusplus.com/reference/std/iterator/InputIterator/

struct three_halves
{
  float operator()(int in) { return static_cast<float>(in) * 1.5f; }
};

using fii = apf::transform_iterator<int*, three_halves>;

TEST_CASE("iterators/transform_iterator"
    , "Test all functions of transform_iterator")
{

ITERATOR_TEST_SECTION_BASE(fii, int)
ITERATOR_TEST_SECTION_DEFAULT_CTOR(fii)
ITERATOR_TEST_SECTION_COPY_ASSIGNMENT(fii, int)
ITERATOR_TEST_SECTION_EQUALITY(fii, int)
ITERATOR_TEST_SECTION_INCREMENT(fii, int)
ITERATOR_TEST_SECTION_DECREMENT(fii, int)
ITERATOR_TEST_SECTION_PLUS_MINUS(fii, int)
ITERATOR_TEST_SECTION_LESS(fii, int)

int array[] = { 1, 2, 3 };

SECTION("dereference", "*a; *a++; a[]")
{
  auto iter = fii(array);

  CHECK(*iter == 1.5f);
  CHECK(iter.base() == &array[0]);
  CHECK(*iter++ == 1.5f);
  CHECK(iter.base() == &array[1]);

  CHECK(*iter-- == 3.0f);
  CHECK(iter.base() == &array[0]);

  CHECK(iter[2] == 4.5f);

  // NOTE: operator->() doesn't make too much sense here, it's not working.
  // See below for an example where it does work.
}

SECTION("test make_transform_iterator", "namespace-level helper function")
{
  CHECK(*apf::make_transform_iterator(array, three_halves()) == 1.5f);
}

} // TEST_CASE

struct mystruct
{
  struct inner
  {
    inner() : num(42) {}
    int num;
  } innerobj;
};

struct special_function1
{
  int operator()(mystruct s)
  {
    return s.innerobj.num;
  }
};

struct special_function2
{
  // Note: mystruct&& doesn't work
  mystruct::inner& operator()(mystruct& s)
  {
    return s.innerobj;
  }
};

struct special_function3
{
  const mystruct::inner& operator()(const mystruct& s) const
  {
    return s.innerobj;
  }
};

struct special_function4
{
  template<typename T>
  int operator()(T&& s)
  {
    return s.innerobj.num;
  }
};

TEST_CASE("iterators/transform_iterator with strange functions", "")
{

mystruct x;

SECTION("special_function1", "")
{
  apf::transform_iterator<mystruct*, special_function1>
    it(&x, special_function1());
  CHECK(*it == 42);
}

SECTION("special_function2", "")
{
  apf::transform_iterator<mystruct*, special_function2>
    it(&x, special_function2());
  CHECK(&*it == &x.innerobj);
  CHECK(it->num == 42);
}

SECTION("special_function3", "")
{
  apf::transform_iterator<mystruct*, special_function3>
    it(&x, special_function3());
  CHECK(&*it == &x.innerobj);
  CHECK(it->num == 42);
}

SECTION("special_function4", "")
{
  apf::transform_iterator<mystruct*, special_function4>
    it(&x, special_function4());
  CHECK(*it == 42);
}

SECTION("lambda functions", "")
{
  apf::transform_iterator<mystruct*, special_function4>
    it(&x, special_function4());

  CHECK(42 == *apf::make_transform_iterator(&x
        , [] (mystruct s) { return s.innerobj.num; }));

  CHECK(42 == *apf::make_transform_iterator(&x
        , [] (mystruct& s) { return s.innerobj.num; }));

  CHECK(42 == *apf::make_transform_iterator(&x
        , [] (const mystruct& s) { return s.innerobj.num; }));
}

} // TEST_CASE

TEST_CASE("transform_proxy", "Test transform_proxy")
{

using vi = std::vector<int>;
vi input;
input.push_back(1);
input.push_back(2);
input.push_back(3);

SECTION("test transform_proxy", "")
{
  auto p = apf::transform_proxy<three_halves, vi>(input);

  CHECK(p.size() == 3);
  CHECK((*p.begin()) == 1.5);
  CHECK((p.begin() + 3) == p.end());
}

SECTION("test tranform_proxy_const", "")
{
  auto p = apf::transform_proxy_const<three_halves, vi>(input);

  CHECK(p.size() == 3);
  CHECK((*p.begin()) == 1.5);
  CHECK((p.begin() + 3) == p.end());
}

} // TEST_CASE
