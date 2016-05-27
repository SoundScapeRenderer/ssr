#include "apf/convolver.h"

#include "catch/catch.hpp"

#define CHECK_RANGE(left, right, range) \
  for (int i = 0; i < range; ++i) { \
    INFO("i = " << i); \
    CHECK((left)[i] == Approx((right)[i])); }

namespace c = apf::conv;

TEST_CASE("Convolver", "Test Convolver")
{

float test_signal[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f
  , 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.0f, 0.0f, 0.0f, 0.0f };
float zeros[20] = { 0.0f };

float filter_data[16] = { 0.0f };
filter_data[10] = 5.0f;
filter_data[11] = 4.0f;
filter_data[12] = 3.0f;

auto partitions = c::min_partitions(8, 16);
auto conv_input = c::Input(8, partitions);
auto conv_output = c::Output(conv_input);
auto filter = c::Filter(8, filter_data, filter_data + 16);

float* result;

SECTION("silence", "")
{
  conv_input.add_block(test_signal);
  result = conv_output.convolve();

  CHECK_RANGE(result, zeros, 8);

  conv_input.add_block(test_signal);
  result = conv_output.convolve();

  CHECK_RANGE(result, zeros, 8);
}

SECTION("impulse", "")
{
  float one = 1.0f;
  auto impulse = c::Filter(8, &one, (&one)+1);

  conv_input.add_block(test_signal);
  conv_output.set_filter(impulse);
  result = conv_output.convolve();

  CHECK_RANGE(result, test_signal, 8);

  conv_input.add_block(test_signal + 8);
  result = conv_output.convolve();

  CHECK_RANGE(result, test_signal + 8, 8);
}

SECTION("... and more", "")
{
  conv_output.set_filter(filter);

  float input[8] = { 0.0f };

  input[1] = 1.0f;
  conv_input.add_block(input);
  result = conv_output.convolve();

  CHECK_RANGE(result, zeros, 8);

  input[1] = 2.0f;
  conv_input.add_block(input);
  CHECK_FALSE(conv_output.queues_empty());
  conv_output.rotate_queues();
  result = conv_output.convolve();

  float expected[8] = { 0.0f };
  expected[3] = 5.0f;
  expected[4] = 4.0f;
  expected[5] = 3.0f;

  CHECK_RANGE(result, expected, 8);

  input[1] = 0.0f;
  conv_input.add_block(input);
  CHECK(conv_output.queues_empty());
  //conv_filter.rotate_queues();  // not necessary, because queues are empty
  result = conv_output.convolve();

  expected[3] = 10.0f;
  expected[4] =  8.0f;
  expected[5] =  6.0f;

  CHECK_RANGE(result, expected, 8);

  conv_input.add_block(input);
  CHECK(conv_output.queues_empty());
  //conv_filter.rotate_queues();  // not necessary, because queues are empty
  result = conv_output.convolve();

  CHECK_RANGE(result, zeros, 8);

  CHECK(conv_output.queues_empty());
}

SECTION("StaticOutput impulse", "")
{
  float one = 1.0f;

  auto sconv_input = c::Input(8, 1);
  auto sconv_output = c::StaticOutput(sconv_input, &one, (&one)+1);

  sconv_input.add_block(test_signal);
  result = sconv_output.convolve();

  CHECK_RANGE(result, test_signal, 8);

  sconv_input.add_block(test_signal + 8);
  result = sconv_output.convolve();

  CHECK_RANGE(result, test_signal + 8, 8);
}

SECTION("StaticOutput frequency domain", "")
{
  float one = 1.0f;
  // 3 partitions (only 1 is really needed), blocksize 8
  auto fd_filter = c::Filter(8, 3);
  // 7 partitions (just for fun), blocksize 8
  auto sconv_input = c::Input(8, 7);

  conv_input.prepare_filter(&one, (&one)+1, fd_filter);
  auto sconv_output = c::StaticOutput(sconv_input, fd_filter);

  CHECK(sconv_input.partitions() == 7);
  CHECK(fd_filter.partitions() == 3);

  sconv_input.add_block(test_signal);
  result = sconv_output.convolve();

  CHECK_RANGE(result, test_signal, 8);
}

SECTION("combinations", "")
{
  auto so = c::StaticOutput(conv_input, filter);

  auto conv = c::Convolver(8, partitions);
  conv.set_filter(filter);
  conv.rotate_queues();

  auto sconv = c::StaticConvolver(filter, partitions);

  float input[8] = { 0.0f };

  input[1] = 1.0f;
  conv_input.add_block(input);
  conv.add_block(input);
  sconv.add_block(input);

  result = so.convolve();
  CHECK_RANGE(result, zeros, 8);
  result = conv.convolve();
  CHECK_RANGE(result, zeros, 8);
  result = sconv.convolve();
  CHECK_RANGE(result, zeros, 8);

  input[1] = 2.0f;
  conv_input.add_block(input);
  conv.add_block(input);
  sconv.add_block(input);

  float expected[8] = { 0.0f };
  expected[3] = 5.0f;
  expected[4] = 4.0f;
  expected[5] = 3.0f;

  result = so.convolve();
  CHECK_RANGE(result, expected, 8);
  result = conv.convolve();
  CHECK_RANGE(result, expected, 8);
  result = sconv.convolve();
  CHECK_RANGE(result, expected, 8);

  input[1] = 0.0f;
  conv_input.add_block(input);
  conv.add_block(input);
  sconv.add_block(input);

  expected[3] = 10.0f;
  expected[4] =  8.0f;
  expected[5] =  6.0f;

  result = so.convolve();
  CHECK_RANGE(result, expected, 8);
  result = conv.convolve();
  CHECK_RANGE(result, expected, 8);
  result = sconv.convolve();
  CHECK_RANGE(result, expected, 8);

  conv_input.add_block(input);
  conv.add_block(input);
  sconv.add_block(input);

  result = so.convolve();
  CHECK_RANGE(result, zeros, 8);
  result = conv.convolve();
  CHECK_RANGE(result, zeros, 8);
  result = sconv.convolve();
  CHECK_RANGE(result, zeros, 8);
}

// TODO: test copy_nested() and transform_nested()!

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
