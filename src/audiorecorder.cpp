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
/// Audio recorder using ecasound (implementation).

#include <unistd.h>  // for usleep()

#include "audiorecorder.h"
#include "ssr_global.h"
#include "posixpathtools.h"

/** _.
 * @param audio_file_name name of audio file which will store the recording.
 * @param format_string ecasound formatstring consisting of
 * "sample_format,channels,sample_rate", for example "16,4,48000" (16 bit, 4
 * channels, 48000 Hz sampling rate).
 * @param record_source JACK client whose output will be recorded. All channels
 * will be connected up to the given number of channels. Further channels in the
 * output client will be ignored.
 * If no @a record_source is given or if it is an empty string, no connections
 * are made.
 * @warning Connections are made in the order of port-creation, numeric
 * portnames like out_1, out_2, etc. are not relevant. If you don't like this
 * behaviour, omit the optional parameter @a record_source and make your
 * connections manually.
 * @warning @a record_source must already be connected to JACK @b and its
 * outputs must already be registered in order to be able to make automatic
 * connections.
 * @param input_prefix_ The input prefix is the thing between the colon (:) and
 * the underscore (_) followed by a number, e.g. alsa_pcm:playback_1, where
 * "playback" is the input prefix.
 * @throw audiorecorder_error
 **/
AudioRecorder::AudioRecorder(const std::string& audio_file_name,
    const std::string& format_string, const std::string& record_source,
    const std::string& client_name_,  const std::string& input_prefix_)
    throw (audiorecorder_error) :
  client_name(client_name_),
  input_prefix(input_prefix_)
{
  _eca.command("cs-add audiorecorder_chainsetup");
  _eca.command("cs-set-audio-format " + format_string);
  _eca.command("c-add recorder_chain");
  // TODO: check if audiofile already exists. If it exists, audio data is added
  // TODO: to the end. This is not wanted!
  _eca.command("ao-add "
      + posixpathtools::get_escaped_filename(audio_file_name));
  if (record_source == "")
  {
    // do not make any connections
    _eca.command("ai-add jack_generic," + this->input_prefix);
  }
  else
  {
    // make connections with record_source automatically
    _eca.command("ai-add jack_auto," + record_source);
  }
  // use JACK transport (only receive), and set client name
  _eca.command("-G:jack," + this->client_name + ",recv");
  VERBOSE_NOLF("AudioRecorder ('" + this->client_name
      + "'): Trying to activate ... ");
  _eca.command("cs-connect");
  if (_eca.error())
  {
    VERBOSE("failed!");
    ERROR("File must be writable and needs an extension recognized by "
        "ecasound, e.g. \".wav\".");
    throw audiorecorder_error("ecasound: " + _eca.last_error());
  }
  VERBOSE("done.");

  if (!this->enable())
  {
    throw audiorecorder_error("Couldn't enable AudioRecorder \""
        + this->client_name + "\"!");
  }

  // It takes a little time until the client is available
  // This is a little ugly, but I don't know a better way to do it.
  // If you know one, tell me, please!
  usleep(ssr::usleeptime);
}

/// disconnects from ecasound
AudioRecorder::~AudioRecorder()
{
  _eca.command("cs-disconnect"); // implies "stop" and "engine-halt"
  VERBOSE2("AudioRecorder dtor ('" + client_name + "').");
}

bool AudioRecorder::enable()
{
  _eca.command("engine-launch");
  if (_eca.error())
  {
    ERROR("ecasound: " + _eca.last_error());
    return false;
  }
  return true;
}

bool AudioRecorder::disable()
{
  _eca.command("engine-halt");
  if (_eca.error())
  {
    ERROR("ecasound: " + _eca.last_error());
    return false;
  }
  return true;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
