// Define and test a trivial iterator.
// This is basically to show and test all iterator macros.

#include "apf/iterator.h"

#include "catch/catch.hpp"

template<typename I>
class trivial_iterator
{
  private:
    using self = trivial_iterator;

  public:
    using value_type = typename std::iterator_traits<I>::value_type;
    using pointer = typename std::iterator_traits<I>::pointer;
    using reference = typename std::iterator_traits<I>::reference;
    using difference_type = typename std::iterator_traits<I>::difference_type;
    using iterator_category
      = typename std::iterator_traits<I>::iterator_category;

    APF_ITERATOR_CONSTRUCTORS(trivial_iterator, I, _base_iterator)
    APF_ITERATOR_BASE(I, _base_iterator)

    APF_ITERATOR_RANDOMACCESS_EQUAL(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_DEREFERENCE(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_ARROW(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREINCREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_PREDECREMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_ADDITION_ASSIGNMENT(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_DIFFERENCE(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_LESS(_base_iterator)
    APF_ITERATOR_RANDOMACCESS_SUBSCRIPT
    APF_ITERATOR_RANDOMACCESS_UNEQUAL
    APF_ITERATOR_RANDOMACCESS_OTHER_COMPARISONS
    APF_ITERATOR_RANDOMACCESS_POSTINCREMENT
    APF_ITERATOR_RANDOMACCESS_POSTDECREMENT
    APF_ITERATOR_RANDOMACCESS_THE_REST

  private:
    I _base_iterator;
};

#include "iterator_test_macros.h"

#include <vector>

TEST_CASE("int*", "")
{

using iter_t = trivial_iterator<int*>;

ITERATOR_TEST_SECTION_BASE(iter_t, int)
ITERATOR_TEST_SECTION_DEFAULT_CTOR(iter_t)
ITERATOR_TEST_SECTION_COPY_ASSIGNMENT(iter_t, int)
ITERATOR_TEST_SECTION_DEREFERENCE(iter_t, int, 42)
ITERATOR_TEST_SECTION_OFFSET_DEREFERENCE(iter_t, int, 23, 42)
ITERATOR_TEST_SECTION_EQUALITY(iter_t, int)
ITERATOR_TEST_SECTION_INCREMENT(iter_t, int)
ITERATOR_TEST_SECTION_DECREMENT(iter_t, int)
ITERATOR_TEST_SECTION_PLUS_MINUS(iter_t, int)
ITERATOR_TEST_SECTION_LESS(iter_t, int)

}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
