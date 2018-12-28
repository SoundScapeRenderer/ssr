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

  apf::NonCausalBlockDelayLine<int>::difference_type correct;
  CHECK(d.delay_is_valid(-1, correct));
  CHECK(correct == -1);
  CHECK_FALSE(d.delay_is_valid(-2, correct));
  CHECK(correct == -1);
  CHECK(d.delay_is_valid(5, correct));
  CHECK(correct == 5);
  CHECK_FALSE(d.delay_is_valid(6, correct));
  CHECK(correct == 5);

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
