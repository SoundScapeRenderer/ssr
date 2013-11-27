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

// Tests for math functions.

#include "apf/math.h"

#include "catch/catch.hpp"

using namespace apf::math;

TEST_CASE("math", "Test all functions of math namespace")
{

SECTION("pi", "")
{
  CHECK(pi<float>() == 4.0f * std::atan(1.0f));
  CHECK(pi<double>() == 4.0 * std::atan(1.0));
  CHECK(pi<long double>() == 4.0l * std::atan(1.0l));

  // pi divided by 180

  CHECK((pi_div_180<float>() * 180.0f / pi<float>()) == 1.0f);
  CHECK((pi_div_180<double>() * 180.0 / pi<double>()) == 1.0);
  CHECK((pi_div_180<long double>() * 180.0l / pi<long double>()) == 1.0l);
}

SECTION("square", "a*a")
{
  CHECK(square(2.0f) == 4.0f);
  CHECK(square(2.0) == 4.0f);
  CHECK(square(2.0l) == 4.0l);
}

SECTION("dB2linear", "")
{
  CHECK(dB2linear(6.0f) == Approx(1.99526231));
  CHECK(dB2linear(6.0) == Approx(1.99526231));
  // Approx doesn't exist for long double
  CHECK(static_cast<double>(dB2linear(6.0l)) == Approx(1.99526231));

  // now with the "power" option

  CHECK(dB2linear(3.0f, true) == Approx(1.99526231));
  CHECK(dB2linear(3.0, true) == Approx(1.99526231));
  CHECK(static_cast<double>(dB2linear(3.0l, true)) == Approx(1.99526231));
}

SECTION("linear2dB", "")
{
  CHECK(linear2dB(1.99526231f) == Approx(6.0));
  CHECK(linear2dB(1.99526231) == Approx(6.0));
  CHECK(static_cast<double>(linear2dB(1.99526231l)) == Approx(6.0));

  CHECK(linear2dB(1.99526231f, true) == Approx(3.0));
  CHECK(linear2dB(1.99526231, true) == Approx(3.0));
  CHECK(static_cast<double>(linear2dB(1.99526231l, true)) == Approx(3.0));

  CHECK(linear2dB(0.0f) == -std::numeric_limits<float>::infinity());
  CHECK(linear2dB(0.0) == -std::numeric_limits<double>::infinity());
  CHECK(linear2dB(0.0l) == -std::numeric_limits<long double>::infinity());

  // TODO: how to check NaN results?
  // linear2dB(-0.1f)
  // linear2dB(-0.1)
  // linear2dB(-0.1l)
}

SECTION("deg2rad", "")
{
  CHECK(deg2rad(180.0f) == pi<float>());
  CHECK(deg2rad(180.0) == pi<double>());
  CHECK(deg2rad(180.0l) == pi<long double>());
}

SECTION("rad2deg", "")
{
  CHECK(rad2deg(pi<float>()) == 180.0f);
  CHECK(rad2deg(pi<double>()) == 180.0);
  CHECK(rad2deg(pi<long double>()) == 180.0l);
}

SECTION("wrap int", "")
{
  CHECK(wrap(-1, 7) == 6);
  CHECK(wrap(0, 7) == 0);
  CHECK(wrap(6, 7) == 6);
  CHECK(wrap(7, 7) == 0);
  CHECK(wrap(8, 7) == 1);
}

SECTION("wrap double", "")
{
  CHECK(wrap(-0.5, 360.0) == 359.5);
  CHECK(wrap(0.0, 360.0) == 0.0);
  CHECK(wrap(359.5, 360.0) == 359.5);
  CHECK(wrap(360.0, 360.0) == 0.0);
  CHECK(wrap(360.5, 360.0) == 0.5);
}

SECTION("wrap_two_pi", "")
{
#define WRAP_TWO_PI(type) \
  CHECK(wrap_two_pi(-pi<type>()) == pi<type>()); \
  CHECK(wrap_two_pi( pi<type>()) == pi<type>()); \
  CHECK(wrap_two_pi(2 * pi<type>()) == 0.0); \
  CHECK(wrap_two_pi(3 * pi<type>()) == pi<type>());

  // TODO: use Approx, enable float and long double

  WRAP_TWO_PI(double);
  //WRAP_TWO_PI(float);
  //WRAP_TWO_PI(long double);

#undef WRAP_TWO_PI
}

SECTION("next_power_of_2", "")
{
  CHECK(next_power_of_2(-3) == 1);
  CHECK(next_power_of_2(-2) == 1);
  CHECK(next_power_of_2(-1) == 1);
  CHECK(next_power_of_2(0) == 1);
  CHECK(next_power_of_2(1) == 1);
  CHECK(next_power_of_2(2) == 2);
  CHECK(next_power_of_2(3) == 4);

  CHECK(next_power_of_2(1.0f) == 1.0f);
  CHECK(next_power_of_2(2.0f) == 2.0f);
  CHECK(next_power_of_2(3.0f) == 4.0f);
  CHECK(next_power_of_2(1.0) == 1.0);
  CHECK(next_power_of_2(2.0) == 2.0);
  CHECK(next_power_of_2(2.5) == 4.0);
  CHECK(next_power_of_2(3.0) == 4.0);
  CHECK(next_power_of_2(1.0l) == 1.0l);
  CHECK(next_power_of_2(2.0l) == 2.0l);
  CHECK(next_power_of_2(3.0l) == 4.0l);
}

SECTION("max_amplitude", "")
{
  auto sig = std::vector<double>(5);

  CHECK(max_amplitude(sig.begin(), sig.end()) == 0.0);

  sig[2] = -2.0;
  CHECK(max_amplitude(sig.begin(), sig.end()) == 2.0);

  sig[3] = 4.0;
  CHECK(max_amplitude(sig.begin(), sig.end()) == 4.0);
}

SECTION("rms", "")
{
  auto sig = std::vector<double>(5);

  CHECK(rms(sig.begin(), sig.end()) == 0.0);

  sig[0] = -1.0;
  sig[1] = -1.0;
  sig[2] = -1.0;
  sig[3] =  1.0;
  sig[4] =  1.0;

  CHECK(rms(sig.begin(), sig.end()) == 1.0);
}

SECTION("raised_cosine", "")
{
  auto rc1 = raised_cosine<float>(1.5f);
  CHECK(rc1(0.75f) == 0.0f);
  CHECK(rc1(1.5f) == 1.0f);

  auto rc2 = raised_cosine<double>(1.5);
  CHECK(rc2(0.75) == 0.0);
  CHECK(rc2(1.5) == 1.0);

  auto rc3 = raised_cosine<double>(360);
  CHECK(rc3(60) == 0.75);
}

SECTION("linear_interpolator", "")
{
  auto in = linear_interpolator<double>(1.5, 3.0, 3.0);
  CHECK(in(0.0) == 1.5);
  CHECK(in(1.0) == 2.0);
  CHECK(in(2.0) == 2.5);

  // using default length 1:
  in = make_linear_interpolator(5.0, 6.0);
  CHECK(in(0.0) == 5.0);
  CHECK(in(0.5) == 5.5);
  CHECK(in(1.0) == 6.0);
}

SECTION("linear_interpolator, integer index", "")
{
  auto in = linear_interpolator<double, int>(5.0, 6.0, 2);
  CHECK(in(0) == 5.0);
  CHECK(in(1) == 5.5);
}

SECTION("linear_interpolator, integer index converted to double", "")
{
  auto in = linear_interpolator<double>(1.0, 2.0, 4);
  CHECK(in(0) == 1.0);
  CHECK(in(1) == 1.25);
}

SECTION("identity", "")
{
  identity<float> id;
  CHECK(id(0.5f) == 0.5f);
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
