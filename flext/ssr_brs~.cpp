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

// BRS renderer as Puredata/Max external.

#include "ssr_flext.h"
#include "brsrenderer.h"

template<>
class SsrFlext<ssr::BrsRenderer> : public SsrFlextBase<ssr::BrsRenderer>
{
  public:
    SsrFlext(int argc, const t_atom* argv)
      : SsrFlextBase<ssr::BrsRenderer>(_get_parameters(argc, argv))
    {
      if (_engine.params.has_key("threads"))
      {
        --argc;
        ++argv;
      }

      while (argc > 0)
      {
        if (!IsSymbol(*argv))
        {
          throw std::invalid_argument(
              "Expected string argument (BRIR file name)!");
        }
        std::string file_name = _get(argc, argv);
        apf::parameter_map params;
        params.set("properties-file", _canvasdir + "/" + file_name);
        auto id = _engine.add_source("", params);
        _engine.get_source(id)->active = true;
        _source_ids.push_back(id);
        AddInSignal();
      }
      if (auto sources = _engine.get_input_list().size(); sources > 0)
      {
        post("%s - created %d sources", thisName(), sources);
      }
      else
      {
          throw std::invalid_argument(
              "At least one BRIR file name must be given!");
      }
      _init();
    }

  private:
    apf::parameter_map _get_parameters(int argc, const t_atom* argv)
    {
      int threads = 0;

      // Note: IsInt(*argv) doesn't work in Pd
      if (IsFloat(*argv))
      {
        if (!_get(argc, argv, threads))
        {
          throw std::invalid_argument("First argument must be an integer!");
        }
        if (threads < 0)
        {
          throw std::invalid_argument("A negative number of threads ... "
              "how is this supposed to work?");
        }
      }
      else
      {
        // do nothing, all other arguments are checked later
      }
      apf::parameter_map params;
      if (threads)
      {
        params.set("threads", threads);
      }
      return params;
    }
};

SSR_FLEXT_INSTANCE(brs, ssr::BrsRenderer)
