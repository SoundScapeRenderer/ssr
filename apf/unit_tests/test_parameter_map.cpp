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

// Tests for parameter_map.h.

#include "apf/parameter_map.h"

#include "catch/catch.hpp"

TEST_CASE("parameter_map", "")
{

SECTION("stuff", "")
{
  apf::parameter_map params;
  params.set("one", "first value");
  CHECK(params["one"] == "first value");
  params.set("two", 2);
  CHECK(params["two"] == "2");
  params.set("three", 3.1415);
  CHECK(params["three"] == "3.1415");
  std::string val1;
  int val2, val3;
  double val4;
  val1 = params["one"];
  CHECK(val1 == "first value");
  val2 = params.get<int>("two");
  CHECK(val2 == 2);
  CHECK_THROWS_AS(params.get<int>("one"), std::invalid_argument);
  val3 = params.get("one", 42); // default value 42 if conversion fails
  CHECK(val3 == 42);
  val4 = params.get("three", 3.0);
  CHECK(val4 == 3.1415);
  if (params.has_key("four"))
  {
    // this is not done because there is no key named "four":
    CHECK(false);
  }
  CHECK_THROWS_AS(params.get<std::string>("four"), std::out_of_range);
  CHECK_THROWS_AS(params["four"], std::out_of_range);
}

SECTION("more stuff", "")
{
  apf::parameter_map params;
  params.set("id", "item42");
  std::string id1, id2, name1, name2;
  id1   = params.get("id"  , "no_id_available");
  CHECK(id1 == "item42");
  id2   = params.get("id"  , "item42");
  CHECK(id2 == "item42");
  name1 = params.get("name", "Default Name");
  CHECK(name1 == "Default Name");
  name2 = params.get("name", "");
  CHECK(name2 == "");
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
