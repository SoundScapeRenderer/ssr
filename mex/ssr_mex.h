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
#include "apf/cxx_thread_policy.h"
#include "loudspeakerrenderer.h"

#include "../src/source.h"

template<typename Renderer>
class SsrMex
{
  public:
    using sample_type = typename Renderer::sample_type;

    void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
    {
      try
      {
        if (nrhs == 0)
        {
          _help(nlhs, plhs, nrhs, prhs);
          return;
        }

        std::string command;
        apf::mex::next_arg(nrhs, prhs, command
            , "First argument must be a string (e.g. 'help')!");

        if (command == "help")
        {
          _help(nlhs, plhs, nrhs, prhs);
        }
        else if (command == "init")
        {
          _init(nlhs, plhs, nrhs, prhs);
        }
        else if (command == "block_size")
        {
          _error_init();
          APF_MEX_ERROR_NO_FURTHER_INPUTS("'block_size'");
          APF_MEX_ERROR_ONE_OPTIONAL_OUTPUT("'block_size'");
          plhs[0] = mxCreateDoubleScalar(_block_size);
        }
        else if (command == "out_channels")
        {
          _error_init();
          APF_MEX_ERROR_NO_FURTHER_INPUTS("'out_channels'");
          APF_MEX_ERROR_ONE_OPTIONAL_OUTPUT("'out_channels'");
          plhs[0] = mxCreateDoubleScalar(_out_channels);
        }
        else if (command == "loudspeaker_position"
            || command == "loudspeaker_orientation")
        {
          _loudspeaker_command(command, nlhs, plhs, nrhs, prhs);
        }
        // Only "clear" shall be documented, the others are hidden features
        else if (command == "free" || command == "delete" || command == "clear")
        {
          APF_MEX_ERROR_NO_FURTHER_INPUTS("'clear'");
          APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("'clear'");
          // This is safe even if engine wasn't initialized before:
          _engine.reset();
        }
        else
        {
          _chained_commands(command, nlhs, plhs, nrhs, prhs);
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

  private:
    void _help(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
    {
      APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("'help'");
      APF_MEX_ERROR_NO_FURTHER_INPUTS("'help'");

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
      APF_MEX_ERROR_NO_OUTPUT_SUPPORTED("'init'");

      // list for converting cell array
      std::vector<std::string> filename_list;

      // try to convert first argument to an integer
      if (!apf::mex::next_arg(nrhs, prhs, _in_channels))
      {
        // if this fails, try to convert to cell array
        apf::mex::next_arg(nrhs, prhs, filename_list
          , "First argument to 'init' must be the number of sources "
          "or a cell array of filenames!");
        // number of sources
        _in_channels = filename_list.size();
      }

      std::map<std::string, std::string> options;

      apf::mex::next_arg(nrhs, prhs, options
          , "Second argument to 'init' must be a scalar structure!");

      // Note: Fields are not checked, the SSR is supposed to do that.

      // optional input for verbosity
      bool verbose = true;
      apf::mex::next_optarg(nrhs, prhs, verbose
          , "Third argument to 'init' must be a logical or a real number!");

      APF_MEX_ERROR_NO_FURTHER_INPUTS("'init'");

      if (verbose)
      {
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
      }

      _engine.reset(new Renderer(apf::parameter_map(std::move(options))));

      _block_size = _engine->block_size();

      _engine->load_reproduction_setup();

      _out_channels = _engine->get_output_list().size();

      for (mwSize i = 0; i < _in_channels; ++i)
      {
        apf::parameter_map source_params;

        if (!filename_list.empty())
        {
          source_params.set("properties_file", filename_list[i]);
        }
        // TODO: specify ID?
        _engine->add_source(source_params);
      }

      _inputs.resize(_in_channels);
      _outputs.resize(_out_channels);

      _engine->activate();  // start parallel processing (if threads > 1)

      if (verbose)
      {
        mexPrintf("Initialization of %s completed, %d outputs available.\n"
            , _engine->name(), _out_channels);
      }
    }

    void _chained_commands(const std::string& command
        , int& nlhs, mxArray**& plhs, int& nrhs, const mxArray**& prhs)
    {
      _error_init();

      if (command == "source_position")
      {
        _source_position(nrhs, prhs);
      }
      else if (command == "source_orientation")
      {
        _source_orientation(nrhs, prhs);
      }
      else if (command == "source_mute")
      {
        _source_mute(nrhs, prhs);
      }
      else if (command == "source_model")
      {
        _source_model(nrhs, prhs);
      }
      else if (command == "reference_position")
      {
        _reference_position(nrhs, prhs);
      }
      else if (command == "reference_orientation")
      {
        _reference_orientation(nrhs, prhs);
      }
      else if (command == "process")
      {
        _process(nlhs, plhs, nrhs, prhs);
      }
      else
      {
        mexPrintf("Command: '%s'\n", command.c_str());
        mexErrMsgTxt("Unknown command!");
      }

      if (nrhs > 0)
      {
        std::string command;
        apf::mex::next_arg(nrhs, prhs, command
            , "Too many arguments (or missing command string)!");
        _chained_commands(command, nlhs, plhs, nrhs, prhs);
      }

      if (nlhs != 0)
      {
        mexErrMsgTxt("Output argument(s) available but not needed!");
      }
    }

    void _process(int& nlhs, mxArray**& plhs, int& nrhs, const mxArray**& prhs)
    {
      APF_MEX_ERROR_EXACTLY_ONE_OUTPUT("'process'!\n"
          "And 'process' can only be used once in a chained comand");

      APF_MEX_ERROR_FURTHER_INPUT_NEEDED("'process'");

      APF_MEX_ERROR_SAME_NUMBER_OF_COLUMNS(_in_channels
          , "as number of sources");

      mwSize signal_length = static_cast<mwSize>(mxGetM(prhs[0]));
      if (signal_length % _block_size != 0 || signal_length == 0)
      {
        mexErrMsgTxt("Number of rows must be a non-zero, integer multiple of "
            "the block size!");
      }

      APF_MEX_ERROR_REAL_INPUT("Argument to 'process'");

#ifdef SSR_MEX_USE_DOUBLE
      if (!mxIsDouble(prhs[0]))
      {
        mexErrMsgTxt("This function only works with double precision data!");
      }
      plhs[0] = mxCreateDoubleMatrix(signal_length, _out_channels, mxREAL);
      sample_type* const output = mxGetPr(plhs[0]);
      sample_type* const input = mxGetPr(prhs[0]);
#else
      if (mxGetClassID(prhs[0]) != mxSINGLE_CLASS)
      {
        mexErrMsgTxt("This function only works with single precision data!");
      }
      plhs[0] = mxCreateNumericMatrix(signal_length, _out_channels
          , mxSINGLE_CLASS, mxREAL);
      sample_type* const output = static_cast<sample_type*>(mxGetData(plhs[0]));
      sample_type* const input = static_cast<sample_type*>(mxGetData(prhs[0]));
#endif

      for (mwSize offset = 0; offset < signal_length; offset+=_block_size)
      {
        for (int i = 0; i < _in_channels; ++i)
        {
          _inputs[i] = input + offset + i*signal_length;
        }

        for (int i = 0; i < _out_channels; ++i)
        {
          _outputs[i] = output + offset + i*signal_length;
        }

        _engine->audio_callback(_block_size, _inputs.data(), _outputs.data());
      }

      --nlhs; ++plhs;
      --nrhs; ++prhs;
    }

    void _loudspeaker_command(const std::string& command
        ,int& nlhs, mxArray**& plhs, int& nrhs, const mxArray**& prhs)
    {
      _loudspeaker_helper(command, nlhs, plhs, nrhs, prhs
          // Dummy argument to distinguish loudspeaker-based renderers:
          , static_cast<Renderer*>(nullptr));
    }

    void _loudspeaker_helper(const std::string& command
        , int&, mxArray**&, int&, const mxArray**&
        , ssr::RendererBase<Renderer>*)
    {
        std::string msg(std::string(_engine->name())
            + " does not support " + command);
        mexErrMsgTxt(msg.c_str());
    }

    void _loudspeaker_helper(const std::string& command
        , int& nlhs, mxArray**& plhs, int& nrhs, const mxArray**& prhs
        , ssr::LoudspeakerRenderer<Renderer>*)
    {
      _error_init();
      APF_MEX_ERROR_NO_FURTHER_INPUTS(command);
      APF_MEX_ERROR_ONE_OPTIONAL_OUTPUT(command);

      std::vector<Loudspeaker> ls_list;
      _engine->get_loudspeakers(ls_list);

      if (command == "loudspeaker_position")
      {
        plhs[0] = mxCreateDoubleMatrix(2, ls_list.size(), mxREAL);
        double* output = mxGetPr(plhs[0]);

        for (const auto& ls: ls_list)
        {
          output[0] = ls.position.x;
          output[1] = ls.position.y;
          output += 2;
        }
      }
      else
      {
        plhs[0] = mxCreateDoubleMatrix(1, ls_list.size(), mxREAL);
        double* output = mxGetPr(plhs[0]);

        for (const auto& ls: ls_list)
        {
          output[0] = ls.orientation.azimuth;
          output++;
        }
      }
    }

    void _source_position(int& nrhs, const mxArray**& prhs)
    {
      APF_MEX_ERROR_FURTHER_INPUT_NEEDED("'source_position'");
      APF_MEX_ERROR_REAL_INPUT("Source positions");
      APF_MEX_ERROR_SAME_NUMBER_OF_COLUMNS(_in_channels
          , "as number of sources");

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

      for (mwSize i = 0; i < _in_channels; ++i)
      {
        // TODO: handle 3D coordinates

        auto* source = _engine->get_source(i + 1);
        // TODO: check if source == nullptr
        source->position = Position(coordinates[i*2], coordinates[i*2+1]);
      }
    }

    void _source_orientation(int& nrhs, const mxArray**& prhs)
    {
      APF_MEX_ERROR_FURTHER_INPUT_NEEDED("'source_orientation'");
      APF_MEX_ERROR_REAL_INPUT("Source orientations");
      APF_MEX_ERROR_SAME_NUMBER_OF_COLUMNS(_in_channels
          , "as number of sources!");

      if (mxGetM(prhs[0]) != 1)
      {
        mexErrMsgTxt("Last argument must be a row vector of angles!");
      }

      double* angles = mxGetPr(prhs[0]);

      --nrhs; ++prhs;

      for (mwSize i = 0; i < _in_channels; ++i)
      {
        auto* source = _engine->get_source(i + 1);
        // TODO: check if source == nullptr
        source->orientation = Orientation(angles[i]);  // degree
      }
    }

    void _source_mute(int& nrhs, const mxArray**& prhs)
    {
      APF_MEX_ERROR_FURTHER_INPUT_NEEDED("'source_mute'");
      APF_MEX_ERROR_SAME_NUMBER_OF_COLUMNS(_in_channels
          , "as number of sources!");

      if (!mxIsLogical(prhs[0]))
      {
        mexErrMsgTxt("Argument after 'source_mute' must be of logical type!");
      }
      if (mxGetM(prhs[0]) != 1)
      {
        mexErrMsgTxt("Argument after 'source_mute' must be a row vector!");
      }

      mxLogical* mute = mxGetLogicals(prhs[0]);

      --nrhs; ++prhs;

      for (mwSize i = 0; i < _in_channels; ++i)
      {
        auto* source = _engine->get_source(i + 1);
        source->mute = mute[i];  // logical
      }
    }

    void _source_model(int& nrhs, const mxArray**& prhs)
    {
      APF_MEX_ERROR_FURTHER_INPUT_NEEDED("'source_model'");

      // list for converting cell array
      std::vector<std::string> model_list;

      apf::mex::next_arg(nrhs, prhs, model_list
          , "Argument after 'source_model' must be a cell array of strings!");

      if (static_cast<mwSize>(model_list.size()) != _in_channels)
      {
        mexErrMsgTxt("Number of elements in cell array after 'source_model' "
          "must be the same as number of sources!");
      }

      for (int i = 0; i < _in_channels; ++i)
      {
        Source::model_t model = Source::unknown;
        if (!apf::str::S2A(model_list[i], model))
        {
          mexPrintf("Model string '%s':", model_list[i].c_str());
          mexErrMsgTxt("Couldn't convert source model string!");
        }
        _engine->get_source(i + 1)->model = model;
      }
    }

    void _reference_position(int& nrhs, const mxArray**& prhs)
    {
      APF_MEX_ERROR_FURTHER_INPUT_NEEDED("'reference_position'");
      APF_MEX_ERROR_REAL_INPUT("Reference position");

      if (mxGetN(prhs[0]) != 1)
      {
        mexErrMsgTxt("Number of columns must be 1");
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

      _engine->state.reference_position =
        Position(coordinates[0], coordinates[1]);
    }

    void _reference_orientation(int& nrhs, const mxArray**& prhs)
    {
      APF_MEX_ERROR_FURTHER_INPUT_NEEDED("'reference_orientation'");
      APF_MEX_ERROR_REAL_INPUT("Reference orientation");

      if (mxGetN(prhs[0]) != 1 || mxGetM(prhs[0]) != 1)
      {
        mexErrMsgTxt("Last argument must be a scalar");
      }

      double* angle = mxGetPr(prhs[0]);

      --nrhs; ++prhs;

      _engine->state.reference_orientation = Orientation(*angle);
    }

    std::unique_ptr<Renderer> _engine;
    mwSize _in_channels, _out_channels, _block_size;
    std::vector<sample_type*> _inputs, _outputs;
};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
