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

// Tests for accumulating_iterator.

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
