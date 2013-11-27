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

// Tests for Combine*.

#include "apf/combine_channels.h"

#include "catch/catch.hpp"

#include <vector>

using Item = std::vector<int>;

using L = std::vector<Item>;

struct SelectChange
{
  apf::CombineChannelsResult::type select(const Item&)
  {
    return apf::CombineChannelsResult::change;
  }

  void update() { /* ... */ }
};

class Crossfade
{
  public:
    using vi = std::vector<int>;

    Crossfade(size_t block_size)
      : _size(block_size)
      , _fade_out(_size, 2)
      , _fade_in(_size, 3)
    {}

    vi::const_iterator fade_out_begin() const { return _fade_out.begin(); }
    vi::const_iterator fade_in_begin() const { return _fade_in.begin(); }
    size_t size() const { return _size; }

  private:
    const size_t _size;

    vi _fade_out;
    vi _fade_in;
};

TEST_CASE("CombineChannels*", "")
{

int a[] = { 1, 2, 3, 4, 5, 6 };

L source;
source.push_back(Item(a, a+3));
source.push_back(Item(a+3, a+6));

Crossfade crossfade(3);

Item target(3, 0);

SECTION("CombineChannelsCopy", "")
{
  apf::CombineChannelsCopy<L, Item> c(source, target);

  // TODO: checks
}

SECTION("CombineChannels", "")
{
  apf::CombineChannels<L, Item> c(source, target);

  // TODO: checks
}

SECTION("CombineChannelsInterpolation", "")
{
  apf::CombineChannelsInterpolation<L, Item> c(source, target);

  // TODO: checks
}

SECTION("CombineChannelsCrossfadeCopy", "")
{
  apf::CombineChannelsCrossfadeCopy<L, Item, Crossfade >
    c(source, target, crossfade);

  c.process(SelectChange());

  // TODO: use CHECK_RANGE() from test_convolver.cpp

  CHECK(target[0] == 25);
  CHECK(target[1] == 35);
  CHECK(target[2] == 45);

  // TODO: more checks?
}

SECTION("CombineChannelsCrossfade", "")
{
  apf::CombineChannelsCrossfade<L, Item, Crossfade >
    c(source, target, crossfade);

  // TODO: checks
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
