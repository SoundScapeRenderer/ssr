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
/// A simple stopwatch

// TODO: check C++11 timing tools:
// http://bstamour.ca/2012/05/13/timing-functions-with-chrono/
// http://solarianprogrammer.com/2012/10/14/cpp-11-timing-code-performance/
// http://ideone.com/clone/SCRI6
// http://kjellkod.wordpress.com/2012/02/06/exploring-c11-part-1-time/

#include <ctime>
#include <iostream>

namespace apf
{

/// A simple stopwatch
class StopWatch
{
  public:
    StopWatch(const std::string& name = "this activity") :
      _start(std::clock()),
      _name(name)
    {}

    ~StopWatch()
    {
      clock_t total = std::clock() - _start;
      std::cout << _name << " took " << double(total)/CLOCKS_PER_SEC
        << " seconds." << std::endl;
    }

  private:
    std::clock_t _start;
    std::string _name;
};

}  // namespace apf

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
