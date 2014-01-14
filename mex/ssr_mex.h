/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
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

/// @file
/// SSR renderers as MEX file for GNU Octave and MATLAB.

#ifndef SSR_MEX_H
#define SSR_MEX_H

#ifdef SSR_MEX_USE_DOUBLE
#define APF_MIMOPROCESSOR_SAMPLE_TYPE double
#else
#define APF_MIMOPROCESSOR_SAMPLE_TYPE float
#endif

#include <memory>  // for std::unique_ptr

#include "apf/mextools.h"
#include "apf/stringtools.h"
#include "apf/pointer_policy.h"

#ifndef APF_MIMOPROCESSOR_THREAD_POLICY
#include "apf/posix_thread_policy.h"
#endif

#include "../src/source.h"

template<typename Renderer>
class SsrMex
{
  public:
    using sample_type = typename Renderer::sample_type;

    void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
    {
      namespace mex = apf::mex;

      try
      {
        if (nrhs == 0)
        {
          _help(nlhs, plhs, nrhs, prhs);
          return;
        }

        std::string command;
        mex::next_arg(nrhs, prhs, command
            , "First argument must be a string (e.g. 'help')!");

        if (command == "help")
        {
          _help(nlhs, plhs, nrhs, prhs);
        }
        else if (command == "init")
        {
          _init(nlhs, plhs, nrhs, prhs);
        }
        else if (command == "process")
        {
          _process(nlhs, plhs, nrhs, prhs);
        }
        else if (command == "source")
        {
          _source(nlhs, plhs, nrhs, prhs);
        }
        else if (command == "block_size")
        {
          _error_init();
          APF_MEX_ERROR_NO_FURTHER_INPUTS("block_size");
          APF_MEX_ERROR_ONLY_ONE_OUTPUT("block_size");
          plhs[0] = mxCreateDoubleScalar(_block_size);
        }
        else if (command == "out_channels")
        {
          _error_init();
          APF_MEX_ERROR_NO_FURTHER_INPUTS("out_channels");
          APF_MEX_ERROR_ONLY_ONE_OUTPUT("out_channels");
          plhs[0] = mxCreateDoubleScalar(_out_channels);
        }
        // Only "clear" shall be documented, the others are hidden features
        else if (command == "free" || command == "delete" || command == "clear")
        {
          APF_MEX_ERROR_NO_FURTHER_INPUTS("clear");
          APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("clear");
          // This is safe even if engine wasn't initialized before:
          _engine.reset();
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

  protected:
    std::unique_ptr<Renderer> _engine;
    mwSize _in_channels, _out_channels, _block_size;
    std::vector<sample_type*> _inputs, _outputs;

  private:
    void _help(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
    {
      APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("help");
      APF_MEX_ERROR_NO_FURTHER_INPUTS("help");

      mexPrintf("\n%1$s: SSR as MEX file\n\n"
"...\n"
"sub-commands: 'init', ...\n"
"...\n"
"\n"
"'init'\n"
"\n"
"...\n"
"    sources = 4;\n"
"    params.sample_rate = 44100;\n"
"    params.block_size = 128;\n"
"    %1$s('init', sources, params)\n"
"...\n"
"TODO: write more help text!\n"
"\n"
          , mexFunctionName());
    }

    void _error_init()
    {
      if (!_engine)
      {
        mexErrMsgTxt("Not initialized, use 'init' first!");
      }
    }

    void _init(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
    {
      namespace mex = apf::mex;

      APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("init");

      mex::next_arg(nrhs, prhs, _in_channels
          , "First argument to 'init' must be the number of sources!");

      std::map<std::string, std::string> options;

      mex::next_arg(nrhs, prhs, options
          , "Second argument to 'init' must be a scalar structure!");

      // Note: Fields are not checked, the SSR is supposed to do that.

      APF_MEX_ERROR_NO_FURTHER_INPUTS("init");

      auto info = std::string("Starting the SSR with following settings:\n");

      info += " * number of sources: ";
      info += apf::str::A2S(_in_channels);
      info += "\n";

      for (auto it: options)
      {
        info += " * ";
        info += it.first;
        info += ": ";
        info += it.second;
        info += "\n";
      }

      mexPrintf(info.c_str());

      auto params = apf::parameter_map(std::move(options));

      _engine.reset(new Renderer(params));

      _block_size = _engine->block_size();

      _engine->load_reproduction_setup();

      _out_channels = _engine->get_output_list().size();

      for (mwSize i = 0; i < _in_channels; ++i)
      {
        // TODO: specify ID?
        _engine->add_source();
      }

      _inputs.resize(_in_channels);
      _outputs.resize(_out_channels);

      _engine->activate();  // start parallel processing (if threads > 1)

      mexPrintf("Initialization completed, %d outputs available.\n", _out_channels);
      // TODO: print renderer name?
    }

    void _process(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
    {
      _error_init();

      if (nlhs != 1 || nrhs != 1)
      {
        mexErrMsgTxt("Exactly one input and one output is needed for 'process'!");
      }
      if (static_cast<mwSize>(mxGetM(prhs[0])) != _block_size)
      {
        mexErrMsgTxt("Number of rows must be the same as block size!");
      }
      if (static_cast<mwSize>(mxGetN(prhs[0])) != _in_channels)
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
      plhs[0] = mxCreateDoubleMatrix(_block_size, _out_channels, mxREAL);
      sample_type* output = mxGetPr(plhs[0]);
      sample_type*  input = mxGetPr(prhs[0]);
#else
      if (mxGetClassID(prhs[0]) != mxSINGLE_CLASS)
      {
        mexErrMsgTxt("This function only works with single precision data!");
      }
      plhs[0] = mxCreateNumericMatrix(_block_size, _out_channels
          , mxSINGLE_CLASS, mxREAL);
      sample_type* output = static_cast<sample_type*>(mxGetData(plhs[0]));
      sample_type*  input = static_cast<sample_type*>(mxGetData(prhs[0]));
#endif

      for (int i = 0; i <= _in_channels; ++i)
      {
        _inputs[i] = input;
        input += _block_size;
      }

      for (int i = 0; i <= _out_channels; ++i)
      {
        _outputs[i] = output;
        output += _block_size;
      }

      _engine->audio_callback(_block_size, _inputs.data(), _outputs.data());
    }

    void _source(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
    {
      namespace mex = apf::mex;

      _error_init();
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
        if (static_cast<mwSize>(mxGetN(prhs[0])) != _in_channels)
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

        for (mwSize i = 0; i < _in_channels; ++i)
        {
          // TODO: handle 3D coordinates

          auto* source = _engine->get_source(i + 1);
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
        if (static_cast<mwSize>(mxGetN(prhs[0])) != _in_channels)
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

        for (mwSize i = 0; i < _in_channels; ++i)
        {
          auto* source = _engine->get_source(i + 1);
          // TODO: check if source == nullptr
          source->orientation = Orientation(angles[i]);  // degree
        }
      }
      else if (command == "model")
      {
        if (nrhs != _in_channels)
        {
          mexErrMsgTxt("Specify as many model strings as there are sources!");
        }

        for (int i = 0; i < _in_channels; ++i)
        {
          std::string model_str;
          mex::next_arg(nrhs, prhs, model_str, "All further arguments to "
              "'source model' must be a valid source model strings!");

          Source::model_t model = Source::unknown;
          if (!apf::str::S2A(model_str, model))
          {
            mexPrintf("Model string '%s':", model_str.c_str());
            mexErrMsgTxt("Couldn't convert source model string!");
          }
          _engine->get_source(i + 1)->model = model;
        }
      }
      else
      {
        // TODO: more stuff: mute, volume, ...

        mexPrintf("Command: 'source %s'\n", command.c_str());
        mexErrMsgTxt("Unknown command!");
      }
    }
};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
