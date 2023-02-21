/******************************************************************************
 * Copyright © 2019 SSR Contributors                                          *
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
/// Main function used by all renderers.

#ifndef SSR_MAIN_H
#define SSR_MAIN_H

#include <csignal>
#include <iostream>

#include "controller.h"

extern "C" inline void signal_handler(int signal)
{
  std::cerr << "\nInterrupted by signal " << signal << std::endl;
  std::exit(EXIT_FAILURE);
}
 
namespace ssr
{

template<typename Renderer>
int main(int argc, char* argv[])
{
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);
  try
  {
    SSR_VERBOSE3("STARTING SSR");
    // NB: This is static to be cleaned up automatically when exit() is called.
    static ssr::Controller<Renderer> controller{argc, argv};
    controller.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cerr << "Unknown error" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

}  // namespace ssr

#endif
