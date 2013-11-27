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

// Tests for FFTW tools.

#include "apf/fftwtools.h"

#include <stdint.h>  // for uintptr_t
#include "catch/catch.hpp"
#include "apf/container.h"  // for fixed_vector

bool is16bytealigned(void* ptr)
{
  return (uintptr_t(ptr) & 0xF) == 0;
}

TEST_CASE("fftw_allocator", "Test fftw_allocator")
{

SECTION("stuff", "")
{
  std::vector<float, apf::fftw_allocator<float>> vf;
  vf.push_back(3.1415f);

  CHECK(vf.front() == 3.1415f);

  std::vector<double, apf::fftw_allocator<double>> vd;
  std::vector<long double, apf::fftw_allocator<long double>> vl;

  apf::fixed_vector<float, apf::fftw_allocator<float>> ff(42);
  apf::fixed_vector<double, apf::fftw_allocator<double>> fd(42);
  apf::fixed_vector<long double, apf::fftw_allocator<long double>> fl(42);

  CHECK(is16bytealigned(&vf.front()));
  CHECK(is16bytealigned(&ff.front()));
}

} // TEST_CASE

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
