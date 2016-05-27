/******************************************************************************
 Copyright (c) 2012-2016 Institut für Nachrichtentechnik, Universität Rostock
 Copyright (c) 2006-2012 Quality & Usability Lab
                         Deutsche Telekom Laboratories, TU Berlin

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*******************************************************************************/

// https://AudioProcessingFramework.github.io/

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
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
