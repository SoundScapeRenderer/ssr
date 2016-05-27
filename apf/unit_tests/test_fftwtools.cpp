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
