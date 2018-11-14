// Count denormals to check if denormal prevention works

// TODO: include into unit tests?

#include <map>
#include <string>
#include <iostream>

#include "apf/biquad.h"

#define MAKE_DENORMAL_COUNTER(Name, Prevention) \
template<typename T> \
struct Name : Prevention<T> { \
  void prevent_denormals(T& val) { \
    this->Prevention<T>::prevent_denormals(val); \
    if (std::abs(val) < std::numeric_limits<T>::min() && (val != 0)) \
      denormal_counter[#Name].first++; \
    denormal_counter[#Name].second++; } };

#define COUNT_DENORMALS_IN_CASCADE(coeffs, cascade, name, prevention) \
  std::vector<decltype(coeffs)> \
    temp##coeffs(cascade.number_of_sections(), coeffs); \
  cascade.set(temp##coeffs.begin(), temp##coeffs.end()); \
  denormal_counter[#name] = std::make_pair(0, 0); \
  input[0] = 1; \
  for (int n = 0; n < number_of_blocks_count; ++n) { \
    cascade.execute(input.begin(), input.end() , output.begin()); \
    input[0] = 0; } \
  std::cout << std::string(#cascade) << " (" << #prevention << ") created " \
    << denormal_counter[#name].first << " denormal numbers (" \
    << static_cast<float>(denormal_counter[#name].first) / static_cast<float>(denormal_counter[#name].second) * 100.0f \
    << "%)." << std::endl;

#define COUNT_DENORMALS(coeffs_flt, coeffs_dbl, name, prevention) { \
  apf::Cascade<apf::BiQuad<float, name>> \
    cascade_flt(number_of_sections_count); \
  apf::Cascade<apf::BiQuad<double, name>> \
    cascade_dbl(number_of_sections_count); \
  COUNT_DENORMALS_IN_CASCADE(coeffs_flt, cascade_flt, name, prevention) \
  COUNT_DENORMALS_IN_CASCADE(coeffs_dbl, cascade_dbl, name, prevention) \
  std::cout << std::endl; }

size_t block_size = 1024;
int number_of_blocks_count = 200;
size_t number_of_sections_count = 10;

// denormal counter map
std::map<std::string, std::pair<int, int>> denormal_counter;

// create denormal counter classes
MAKE_DENORMAL_COUNTER(count_dc, apf::dp::dc)
MAKE_DENORMAL_COUNTER(count_ac, apf::dp::ac)
MAKE_DENORMAL_COUNTER(count_quant, apf::dp::quantization)
MAKE_DENORMAL_COUNTER(count_set_zero_1, apf::dp::set_zero_2) // TODO: remove apf::dp::set_zero_1
MAKE_DENORMAL_COUNTER(count_set_zero_2, apf::dp::set_zero_3)
MAKE_DENORMAL_COUNTER(count_ftz_on, apf::dp::none)
MAKE_DENORMAL_COUNTER(count_daz_on, apf::dp::none)
MAKE_DENORMAL_COUNTER(count_ftz_on_and_daz_on, apf::dp::none)
MAKE_DENORMAL_COUNTER(count_none, apf::dp::none)

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

  COUNT_DENORMALS(benign_flt, benign_dbl, count_ac, apf::dp::ac)
  COUNT_DENORMALS(benign_flt, benign_dbl, count_dc, apf::dp::dc)
  COUNT_DENORMALS(benign_flt, benign_dbl, count_quant, apf::dp::quantization)
  COUNT_DENORMALS(benign_flt, benign_dbl, count_set_zero_1, apf::dp::set_zero_2)
  COUNT_DENORMALS(benign_flt, benign_dbl, count_set_zero_2, apf::dp::set_zero_3)

#ifdef __SSE__
  std::cout << "FTZ on" << std::endl;
  apf::dp::ftz_on();
  COUNT_DENORMALS(benign_flt, benign_dbl, count_ftz_on, apf::dp::none)
#ifdef __SSE3__
  std::cout << "DAZ on" << std::endl;
  apf::dp::daz_on();
  COUNT_DENORMALS(benign_flt, benign_dbl, count_daz_on, apf::dp::none)
#endif
  std::cout << "FTZ off" << std::endl;
  apf::dp::ftz_off();
#ifdef __SSE3__
  COUNT_DENORMALS(benign_flt, benign_dbl, count_ftz_on_and_daz_on, apf::dp::none)
  std::cout << "DAZ off" << std::endl;
  apf::dp::daz_off();
#endif
#endif
  COUNT_DENORMALS(benign_flt, benign_dbl, count_none, apf::dp::none)

  std::cout << "\n==> Now the malignant coefficients:" << std::endl;
  std::cout << std::endl;

  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_ac, apf::dp::ac)
  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_dc, apf::dp::dc)
  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_quant, apf::dp::quantization)
  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_set_zero_1, apf::dp::set_zero_2)
  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_set_zero_2, apf::dp::set_zero_3)

#ifdef __SSE__
  std::cout << "FTZ on" << std::endl;
  apf::dp::ftz_on();
  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_ftz_on, apf::dp::none)
#ifdef __SSE3__
  std::cout << "DAZ on" << std::endl;
  apf::dp::daz_on();
  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_daz_on, apf::dp::none)
#endif
  std::cout << "FTZ off" << std::endl;
  apf::dp::ftz_off();
#ifdef __SSE3__
  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_ftz_on_and_daz_on, apf::dp::none)
  std::cout << "DAZ off" << std::endl;
  apf::dp::daz_off();
#endif
#endif

  std::cout << "The following can take quite a while ... abort with Ctrl+C\n\n";

  COUNT_DENORMALS(malignant_flt, malignant_dbl, count_none, apf::dp::none)
}
