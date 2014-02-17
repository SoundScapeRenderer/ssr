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

// Example for the MimoProcessor running as a Matlab (or GNU octave) MEX file.
//
// Compile for GNU Octave: mkoctfile --mex mex_simpleengine.cpp
// Compile for Matlab: not tested yet!
//
// Usage example:
//
// mex_simpleprocessor('init', 2, 3, 2, 4, 44100)
// x = mex_simpleprocessor('process', single(ones(4,2)))
// mex_simpleprocessor clear

#include <mex.h>
#include <string>
#include <vector>
#include <memory>  // for std::unique_ptr

#ifdef MEX_USE_DOUBLE
#define APF_MIMOPROCESSOR_SAMPLE_TYPE double
#else
#define APF_MIMOPROCESSOR_SAMPLE_TYPE float
#endif

#include "apf/pointer_policy.h"
#include "apf/posix_thread_policy.h"

#include "simpleprocessor.h"

// The single entry-point for Matlab is the function mexFunction(), see below!

using typename SimpleProcessor::sample_type;

// global variables holding the state
std::unique_ptr<SimpleProcessor> engine;
mwSize in_channels, out_channels, threads=1, block_size=64, sample_rate=44100;
std::vector<sample_type*> inputs, outputs;

void engine_init(int nrhs, const mxArray* prhs[])
{
  if (nrhs < 2)
  {
    mexErrMsgTxt("At least 2 further parameters are needed for \"init\"!");
  }
  if (nrhs > 0)
  {
    in_channels = static_cast<mwSize>(*mxGetPr(prhs[0]));
    --nrhs; ++prhs;
  }
  if (nrhs > 0)
  {
    out_channels = static_cast<mwSize>(*mxGetPr(prhs[0]));
    --nrhs; ++prhs;
  }
  if (nrhs > 0)
  {
    threads = static_cast<mwSize>(*mxGetPr(prhs[0]));
    --nrhs; ++prhs;
  }
  if (nrhs > 0)
  {
    block_size = static_cast<mwSize>(*mxGetPr(prhs[0]));
    --nrhs; ++prhs;
  }
  if (nrhs > 0)
  {
    sample_rate = static_cast<mwSize>(*mxGetPr(prhs[0]));
    --nrhs; ++prhs;
  }
  if (nrhs > 0)
  {
    mexErrMsgTxt("Too many input arguments!");
  }

  mexPrintf("Starting SimpleProcessor with following settings:\n"
      " * in channels: %d\n"
      " * out channels: %d\n"
      " * threads: %d\n"
      " * block size: %d\n"
      " * sample rate: %d\n"
#ifdef MEX_USE_DOUBLE
      " * data type: double precision\n"
#else
      " * data type: single precision\n"
#endif
      , in_channels, out_channels, threads, block_size, sample_rate);

  auto temp = apf::parameter_map();
  temp.set("in_channels", in_channels);
  temp.set("out_channels", out_channels);
  temp.set("threads", threads);
  temp.set("block_size", block_size);
  temp.set("sample_rate", sample_rate);
  engine.reset(new SimpleProcessor(temp));

  inputs.resize(in_channels);
  outputs.resize(out_channels);
}

void engine_process(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  if (!engine)
  {
    mexErrMsgTxt("SimpleProcessor isn't initialized, use 'init' first!");
  }
  if (nlhs != 1 || nrhs != 1)
  {
    mexErrMsgTxt("Exactly one input and one output is needed!");
  }
  if (static_cast<mwSize>(mxGetM(prhs[0])) != block_size)
  {
    mexErrMsgTxt("Number of rows must be the same as block size!");
  }
  if (static_cast<mwSize>(mxGetN(prhs[0])) != in_channels)
  {
    mexErrMsgTxt("Number of columns must be the same as number of inputs!");
  }
  if (mxIsComplex(prhs[0]))
  {
    mexErrMsgTxt("Complex values are not allowed!");
  }
  if (!mxIsNumeric(prhs[0]))
  {
    mexErrMsgTxt("Input must be a numeric matrix!");
  }

#ifdef MEX_USE_DOUBLE
  if (!mxIsDouble(prhs[0]))
  {
    mexErrMsgTxt("This function only works with double precision data!");
  }
  plhs[0] = mxCreateDoubleMatrix(block_size, out_channels, mxREAL);
  sample_type* output = mxGetPr(plhs[0]);
  sample_type*  input = mxGetPr(prhs[0]);
#else
  if (mxGetClassID(prhs[0]) != mxSINGLE_CLASS)
  {
    mexErrMsgTxt("This function only works with single precision data!");
  }
  plhs[0] = mxCreateNumericMatrix(block_size, out_channels
      , mxSINGLE_CLASS, mxREAL);
  sample_type* output = static_cast<sample_type*>(mxGetData(plhs[0]));
  sample_type*  input = static_cast<sample_type*>(mxGetData(prhs[0]));
#endif

  for (int i = 0; i <= in_channels; ++i)
  {
    inputs[i] = input;
    input += block_size;
  }

  for (int i = 0; i <= out_channels; ++i)
  {
    outputs[i] = output;
    output += block_size;
  }

  engine->audio_callback(block_size, inputs.data(), outputs.data());
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  auto command = std::string();

  if (nrhs >= 1 && mxIsChar(prhs[0]))
  {
    auto temp = mxArrayToString(prhs[0]);
    command = temp;
    mxFree(temp);
    --nrhs;
    ++prhs;
  }
  else
  {
    mexErrMsgTxt("First argument must be a string!");
  }

  if (command == "help")
  {
    mexPrintf("This is a useless help text.\n");
  }
  else if (command == "init")
  {
    engine_init(nrhs, prhs);
  }
  else if (command == "process")
  {
    engine_process(nlhs, plhs, nrhs, prhs);
  }
  else if (command == "free" || command == "delete" || command == "clear")
  {
    engine.reset();
  }
  else
  {
    mexPrintf("Command: \"%s\"\n", command.c_str());
    mexErrMsgTxt("Unknown command!");
  }
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
