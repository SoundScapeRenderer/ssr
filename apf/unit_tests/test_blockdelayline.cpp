/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
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

// Tests for delay line.

#include "apf/blockdelayline.h"

#include "catch/catch.hpp"

#define CHECK_RANGE(left, right, range) \
  for (int i = 0; i < range; ++i) { \
    INFO("i = " << i); \
    CHECK((left)[i] == (right)[i]); }

TEST_CASE("block delay line", "Test the delay line")
{

int src[] = { 1, 2, 3, 4, 5, 6 };
int target[6] = { 0 };

SECTION("blocksize=1, max_delay=0", "")
{
  apf::BlockDelayLine<int> d(1, 0);
  d.write_block(src);
  CHECK(d.read_block(target, 0));
  CHECK(*target == 1);
  CHECK_FALSE(d.read_block(target, 1));
  d.write_block(src+1);
  CHECK(d.read_block(target, 0));
  CHECK(*target == 2);
}

SECTION("blocksize=1, max_delay=1", "")
{
  apf::BlockDelayLine<int> d(1, 1);
  d.write_block(src);
  d.write_block(src+1);
  CHECK(d.read_block(target, 0));
  CHECK(*target == 2);
  CHECK(d.read_block(target, 1));
  CHECK(*target == 1);
  d.write_block(src+2);
  CHECK(d.read_block(target, 0));
  CHECK(*target == 3);
}

SECTION("blocksize=3, max_delay=5", "")
{
  int empty[3] = { 0 };
  apf::BlockDelayLine<int> d(3, 5);
  d.write_block(src);
  d.write_block(src+3);
  d.write_block(empty);
  d.write_block(empty);
  CHECK(d.read_block(target, 4));
  int expected[3] = { 6, 0, 0 };
  CHECK_RANGE(target, expected, 3);

  CHECK(d.read_block(target, 4, 2));

  expected[0] = 12;
  CHECK_RANGE(target, expected, 3);
}

SECTION("non-causal delay line", "")
{
  int empty[3] = { 0 };
  apf::NonCausalBlockDelayLine<int> d(3, 5, 1);
  d.write_block(src);
  d.write_block(src+3);
  d.write_block(empty);
  d.write_block(empty);

  CHECK(d.delay_is_valid(-1));
  CHECK_FALSE(d.delay_is_valid(-2));
  CHECK(d.delay_is_valid(5));
  CHECK_FALSE(d.delay_is_valid(6));

  CHECK(d.read_block(target, 3));
  int expected[3] = { 6, 0, 0 };
  CHECK_RANGE(target, expected, 3);

  CHECK(d.read_block(target, 3, 2));

  expected[0] = 12;
  CHECK_RANGE(target, expected, 3);

  d.write_block(src);
  CHECK(d.read_block(target, -1));
  CHECK_RANGE(target, src, 3);
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
