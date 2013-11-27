/******************************************************************************
 * Copyright © 2012-2013 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://spatialaudio.net/ssr                           ssr@spatialaudio.net *
 ******************************************************************************/

// NFC-HOA renderer as MEX file for GNU Octave and MATLAB.

#include <mex.h>
#include <memory>  // for std::auto_ptr

#ifdef SSR_MEX_USE_DOUBLE
#define APF_MIMOPROCESSOR_SAMPLE_TYPE double
#else
#define APF_MIMOPROCESSOR_SAMPLE_TYPE float
#endif

#include "apf/pointer_policy.h"
#include "apf/posix_thread_policy.h"
#include "apf/stringtools.h"

using apf::str::S2A;

#include "nfchoarenderer.h"

// The single entry-point for Matlab is the function mexFunction(), see below!

// global variables holding the state
std::auto_ptr<ssr::NfcHoaRenderer> engine;
mwSize in_channels, out_channels, block_size, sample_rate, threads;
typedef ssr::NfcHoaRenderer::sample_type sample_type;
std::vector<sample_type*> inputs, outputs;

// TODO: separate file with generic helper functions (maybe apf::mex namespace?)

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
  if (temp != floor(temp)) return false;
  out = temp;
  return true;
}

bool convert(const mxArray* in, size_t& out)
{
  if (!mxIsDouble(in) || mxIsComplex(in)) return false;
  if (mxGetNumberOfElements(in) != 1) return false;
  double temp = mxGetScalar(in);
  if (temp < 0 || temp != floor(temp)) return false;
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

void error_init()
{
  if (!engine.get())
  {
    mexErrMsgTxt("ssr_nfc_hoa isn't initialized, use 'init' first!");
  }
}

void help(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("help");
  APF_MEX_ERROR_NO_FURTHER_INPUTS("help");

  mexPrintf("TODO: write help text!\n");
}

void init(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("init");

  std::string reproduction_setup;
  mex::next_arg(nrhs, prhs, reproduction_setup, "'init': Second argument "
      "must be a string (the reproduction setup file name)!");

  mex::next_arg(nrhs, prhs, in_channels
      , "'init': Third argument must be the number of input channels!");

  mex::next_arg(nrhs, prhs, block_size
      , "'init': Fourth argument must be the block size!");

  mex::next_arg(nrhs, prhs, sample_rate
      , "'init': Fifth argument must be the sample rate!");

  threads = 1;  // TODO: get reasonable default value
  mex::next_optarg(nrhs, prhs, threads
      , "'init': Sixth argument: number of threads!");

  APF_MEX_ERROR_NO_FURTHER_INPUTS("init");

  mexPrintf("Starting ssr_nfc_hoa with following settings:\n"
      " * reproduction setup: %s\n"
      " * in channels: %d\n"
      " * block size: %d\n"
      " * sample rate: %d\n"
      " * threads: %d\n"
      , reproduction_setup.c_str(), in_channels
      , block_size, sample_rate, threads);

  apf::parameter_map params;
  params.set("reproduction_setup", reproduction_setup);
  // TODO: set XML Schema file?
  //params.set("xml_schema", xml_schema);
  params.set("block_size", block_size);
  params.set("sample_rate", sample_rate);
  params.set("threads", threads);
  engine.reset(new ssr::NfcHoaRenderer(params));

  engine->load_reproduction_setup();

  out_channels = engine->get_output_list().size();

  for (mwSize i = 0; i < in_channels; ++i)
  {
    // TODO: specify ID?
    engine->add_source();
  }

  inputs.resize(in_channels);
  outputs.resize(out_channels);

  engine->activate();  // start parallel processing (if threads > 1)

  mexPrintf("Initialization completed, %d outputs available.\n", out_channels);
}

void process(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  error_init();

  if (nlhs != 1 || nrhs != 1)
  {
    mexErrMsgTxt("Exactly one input and one output is needed for 'process'!");
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

#ifdef SSR_MEX_USE_DOUBLE
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

void source(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  error_init();
  APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("source");

  std::string command;
  mex::next_arg(nrhs, prhs, command, "'source': second argument must be a "
      "string (the source property to set)!");

  if (command == "position")
  {
    if (nrhs < 1)
    {
      mexErrMsgTxt("'source position' needs a further argument!");
    }
    if (mxIsComplex(prhs[0]))
    {
      mexErrMsgTxt("Complex values are not allowed!");
    }
    if (!mxIsNumeric(prhs[0]))
    {
      mexErrMsgTxt("source positions must be in a numeric matrix!");
    }
    if (static_cast<mwSize>(mxGetN(prhs[0])) != in_channels)
    {
      mexErrMsgTxt("Number of columns must be the same as number of sources!");
    }
    if (mxGetM(prhs[0]) == 3)
    {
      mexErrMsgTxt("Three-dimensional positions are not supported (yet)!");
    }
    if (mxGetM(prhs[0]) != 2)
    {
      mexErrMsgTxt("Number of rows must be 2 (x and y coordinates)!");
    }

    double* coordinates = mxGetPr(prhs[0]);

    --nrhs; ++prhs;
    APF_MEX_ERROR_NO_FURTHER_INPUTS("source position");

    for (mwSize i = 0; i < in_channels; ++i)
    {
      // TODO: handle 3D coordinates

      ssr::NfcHoaRenderer::SourceBase* source = engine->get_source(i + 1);
      // TODO: check if source == nullptr
      source->position = Position(coordinates[i*2], coordinates[i*2+1]);
    }
  }
  else if (command == "orientation")
  {
    if (nrhs < 1)
    {
      mexErrMsgTxt("'source orientation' needs a further argument!");
    }
    if (mxIsComplex(prhs[0]))
    {
      mexErrMsgTxt("Complex values are not allowed!");
    }
    if (!mxIsNumeric(prhs[0]))
    {
      mexErrMsgTxt("source orientations must be in a numeric matrix!");
    }
    if (static_cast<mwSize>(mxGetN(prhs[0])) != in_channels)
    {
      mexErrMsgTxt("Number of columns must be the same as number of sources!");
    }
    if (mxGetM(prhs[0]) != 1)
    {
      mexErrMsgTxt("Last argument must be a row vector of angles!");
    }

    double* angles = mxGetPr(prhs[0]);

    --nrhs; ++prhs;
    APF_MEX_ERROR_NO_FURTHER_INPUTS("source orientation");

    for (mwSize i = 0; i < in_channels; ++i)
    {
      ssr::NfcHoaRenderer::SourceBase* source = engine->get_source(i + 1);
      // TODO: check if source == nullptr
      source->orientation = Orientation(angles[i]);  // degree
    }
  }
  else if (command == "model")
  {
    if (nrhs != in_channels)
    {
      mexErrMsgTxt("Specify as many model strings as there are sources!");
    }

    for (int i = 0; i < in_channels; ++i)
    {
      std::string model_str;
      mex::next_arg(nrhs, prhs, model_str, "All further arguments to "
          "'source model' must be a valid source model strings!");

      Source::model_t model = Source::unknown;
      if (!S2A(model_str, model))
      {
        mexPrintf("Model string '%s':", model_str.c_str());
        mexErrMsgTxt("Couldn't convert source model string!");
      }
      engine->get_source(i + 1)->model = model;
    }
  }
  else
  {
    // TODO: more stuff: mute, volume, ...

    mexPrintf("Command: 'source %s'\n", command.c_str());
    mexErrMsgTxt("Unknown command!");
  }
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
  try
  {
    std::string command;
    mex::next_arg(nrhs, prhs, command
        , "First argument must be a string (e.g. 'help')!");

    if (command == "help")
    {
      help(nlhs, plhs, nrhs, prhs);
    }
    else if (command == "init")
    {
      init(nlhs, plhs, nrhs, prhs);
    }
    else if (command == "process")
    {
      process(nlhs, plhs, nrhs, prhs);
    }
    else if (command == "source")
    {
      source(nlhs, plhs, nrhs, prhs);
    }
    else if (command == "block_size")
    {
      error_init();
      APF_MEX_ERROR_NO_FURTHER_INPUTS("block_size");
      APF_MEX_ERROR_ONLY_ONE_OUTPUT("block_size");
      plhs[0] = mxCreateDoubleScalar(block_size);
    }
    else if (command == "out_channels")
    {
      error_init();
      APF_MEX_ERROR_NO_FURTHER_INPUTS("out_channels");
      APF_MEX_ERROR_ONLY_ONE_OUTPUT("out_channels");
      plhs[0] = mxCreateDoubleScalar(out_channels);
    }
    // Only "clear" shall be documented, the others are hidden features
    else if (command == "free" || command == "delete" || command == "clear")
    {
      APF_MEX_ERROR_NO_FURTHER_INPUTS("clear");
      APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("clear");
      // This is safe even if engine wasn't initialized before:
      engine.reset();
    }
    else
    {
      mexPrintf("Command: '%s'\n", command.c_str());
      mexErrMsgTxt("Unknown command!");
    }
  }
  catch (std::exception& e)
  {
    mexErrMsgTxt(e.what());
  }
  catch (...)
  {
    mexErrMsgTxt("Unknown exception!");
  }
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
