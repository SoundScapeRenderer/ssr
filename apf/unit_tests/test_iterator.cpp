/******************************************************************************
 * Copyright © 2012-2013 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the Audio Processing Framework (APF).                 *
 *                                                                            *
 * The APF is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The APF is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 *                                 http://AudioProcessingFramework.github.com *
 ******************************************************************************/

// Tests iterators.
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

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
