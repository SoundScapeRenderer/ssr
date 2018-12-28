// Some macros which can be used for all iterator tests.

// The iterator requirements are based on
// http://www.cplusplus.com/reference/std/iterator/RandomAccessIterator/

// NOTE: base() should be "const"
// TODO: how to forbid non-const version?
#define ITERATOR_TEST_SECTION_BASE(iterator_type, base_type) \
SECTION("base", "Test base() member function") { \
  base_type ptr; \
  const iterator_type iter(&ptr); \
  CHECK(iter.base() == &ptr); }

// no CHECKs here, but at least it has to compile:
#define ITERATOR_TEST_SECTION_DEFAULT_CTOR(iterator_type) \
SECTION("default ctor", "X b; X();") { \
  iterator_type iter1; \
  iter1 = iterator_type(); }

#define ITERATOR_TEST_SECTION_COPY_ASSIGNMENT(iterator_type, base_type) \
SECTION("copy ctor, assignment", "X b(a); b=a;") { \
  base_type ptr; \
  iterator_type iter1(&ptr); \
  iterator_type iter2(iter1); \
  iterator_type iter3 = iter1; \
  iterator_type iter4; \
  iter4 = iter1; \
  CHECK(iter1.base() == &ptr); \
  CHECK(iter2.base() == &ptr); \
  CHECK(iter3.base() == &ptr); \
  CHECK(iter4.base() == &ptr); }

#define ITERATOR_TEST_SECTION_DEREFERENCE(iterator_type, base_type, value) \
SECTION("dereference", "*a; a->m;") { \
  base_type n = value; \
  iterator_type iter1(&n); \
  CHECK(*iter1 == value); \
  CHECK(iter1.operator->() == &n); \
  base_type o = base_type(); \
  iterator_type iter2(&o); \
  *iter2 = value; \
  CHECK(o == value); }

#define ITERATOR_TEST_SECTION_OFFSET_DEREFERENCE(iterator_type, base_type, \
    value0, value1) \
SECTION("offset dereference", "a[n]") { \
  base_type n[] = { value0, value1 }; \
  iterator_type iter1(n); \
  CHECK(iter1.base() == &n[0]); \
  CHECK(iter1[0] == value0); \
  CHECK(iter1[1] == value1); \
  iter1[1] = value0; \
  CHECK(n[1] == value0); }

#define ITERATOR_TEST_SECTION_EQUALITY(iterator_type, base_type) \
SECTION("equality/inequality", "a == b; a != b") { \
  base_type ptr1; \
  iterator_type iter1(&ptr1); \
  iterator_type iter2(&ptr1); \
  CHECK(iter1 == iter2); \
  CHECK_FALSE(iter1 != iter2); \
  base_type ptr2; \
  iterator_type iter3(&ptr2); \
  CHECK(iter1 != iter3); \
  CHECK_FALSE(iter1 == iter3); }

#define ITERATOR_TEST_SECTION_INCREMENT(iterator_type, base_type) \
SECTION("increment", "++a; a++") { \
  base_type n[3]; \
  iterator_type iter1(n); \
  iterator_type iter2; \
  CHECK(iter1.base() == &n[0]); \
  iter2 = ++iter1; \
  CHECK(iter1.base() == &n[1]); \
  CHECK(iter2.base() == &n[1]); \
  iter2 = iter1++; \
  CHECK(iter1.base() == &n[2]); \
  CHECK(iter2.base() == &n[1]); }

#define ITERATOR_TEST_SECTION_DECREMENT(iterator_type, base_type) \
SECTION("decrement", "--a; a--") { \
  base_type n[3]; \
  iterator_type iter1(&n[2]); \
  iterator_type iter2; \
  CHECK(iter1.base() == &n[2]); \
  iter2 = --iter1; \
  CHECK(iter1.base() == &n[1]); \
  CHECK(iter2.base() == &n[1]); \
  iter2 = iter1--; \
  CHECK(iter1.base() == &n[0]); \
  CHECK(iter2.base() == &n[1]); }

#define ITERATOR_TEST_SECTION_PLUS_MINUS(iterator_type, base_type) \
SECTION("plus/minus", "a + n; n + a; a - n; a - b; a += n; a -= n") { \
  base_type n[3]; \
  iterator_type iter1(n); \
  iterator_type iter2; \
  CHECK(iter1.base() == &n[0]); \
  iter2 = iter1 + 2; \
  CHECK(iter2.base() == &n[2]); \
  iter2 = 2 + iter1; \
  CHECK(iter2.base() == &n[2]); \
  iter1 += 2; \
  CHECK(iter1.base() == &n[2]); \
  iter2 = iter1 - 2; \
  CHECK(iter2.base() == &n[0]); \
  CHECK((iter1 - iter2) == 2); \
  iter1 -= 2; \
  CHECK(iter1.base() == &n[0]); }

#define ITERATOR_TEST_SECTION_LESS(iterator_type, base_type) \
SECTION("less et al.", "a < b; a > b; a <= b; a >= b") { \
  base_type n[2]; \
  iterator_type iter1(&n[0]); \
  iterator_type iter2(&n[1]); \
  iterator_type iter3(&n[1]); \
  CHECK(iter1 < iter2); \
  CHECK_FALSE(iter2 < iter3); \
  CHECK_FALSE(iter2 < iter1); \
  CHECK(iter2 > iter1); \
  CHECK_FALSE(iter2 > iter3); \
  CHECK_FALSE(iter1 > iter2); \
  CHECK(iter1 <= iter2); \
  CHECK(iter2 <= iter3); \
  CHECK_FALSE(iter2 <= iter1); \
  CHECK(iter2 >= iter1); \
  CHECK(iter2 >= iter3); \
  CHECK_FALSE(iter1 >= iter2); }
