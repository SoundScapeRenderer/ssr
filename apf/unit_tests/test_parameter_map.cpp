#include "apf/parameter_map.h"

#include "catch/catch.hpp"

TEST_CASE("parameter_map", "")
{

SECTION("constructors", "")
{
  apf::parameter_map pm1;  // default
  apf::parameter_map pm2{pm1};  // copy
  apf::parameter_map pm3{apf::parameter_map{}};  // move

  std::map<std::string, std::string> m1;
  apf::parameter_map pm4{m1};  // copy
  apf::parameter_map pm5{std::map<std::string, std::string>{}};  // move
}

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
  CHECK_THROWS_AS(params.get<int>("one"), std::invalid_argument&);
  val3 = params.get("one", 42); // default value 42 if conversion fails
  CHECK(val3 == 42);
  val4 = params.get("three", 3.0);
  CHECK(val4 == 3.1415);
  if (params.has_key("four"))
  {
    // this is not done because there is no key named "four":
    CHECK(false);
  }
  CHECK_THROWS_AS(params.get<std::string>("four"), std::out_of_range&);
  CHECK_THROWS_AS(params["four"], std::out_of_range&);
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
