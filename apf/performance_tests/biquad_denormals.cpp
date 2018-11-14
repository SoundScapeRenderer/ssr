// Performance tests for BiQuad and denormal prevention.

// TODO: make proper statistics for easily comparable test runs
// TODO: run with different compiler flags (see Makefile)

#include <vector>
#include <string>

#include "apf/biquad.h"
#include "apf/stopwatch.h"

const int block_size = 1024;
const int number_of_blocks = 50000;
const int number_of_sections_test1 = 1;
const int number_of_sections_test2 = 10;
const int number_of_sections_test3 = 14;

#define FILL_CASCADE(cascade, coeffs) \
std::vector<decltype(coeffs)> temp ## cascade(cascade.number_of_sections(), coeffs); \
cascade.set(temp ## cascade.begin(), temp ## cascade.end());

#define TEST_BIQUAD_INTERNAL(coeffs, cascade_test1, cascade_test2, cascade_test3, prevention) \
  FILL_CASCADE(cascade_test1, coeffs); \
  FILL_CASCADE(cascade_test2, coeffs); \
  FILL_CASCADE(cascade_test3, coeffs); \
  { \
    apf::StopWatch watch(std::string(#cascade_test1) + " (" + #prevention + ")"); \
    input[0] = 1; \
    for (int n = 0; n < number_of_blocks; ++n) { \
      cascade_test1.execute(input.begin(), input.end() , output.begin()); \
      input[0] = 0; } \
  } \
  { \
    apf::StopWatch watch(std::string(#cascade_test2) + " (" + #prevention + ")"); \
    input[0] = 1; \
    for (int n = 0; n < number_of_blocks; ++n) { \
      cascade_test2.execute(input.begin(), input.end() , output.begin()); \
      input[0] = 0; } \
  } \
  { \
    apf::StopWatch watch(std::string(#cascade_test3) + " (" + #prevention + ")"); \
    input[0] = 1; \
    for (int n = 0; n < number_of_blocks; ++n) { \
      cascade_test3.execute(input.begin(), input.end() , output.begin()); \
      input[0] = 0; } \
  }

#define TEST_BIQUAD(coeffs_flt, coeffs_dbl, prevention) { \
  apf::Cascade<apf::BiQuad<float, prevention>> \
    cascade_test1_flt(number_of_sections_test1); \
  apf::Cascade<apf::BiQuad<double, prevention>> \
    cascade_test1_dbl(number_of_sections_test1); \
  apf::Cascade<apf::BiQuad<float, prevention>> \
    cascade_test2_flt(number_of_sections_test2); \
  apf::Cascade<apf::BiQuad<double, prevention>> \
    cascade_test2_dbl(number_of_sections_test2); \
  apf::Cascade<apf::BiQuad<float, prevention>> \
    cascade_test3_flt(number_of_sections_test3); \
  apf::Cascade<apf::BiQuad<double, prevention>> \
    cascade_test3_dbl(number_of_sections_test3); \
  TEST_BIQUAD_INTERNAL(coeffs_flt, cascade_test1_flt, cascade_test2_flt, cascade_test3_flt, prevention) \
  TEST_BIQUAD_INTERNAL(coeffs_dbl, cascade_test1_dbl, cascade_test2_dbl, cascade_test3_dbl, prevention) \
  std::cout << std::endl; }

int main()
{
  // We're only interested in single precision audio data
  std::vector<float>  input(block_size);
  std::vector<float> output(block_size);

  apf::SosCoefficients<float>  benign_flt, malignant_flt;
  apf::SosCoefficients<double> benign_dbl, malignant_dbl;

  // simple LPF
  benign_flt.b0 = 0.2f;        benign_dbl.b0 = 0.2;
  benign_flt.b1 = 0.5f;        benign_dbl.b1 = 0.5;
  benign_flt.b2 = 0.2f;        benign_dbl.b2 = 0.2;
  benign_flt.a1 = 0.5f;        benign_dbl.a1 = 0.5;
  benign_flt.a2 = 0.2f;        benign_dbl.a2 = 0.2;

  // HPF similar to the one used in DCA algorithm
  malignant_flt.b0 =  0.98f;   malignant_dbl.b0 =  0.98;
  malignant_flt.b1 = -1.9f;    malignant_dbl.b1 = -1.9;
  malignant_flt.b2 =  0.93f;   malignant_dbl.b2 =  0.93;
  malignant_flt.a1 = -1.85f;   malignant_dbl.a1 = -1.85;
  malignant_flt.a2 =  0.9f;    malignant_dbl.a2 =  0.9;

  std::cout << "\n==> First the benign coefficients:\n" << std::endl;

  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::ac)
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::dc)
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::quantization)
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::set_zero_1)
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::set_zero_2)
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::set_zero_3)

#ifdef __SSE__
  std::cout << "FTZ on" << std::endl;
  apf::dp::ftz_on();
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::none)
#ifdef __SSE3__
  std::cout << "DAZ on" << std::endl;
  apf::dp::daz_on();
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::none)
#endif
  std::cout << "FTZ off" << std::endl;
  apf::dp::ftz_off();
#ifdef __SSE3__
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::none)
  std::cout << "DAZ off" << std::endl;
  apf::dp::daz_off();
#endif
#endif
  TEST_BIQUAD(benign_flt, benign_dbl, apf::dp::none)
  std::cout << std::endl;

  std::cout << "\n==> And now the malignant coefficients:" << std::endl;

  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::ac)
  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::dc)
  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::quantization)
  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::set_zero_1)
  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::set_zero_2)
  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::set_zero_3)

#ifdef __SSE__
  std::cout << "FTZ on" << std::endl;
  apf::dp::ftz_on();
  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::none)
#ifdef __SSE3__
  std::cout << "DAZ on" << std::endl;
  apf::dp::daz_on();
  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::none)
#endif
  std::cout << "FTZ off" << std::endl;
  apf::dp::ftz_off();
#ifdef __SSE3__
  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::none)
  std::cout << "DAZ off" << std::endl;
  apf::dp::daz_off();
#endif
#endif

  std::cout << "The following can take quite a while ... abort with Ctrl+C\n\n";

  TEST_BIQUAD(malignant_flt, malignant_dbl, apf::dp::none)
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
