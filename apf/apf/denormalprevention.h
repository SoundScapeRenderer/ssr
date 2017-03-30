/******************************************************************************
 Copyright (c) 2012-2016 Institut für Nachrichtentechnik, Universität Rostock
 Copyright (c) 2006-2012 Quality & Usability Lab
                         Deutsche Telekom Laboratories, TU Berlin

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*******************************************************************************/

// https://AudioProcessingFramework.github.io/

/// @file
/// Different methods to prevent denormal numbers.

#ifndef APF_DENORMALPREVENTION_H
#define APF_DENORMALPREVENTION_H

#include <limits>  // for std::numeric_limits()
#include <cmath>  // for std::abs()

#ifdef __SSE__
#include <xmmintrin.h>  // for SSE intrinsics
#endif
#ifdef __SSE3__
#include <pmmintrin.h>  // for SSE3 intrinsics
#endif

namespace apf
{

/// Denormal prevention
/// @see Laurent de Soras, "Denormal numbers in floating point signal processing
///   applications": http://ldesoras.free.fr/doc/articles/denormal-en.pdf
namespace dp
{

/// Disable denormal prevention.
template<typename T>
struct none
{
  void prevent_denormals(T&) {}
};

template<typename> struct dc;  // default case not implemented!

/// Add DC signal (float specialization).
template<>
struct dc<float>
{
  static void prevent_denormals(float& val) { val += 1e-18f; }
};

/// Add DC signal (double specialization).
template<>
struct dc<double>
{
  static void prevent_denormals(double& val) { val += 1e-30; }
};

template<typename> struct ac;  // default case not implemented!

/// Add sine component at nyquist frequency (float specialization).
template<>
struct ac<float>
{
  public:
    ac() : _anti_denorm(1e-18f) {}

    void prevent_denormals(float& val)
    {
      _anti_denorm = -_anti_denorm;
      val += _anti_denorm;
    }

  private:
    float _anti_denorm;
};

/// Add sine component at nyquist frequency (double specialization).
template<>
struct ac<double>
{
  public:
    ac() : _anti_denorm(1e-30) {}

    void prevent_denormals(double& val)
    {
      _anti_denorm = -_anti_denorm;
      val += _anti_denorm;
    }

  private:
    double _anti_denorm;
};

template<typename> struct quantization;  // default case not implemented!

/// Quantize denormal numbers (float specialization).
template<>
struct quantization<float>
{
  static void prevent_denormals(float& val)
  {
    val += 1e-18f;
    val -= 1e-18f;
  }
};

/// Quantize denormal numbers (double specialization).
template<>
struct quantization<double>
{
  static void prevent_denormals(double& val)
  {
    val += 1e-30;
    val -= 1e-30;
  }
};

/// Detect denormals and set 0.
template<typename T>
struct set_zero_1
{
  static void prevent_denormals(T& val)
  {
    if (std::abs(val) < std::numeric_limits<T>::min() && (val != 0)) val = 0;
  }
};

/// Detect denormals and set 0.
template<typename T>
struct set_zero_2
{
  static void prevent_denormals(T& val)
  {
    if ((val != 0) && std::abs(val) < std::numeric_limits<T>::min()) val = 0;
  }
};

/// Detect denormals and set 0.
template<typename T>
struct set_zero_3
{
  static void prevent_denormals(T& val)
  {
    if (std::abs(val) < std::numeric_limits<T>::min()) val = 0;
  }
};

#if 0
// add noise component; equally distributed spectrum
// NOTE: noise appears to be kind of deterministic
// - temporarily deactivated due to warnings

template<typename> struct NoisePrevention;

template<>
struct NoisePrevention<float>
{
  public:
    NoisePrevention() : _rand_state(1) {}

    void prevent_denormals(float& val)
    {
      _rand_state = _rand_state * 1234567UL + 890123UL;
      int mantissa = _rand_state & 0x807F0000; // Keep only most significant bits
      int flt_rnd = mantissa | 0x1E000000; // Set exponent
      val += *reinterpret_cast<const float*>(&flt_rnd);
    }

  private:
    unsigned int _rand_state;
};
#endif

#ifdef __SSE__
// The compiler must be set to generate SSE instructions automatically!
// In GCC, this is done with -mfpmath=sse (which is on by default on amd64)

/// Set Flush-To-Zero (FTZ).
/// @note requires SSE support
inline void ftz_on()
{
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
}

/// Unset Flush-To-Zero (FTZ).
/// @note requires SSE support
inline void ftz_off()
{
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
}

#ifdef __SSE3__
/// Set Denormals-Are-Zero (DAZ).
/// @note requires SSE3 support
///
/// From http://softpixel.com/~cwright/programming/simd/sse.php:
///
/// DAZ wasn't available in the first version of SSE. Since setting a reserved
/// bit in MXCSR causes a general protection fault, we need to be able to check
/// the availability of this feature without causing problems. To do this, one
/// needs to set up a 512-byte area of memory to save the SSE state to, using
/// fxsave, and then one needs to inspect bytes 28 through 31 for the MXCSR_MASK
/// value. If bit 6 is set, DAZ is supported, otherwise, it isn't.
void daz_on()
{
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}

/// Unset Denormals-Are-Zero (DAZ).
/// @note requires SSE3 support
void daz_off()
{
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
}
#endif
#endif

}  // namespace dp

}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
