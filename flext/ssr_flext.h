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
/// SSR renderers as Puredata/Max externals.

#ifndef SSR_FLEXT_H
#define SSR_FLEXT_H

#include <string>

#include <flext.h>

// check for appropriate flext version (CbSignal was introduced in 0.5.0)
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0!
#endif

#define APF_MIMOPROCESSOR_SAMPLE_TYPE t_sample

#define SSR_FLEXT_INSTANCE(name, renderer) \
class ssr_ ## name : public SsrFlext<renderer> { \
  using SsrFlext<renderer>::SsrFlext; \
  FLEXT_HEADER_S(ssr_ ## name, flext_dsp, setup) }; \
FLEXT_NEW_DSP_V("ssr_" #name "~", ssr_ ## name)

#include "apf/pointer_policy.h"
#include "apf/cxx_thread_policy.h"

#include "../src/source.h"

template<typename Renderer>
class SsrFlext : public flext_dsp
{
  FLEXT_HEADER_S(SsrFlext<Renderer>, flext_dsp, setup)

  private:
    // TODO: put these functions somewhere else?
    static bool _get(int& argc, const t_atom*& argv, int& out)
    {
      if (!IsFloat(*argv)) return false;
      int result = GetInt(*argv);
      if (result != GetFloat(*argv)) return false;
      out = result;
      --argc;
      ++argv;
      return true;
    }

    static bool _get(int& argc, const t_atom*& argv, float& out)
    {
      if (!IsFloat(*argv)) return false;
      out = GetFloat(*argv);
      --argc;
      ++argv;
      return true;
    }

    static bool _get(int& argc, const t_atom*& argv, bool& out)
    {
      if (!CanbeBool(*argv)) return false;
      out = GetBool(*argv);
      --argc;
      ++argv;
      return true;
    }

    static const char* _get(int& argc, const t_atom*& argv)
    {
      const char* result = "";
      if (!IsSymbol(*argv)) return result;
      result = GetString(*argv);
      --argc;
      ++argv;
      return result;
    }

    apf::parameter_map _get_parameters(int argc, const t_atom* argv)
    {
      _in_channels = 0;
      int threads = 0;
      std::string first_string = "", second_string = "";

      while (argc > 0)
      {
        // Note: IsInt(*argv) doesn't work in Pd
        if (IsFloat(*argv))
        {
          int arg;
          if (!_get(argc, argv, arg))
          {
            throw std::invalid_argument("Numeric arguments must be integers!");
          }
          if (!_in_channels)
          {
            if (arg < 0)
            {
              throw std::invalid_argument("A negative number of sources ... "
                  "how is this supposed to work?");
            }
            _in_channels = arg;
          }
          else if (!threads)
          {
            if (arg < 0)
            {
              throw std::invalid_argument("A negative number of threads ... "
                  "how is this supposed to work?");
            }
            threads = arg;
          }
          else
          {
            throw std::invalid_argument("Too many numeric arguments!");
          }
        }
        else if (IsSymbol(*argv))
        {
          if (first_string == "")
          {
            first_string = _get(argc, argv);
          }
          else if (second_string == "")
          {
            second_string = _get(argc, argv);
          }
          else
          {
            throw std::invalid_argument("Too many string arguments!");
          }
        }
        else
        {
          throw std::invalid_argument("Unsupported argument type!");
        }
      }

      if (!_in_channels)
      {
        post("%s - first numeric argument should specify number of sources! "
            "None given, creating one source by default ...", thisName());
        _in_channels = 1;
      }

      if (first_string == "")
      {
        throw std::invalid_argument(
            "At least one string must be specified as argument!");
      }

      apf::parameter_map params;

      params.set("reproduction_setup", first_string);
      params.set("hrir_file", first_string);

      // TODO: At some point, "prefilter_file" should become part of the
      // configuration file for the reproduction setup.
      // As soon as this happens, "second_string" should be removed!

      if (second_string != "")
      {
        params.set("prefilter_file", second_string);
      }

      if (threads)
      {
        params.set("threads", threads);
      }

      params.set("block_size", Blocksize());
      params.set("sample_rate", Samplerate());

      // TODO: let the user choose those values:
      params.set("delayline_size", 100000);  // in samples
      params.set("initial_delay", 1000);  // in samples

      std::string info;
      for (auto it: params)
      {
        info += "\n";
        info += " * ";
        info += it.first;
        info += ": ";
        info += it.second;
      }
      post("%s - trying to start with following options:\n * sources: %d%s"
          , thisName(), _in_channels, info.c_str());

      return params;
    }

  public:
    SsrFlext(int argc, const t_atom* argv)
      : _engine(_get_parameters(argc, argv))
    {
      _engine.load_reproduction_setup();

      for (size_t i = 0; i < _in_channels; ++i)
      {
        _engine.add_source();
        AddInSignal();
      }

      AddOutSignal(_engine.get_output_list().size());
      _engine.activate();  // start parallel processing (if threads > 1)
      post("%s - initialization of %s completed, %d outputs available"
          , thisName(), _engine.name(), CntOut());
    }

    static void setup(t_classid c)
    {
      FLEXT_CADDMETHOD(c, 0, _handle_messages);

      post("Thanks for using the SoundScape Renderer (SSR)!");
      post("For more information, visit http://spatialaudio.net/ssr/");
    }

  private:
    virtual void CbSignal()
    {
      _engine.audio_callback(Blocksize(), InSig(), OutSig());
    }

    FLEXT_CALLBACK_A(_handle_messages)
    void _handle_messages(const t_symbol* s, int argc, const t_atom* argv)
    {
      std::string cmd1 = GetString(s);

      if (cmd1 == "src")
      {
        if (argc < 1)
        {
          error("%s - too few arguments for %s", thisName(), cmd1.c_str());
          return;
        }

        int src_id;
        if (!_get(argc, argv, src_id))
        {
          error("%s - src expects integer source ID (starting with 1)"
              , thisName());
          return;
        }

        auto* source = _engine.get_source(src_id);

        if (!source)
        {
          error("%s - couldn't find source %d", thisName(), src_id);
          return;
        }

        std::string cmd2 = _get(argc, argv);
        if (cmd2 == "")
        {
          error("%s - %s %d must be followed by a string!"
              , thisName(), cmd1.c_str(), src_id);
          return;
        }
        else if (cmd2 == "pos")
        {
          if (argc != 2)
          {
            error("%s - src %d pos must be followed by exactly 2 coordinates!"
                , thisName(), src_id);
            return;
          }
          float x, y;
          if (!_get(argc, argv, x))
          {
            error("%s - x must be a float value!", thisName());
            return;
          }
          if (!_get(argc, argv, y))
          {
            error("%s - y must be a float value!", thisName());
            return;
          }
          source->position = Position(x, y);
        }
        else if (cmd2 == "azi")
        {
          if (argc != 1)
          {
            error("%s - src %d azi must be followed by exactly 1 angle!"
                , thisName(), src_id);
            return;
          }
          float azi;
          if (!_get(argc, argv, azi))
          {
            error("%s - src azi expects a float value!", thisName());
            return;
          }
          source->orientation = Orientation(azi);
        }
        else if (cmd2 == "gain")
        {
          if (argc != 1)
          {
            error("%s - src %d gain must be followed by exactly 1 value!"
                , thisName(), src_id);
            return;
          }
          float gain;
          if (!_get(argc, argv, gain))
          {
            error("%s - src %d gain expects a float value!"
                , thisName(), src_id);
            return;
          }
          source->gain = gain;
        }
        else if (cmd2 == "mute")
        {
          if (argc != 1)
          {
            error("%s - src %d mute must be followed by exactly 1 argument!"
                , thisName(), src_id);
            return;
          }
          bool mute;
          if (!_get(argc, argv, mute))
          {
            error("%s - src mute expects a boolean value!", thisName());
            return;
          }
          source->mute = mute;
        }
        else if (cmd2 == "model")
        {
          if (argc != 1)
          {
            error("%s - src %d model must be followed by exactly 1 argument!"
                , thisName(), src_id);
            return;
          }
          std::string model_str = _get(argc, argv);
          if (model_str == "")
          {
            error("%s - src model expects a string value!", thisName());
            return;
          }
          Source::model_t model = Source::unknown;
          if (!apf::str::S2A(model_str, model))
          {
            error("%s - couldn't convert model string: %s"
                , thisName(), model_str.c_str());
            return;
          }
          source->model = model;
        }
        else
        {
          error("%s - unknown command: src %d %s", thisName()
              , src_id, cmd2.c_str());
          return;
        }
      }
      else if (cmd1 == "ref")
      {
        if (argc < 1)
        {
          error("%s - too few arguments for %s", thisName(), cmd1.c_str());
          return;
        }

        bool offset = false;
        std::string cmd2 = _get(argc, argv);
        if (cmd2 == "offset")
        {
          offset = true;
          cmd2 = _get(argc, argv);
        }

        const char* offset_str = offset ? " offset" : "";

        if (cmd2 == "")
        {
          error("%s - ref%s must be followed by a string!"
              , thisName(), offset_str);
          return;
        }
        else if (cmd2 == "pos")
        {
          if (argc != 2)
          {
            error("%s - ref%s pos must be followed by exactly 2 coordinates!"
                , thisName(), offset_str);
            return;
          }
          float x, y;
          if (!_get(argc, argv, x))
          {
            error("%s - x must be a float value!", thisName());
            return;
          }
          if (!_get(argc, argv, y))
          {
            error("%s - y must be a float value!", thisName());
            return;
          }
          if (offset)
          {
            _engine.state.reference_offset_position = Position(x, y);
          }
          else
          {
            _engine.state.reference_position = Position(x, y);
          }
        }
        else if (cmd2 == "azi")
        {
          if (argc != 1)
          {
            error("%s - ref%s azi must be followed by exactly 1 angle!"
                , thisName(), offset_str);
            return;
          }
          float azi;
          if (!_get(argc, argv, azi))
          {
            error("%s - ref%s azi expects a float value!"
                , thisName(), offset_str);
            return;
          }
          if (offset)
          {
            _engine.state.reference_offset_orientation = Orientation(azi);
          }
          else
          {
            _engine.state.reference_orientation = Orientation(azi);
          }
        }
        else
        {
          error("%s - invalid string for ref%s: %s"
              , thisName(), offset_str, cmd2.c_str());
          return;
        }
      }
      else if (cmd1 == "vol")
      {
        if (argc != 1)
        {
          error("%s - vol must be followed by exactly 1 value (in dB)!"
              , thisName());
          return;
        }
        float master_volume;
        if (!_get(argc, argv, master_volume))
        {
          error("%s - vol expects a float value!", thisName());
          return;
        }
        _engine.state.master_volume = master_volume;
      }
      else if (cmd1 == "processing")
      {
        if (argc != 1)
        {
          error("%s - processing must be followed by exactly 1 value!"
              , thisName());
          return;
        }
        bool processing;
        if (!_get(argc, argv, processing))
        {
          error("%s - processing expects a boolean value!", thisName());
          return;
        }
        _engine.state.processing = processing;
      }
      else
      {
        // TODO: amplitude_reference_distance?

        error("%s - unknown command: %s", thisName(), cmd1.c_str());
        return;
      }
    }

    int _in_channels;
    Renderer _engine;
};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
