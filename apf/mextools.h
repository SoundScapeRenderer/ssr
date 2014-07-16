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

/// @file
/// Some tools for working with the MEX API for Matlab/Octave.

#ifndef APF_MEXTOOLS_H
#define APF_MEXTOOLS_H

#include <mex.h>
#include <string>
#include <map>
#include <cmath>  // for std::floor()

#include "apf/stringtools.h"  // for A2S()

#define APF_MEX_ERROR_NO_OUTPUT_SUPPORTED(name) \
(void)plhs; \
if (nlhs > 0) { \
  std::string msg("No output parameters are supported for " \
      + std::string(name) + "!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_EXACTLY_ONE_OUTPUT(name) \
(void)plhs; \
if (nlhs != 1) { \
  std::string msg("Exactly one output parameter is supported for " \
      + std::string(name) + "!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_ONE_OPTIONAL_OUTPUT(name) \
(void)plhs; \
if (nlhs > 1) { \
  std::string msg("No more than one output parameter is supported for " \
      + std::string(name) + "!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_NO_FURTHER_INPUTS(name) \
(void)prhs; \
if (nrhs > 0) { \
  std::string msg("No further input parameters are supported for " \
      + std::string(name) + "!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_FURTHER_INPUT_NEEDED(text) \
(void)prhs; \
if (nrhs < 1) { \
  std::string msg(std::string(text) + " needs a further input parameter!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_NUMERIC_INPUT(text) \
(void)prhs; \
if (!mxIsNumeric(prhs[0])) { \
  std::string msg(std::string(text) + " must be a numeric matrix!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_REAL_INPUT(text) \
(void)prhs; \
APF_MEX_ERROR_NUMERIC_INPUT(text); \
if (mxIsComplex(prhs[0])) { \
  std::string msg(std::string(text) + " must not be complex!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_SAME_NUMBER_OF_ROWS(value, text) \
(void)prhs; \
if (static_cast<mwSize>(mxGetM(prhs[0])) != (value)) { \
  std::string msg("Number of rows must be the same " \
      + std::string(text) + "!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_SAME_NUMBER_OF_COLUMNS(value, text) \
(void)prhs; \
if (static_cast<mwSize>(mxGetN(prhs[0])) != (value)) { \
  std::string msg("Number of columns must be the same " \
      + std::string(text) + "!"); \
  mexErrMsgTxt(msg.c_str()); }

namespace apf
{
/// Helper functions for creating MEX files
namespace mex
{

// TODO: check if (and how) user-specified overloads of convert() work
// TODO: use a traits class, if necessary

/// Convert @c mxArray to @c std::string
bool convert(const mxArray* in, std::string& out)
{
  if (!mxIsChar(in)) return false;
  if (mxGetM(in) != 1) return false;

  char* temp = mxArrayToString(in);
  out = temp;
  mxFree(temp);
  return true;
}

/// Convert @c mxArray to @c double
bool convert(const mxArray* in, double& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  out = mxGetScalar(in);
  return true;
}

/// Convert @c mxArray to @c int
bool convert(const mxArray* in, int& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  double temp = mxGetScalar(in);
  if (temp != std::floor(temp)) return false;
  out = temp;
  return true;
}

/// Convert @c mxArray to @c size_t
bool convert(const mxArray* in, size_t& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  double temp = mxGetScalar(in);
  if (temp < 0 || temp != std::floor(temp)) return false;
  out = temp;
  return true;
}

/// Convert @c mxArray to a @c std::map of @c std::string%s.
/// This expects a scalar structure!
/// Values must be real scalar numbers or strings!
/// @warning In case of a conversion error, the map may be partially filled!
// TODO: allow wstring?
bool convert(const mxArray* in, std::map<std::string, std::string>& out)
{
  if (!mxIsStruct(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;

  for (int i = 0; i < mxGetNumberOfFields(in); ++i)
  {
    auto fieldname = std::string(mxGetFieldNameByNumber(in, i));

    // Second argument: number of element (we expect only one):
    mxArray* field = mxGetFieldByNumber(in, 0, i);

    double doublevalue;
    std::string stringvalue;

    // TODO: check for size_t and int?

    if (convert(field, doublevalue))
    {
      stringvalue = apf::str::A2S(doublevalue);
    }
    else if (convert(field, stringvalue))
    {
      // do nothing
    }
    else
    {
      mexPrintf("Trying to convert '%s' ...\n", fieldname.c_str());
      mexErrMsgTxt("Value must be a real scalar number or a string!");
    }
    out[fieldname] = stringvalue;
  }
  return true;
}

namespace internal
{

template<bool optional, typename T>
bool next_arg_helper(int& n, const mxArray**& p, T& data)
{
  return (n-- < 1) ? optional : convert(p++[0], data);
}

}  // namespace internal

/// Get next argument, converted to @p T.
/// @param n Number of arguments, typically @c nrhs
/// @param p Pointer to arguments, typically @c prhs
/// @return @b true if argument was available and if conversion was successful
/// @post @p n is decremented, @p p is incremented
template<typename T>
bool next_arg(int& n, const mxArray**& p, T& data)
{
  return internal::next_arg_helper<false>(n, p, data);
}

/// Get next optional argument, converted to @p T.
/// @return @b true if no argument available or if conversion was successful
/// @see next_arg()
template<typename T>
bool next_optarg(int& n, const mxArray**& p, T& data)
{
  return internal::next_arg_helper<true>(n, p, data);
}

/// Get next argument, converted to @p T.
/// @see next_arg()
/// @param error Message to be displayed on error
template<typename T>
void next_arg(int& n, const mxArray**& p, T& data, const std::string& error)
{
  if (!next_arg(n, p, data)) mexErrMsgTxt(error.c_str());
}

/// Get next optional argument, converted to @p T.
/// @see next_optarg()
/// @param error Message to be displayed on error
template<typename T>
void next_optarg(int& n, const mxArray**& p, T& data, const std::string& error)
{
  if (!next_optarg(n, p, data)) mexErrMsgTxt(error.c_str());
}

}  // namespace mex
}  // namespace apf

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
