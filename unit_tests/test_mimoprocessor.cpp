/******************************************************************************
 * Copyright © 2012-2013 Institut für Nachrichtentechnik, Universität Rostock *
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

// Tests for MimoProcessor.

#include "apf/mimoprocessor.h"

#include "catch/catch.hpp"

#include "apf/pointer_policy.h"
#include "apf/dummy_thread_policy.h"

struct DummyProcessor : public apf::MimoProcessor<DummyProcessor
                        , apf::pointer_policy<float*>
                        , apf::dummy_thread_policy>
{
  DummyProcessor(const apf::parameter_map& p)
    : apf::MimoProcessor<DummyProcessor, apf::pointer_policy<float*>
      , apf::dummy_thread_policy>(p)
  {}

  void process() {}
};

TEST_CASE("MimoProcessor", "Test MimoProcessor")
{

SECTION("compilation", "does it compile?")
{
  apf::parameter_map p;
  // some strange values:
  p.set("sample_rate", 1000);
  p.set("block_size", 33);
  DummyProcessor dummy(p);
}

// TODO: more tests!

} // TEST_CASE MimoProcessor

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
