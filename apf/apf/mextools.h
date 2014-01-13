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
#include <cmath>  // for std::floor()

#define APF_MEX_ERROR_NO_OUTPUT_SUPPORTED(name) \
(void)plhs; \
if (nlhs > 0) { \
  std::string msg("No output parameters are supported for '" \
      + std::string(name) + "'!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_NO_FURTHER_INPUTS(name) \
(void)prhs; \
if (nrhs > 0) { \
  std::string msg("No further input parameters are supported for '" \
      + std::string(name) + "'!"); \
  mexErrMsgTxt(msg.c_str()); }

#define APF_MEX_ERROR_ONLY_ONE_OUTPUT(name) \
(void)plhs; \
if (nlhs > 1) { \
  std::string msg("Only one output parameter is supported for '" \
      + std::string(name) + "'!"); \
  mexErrMsgTxt(msg.c_str()); }

namespace apf
{
namespace mex
{

// TODO: check if (and how) user-specified overloads of convert() work
// TODO: use a traits class, if necessary

bool convert(const mxArray* in, std::string& out)
{
  if (!mxIsChar(in)) return false;
  if (mxGetM(in) != 1) return false;

  char* temp = mxArrayToString(in);
  out = temp;
  mxFree(temp);
  return true;
}

bool convert(const mxArray* in, double& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  out = mxGetScalar(in);
  return true;
}

bool convert(const mxArray* in, int& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  double temp = mxGetScalar(in);
  if (temp != std::floor(temp)) return false;
  out = temp;
  return true;
}

bool convert(const mxArray* in, size_t& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  double temp = mxGetScalar(in);
  if (temp < 0 || temp != std::floor(temp)) return false;
  out = temp;
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

template<typename T>
bool next_arg(int& n, const mxArray**& p, T& data)
{
  return internal::next_arg_helper<false>(n, p, data);
}

template<typename T>
bool next_optarg(int& n, const mxArray**& p, T& data)
{
  return internal::next_arg_helper<true>(n, p, data);
}

template<typename T>
void next_arg(int& n, const mxArray**& p, T& data, const std::string& error)
{
  if (!next_arg(n, p, data)) mexErrMsgTxt(error.c_str());
}

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
