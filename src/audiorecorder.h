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
/// Audio recorder using ecasound (definition).

#ifndef SSR_AUDIORECORDER_H
#define SSR_AUDIORECORDER_H

#include <eca-control-interface.h>
#include <string>
#include <memory>
#include <stdexcept> // for std::runtime_error

/**
 * Writes a (multichannel) soundfile using ecasound and JACK transport.
 **/
class AudioRecorder
{
  public:
    using ptr_t = std::unique_ptr<AudioRecorder>; ///< auto_ptr to AudioRecorder

    /// exception to be thrown by ctor.
    struct audiorecorder_error : public std::runtime_error
    {
      audiorecorder_error(const std::string& s): std::runtime_error(s) {}
    };

    ~AudioRecorder();

    AudioRecorder(const std::string& audio_file_name,
        const std::string& format_string, const std::string& record_source,
        const std::string& client_name = "recorder",
        const std::string& input_prefix = "channel");

    bool enable();   ///< enable recording as soon as transport is started
    bool disable();  ///< disable recording

    const std::string client_name;   ///< name of JACK client used by ecasound
    const std::string input_prefix;  ///< prefix used for channels

  private:
    ECA_CONTROL_INTERFACE _eca; ///< interface to ecasound
};

#endif
