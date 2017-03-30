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
/// Command line parsing, and config-file related stuff (definition).

#ifndef SSR_CONFIGURATION_H
#define SSR_CONFIGURATION_H

#include <string>

#include "apf/parameter_map.h"

namespace ssr
{

/// program settings from config-file and command line
struct conf_struct
{
  apf::parameter_map renderer_params;
  std::string exec_name;                ///< name of executable (without path)
  float stand_ampl_ref_dist;            ///< distance where plane sources are
                                        ///< as loud as the other source types
  bool gui;                             ///< start GUI?
  bool ip_server;                       ///< start IP server?
  std::string tracker;                  ///< type of head tracker (or "")
  std::string tracker_ports;            ///< space-separated serial ports
  bool freewheeling;                    ///< use JACK's freewheeling mode?
  std::string scene_file_name;          ///< scene file to load
  //std::string playback_setup_file_name; ///< reproduction setup to load
  std::string xml_schema;               ///< schema file to validate XML files
  std::string audio_recorder_file_name; ///< output file for audio recorder
  std::string input_port_prefix;        ///< e.g. "alsa_pcm:capture"
  std::string output_port_prefix;       ///< e.g. "alsa_pcm:playback"
  std::string path_to_gui_images;       ///< dto.
  std::string path_to_scene_menu;       ///< path to scene_menu.conf
  int end_of_message_character;         ///< ASCII
  bool auto_rotate_sources;             ///< Automatic orientation of sources

  int server_port;                      ///< listening port
  /// size of delay line (in samples)
  int wfs_delayline_size;
  /// maximum negative delay (in samples, wfs_initial_delay >= 0)
  int wfs_initial_delay;

  //int hrir_size;
  //std::string hrir_file_name;
  int ambisonics_order;
  bool in_phase_rendering;

  bool loop; ///< temporary solution for looping sound files
};

conf_struct configuration(int& argc, char* argv[]);

// static int parse(const char *line, char *key, char *value);
// static int is_comment_or_empty(const char *line);
int load_config_file(const char *filename, conf_struct& conf);

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
