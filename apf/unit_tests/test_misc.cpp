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

// Tests for misc.h.

#include "apf/misc.h"

#include <cassert>
#include "catch/catch.hpp"

template<typename Derived>
struct B : apf::CRTP<Derived>
{
  int value() { return this->derived().i; }
};

struct D : B<D>
{
  int i = 42;
};

TEST_CASE("CRTP", "")
{
auto d = D();
CHECK(d.value() == 42);
}

TEST_CASE("BlockParameter", "")
{

SECTION("default ctor", "")
{
  auto bp = apf::BlockParameter<int>();
  CHECK(0 == bp.get());
  CHECK(0 == bp.old());
}

SECTION("int", "")
{
  auto bp = apf::BlockParameter<int>(111);
  CHECK(111 == bp.get());
  CHECK(111 == bp.old());
  CHECK(111 == bp.both());
  CHECK(bp.both() == 111);
  CHECK_FALSE(bp.changed());

  // TODO: once Catch supports checking for asserts, enable this:
  //assert(bp.exactly_one_assignment());

  bp = 222;
  CHECK(222 == bp.get());
  CHECK(111 == bp.old());
  CHECK(bp.changed());

  CHECK_FALSE(111 == bp.both());
  CHECK_FALSE(bp.both() == 111);
  CHECK(bp.both() != 0);
  CHECK(0 != bp.both());
  CHECK(bp.both() > 0);
  CHECK(bp.both() >= 0);
  CHECK_FALSE(0 > bp.both());
  CHECK_FALSE(0 >= bp.both());
  CHECK_FALSE(bp.both() > 333);
  CHECK_FALSE(bp.both() >= 333);
  CHECK(333 > bp.both());
  CHECK(333 >= bp.both());
  CHECK(bp.both() < 333);
  CHECK(bp.both() <= 333);
  CHECK_FALSE(333 < bp.both());
  CHECK_FALSE(333 <= bp.both());
  CHECK_FALSE(bp.both() < 0);
  CHECK_FALSE(bp.both() <= 0);
  CHECK(0 < bp.both());
  CHECK(0 <= bp.both());

  assert(bp.exactly_one_assignment());

  bp = 333;
  CHECK(333 == bp.get());
  CHECK(222 == bp.old());
  CHECK(bp.changed());

  bp -= 111;
  CHECK_FALSE(bp.changed());

  ++bp;
  CHECK(222 == bp.old());
  CHECK(223 == bp.get());

  assert(bp.exactly_one_assignment());
}

SECTION("conversion operator", "")
{
  auto bp = apf::BlockParameter<int>(42);
  int i = 0;
  CHECK(0 == i);
  i = bp;
  CHECK(42 == i);
  CHECK((i - bp) == 0);
}

SECTION("conversion operator from const object", "")
{
  const auto bp = apf::BlockParameter<int>(42);
  int i = 0;
  CHECK(0 == i);
  i = bp;
  CHECK(42 == i);
  CHECK((i - bp) == 0);
}

struct NonCopyable
{
  NonCopyable(int) {};  // hypothetical constructor
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable(NonCopyable&&) = default;
  NonCopyable& operator=(NonCopyable&& other) = default;
};

SECTION("non-copyable T", "")
{
  // These are just compile-time checks:

  auto bp = apf::BlockParameter<NonCopyable>{42};
  bp = NonCopyable(43);
};

// TODO: move CountCtors in a separate file?

struct CountCtors
{
  CountCtors() { ++ default_constructor; }
  CountCtors(const CountCtors&) { ++ copy_constructor; }
  CountCtors(CountCtors&&) { ++move_constructor; }

  CountCtors& operator=(const CountCtors&) { ++copy_assignment; return *this; };
  CountCtors& operator=(CountCtors&&) { ++move_assignment; return *this; };

  int default_constructor = 0;
  int copy_constructor = 0;
  int move_constructor = 0;

  int copy_assignment = 0;
  int move_assignment = 0;
};

SECTION("check if move ctor and move assignment is used", "")
{
  auto bp = apf::BlockParameter<CountCtors>{CountCtors()};
  CHECK(bp.get().copy_constructor == 1);
  CHECK(bp.old().move_constructor == 1);

  bp = CountCtors();

  CHECK(bp.get().move_assignment == 1);
  CHECK(bp.get().copy_assignment == 0);
  CHECK(bp.old().move_assignment == 1);
  CHECK(bp.old().copy_assignment == 0);
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
