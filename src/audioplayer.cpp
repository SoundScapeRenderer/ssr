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
/// Audio player using ecasound (implementation).

#include <algorithm>
#include <thread>   // std::this_thread::sleep_for
#include <chrono>   // std::chrono::microseconds
#include <jack/jack.h> // for jack_client_name_size()

#include "audioplayer.h"
#include "maptools.h"
#include "ssr_global.h"
#include "apf/stringtools.h"
#include "posixpathtools.h"

using maptools::get_item;

/// delete the file map.
AudioPlayer::~AudioPlayer()
{
  if (!_file_map.empty())
  {
    maptools::purge(_file_map);
    VERBOSE2("AudioPlayer dtor: file map deleted.");
  }
  else
  {
    VERBOSE2("AudioPlayer dtor.");
  }
}

/** _.
 * If the file is already opened, an existing instance of ecasound is used.
 * @param audio_file_name name of the audio file, what did you think?
 * @param channel select a channel of a multichannel file (starting with 1)
 * @param loop temporary solution for a loop mode
 * @return Name of the JACK port.
 * @warning If @a audio_file_name uses symbolic links or such things it can
 * happen that one file is opened several times.
 **/
std::string AudioPlayer::get_port_name(const std::string& audio_file_name,
    int channel, bool loop)
{
  assert(channel >= 0);

  auto registered_file = get_item(_file_map, audio_file_name);
  if (registered_file != nullptr)
  {
    VERBOSE2("AudioPlayer: Input file '" + audio_file_name
        + "' already registered.");
    if (channel > registered_file->get_channels())
    {
      ERROR("AudioPlayer: Channel " << channel << " doesn't exist in '"
          + audio_file_name + "'!");
      return "";
    }
  }
  else // file not yet registered
  {
    auto temp = Soundfile::create(audio_file_name, loop);
    if (!temp)
    {
      WARNING("AudioPlayer: Initialization of soundfile '" + audio_file_name
          + "' failed!");
      return "";
    }
    if (channel > temp->get_channels())
    {
      ERROR("AudioPlayer: Channel " << channel << " doesn't exist in '"
          + audio_file_name + "'!");
      // if wrong channel is requested, audiofile is not registered.
      return "";
    }
    registered_file = temp.get();
    _file_map[audio_file_name] = temp.release();
  }
  return registered_file->get_client_name() + ":"
    + registered_file->output_prefix + "_" + apf::str::A2S(channel);
}

/** _.
 * @param audio_file_name the audio file you want to know the length of.
 * @warning If the file wasn't loaded before, 0 is returned!
 **/
long int AudioPlayer::get_file_length(const std::string& audio_file_name) const
{
  const Soundfile* const file = get_item(_file_map, audio_file_name);
  return file ? file->get_length() : 0;
}

std::string
AudioPlayer::Soundfile::get_format(const std::string& filename
    , size_t& channels, size_t& sample_rate)
{
  ECA_CONTROL_INTERFACE eca;
  eca.command("cs-add dummy_chainsetup");
  eca.command("c-add dummy_chain");

  eca.command("ai-add sndfile,"
      + posixpathtools::get_escaped_filename(filename));
  eca.command("ao-add null");
  eca.command("cs-connect");
  if (eca.error())
  {
    throw soundfile_error("get_format(): " + eca.last_error());
  }
  eca.command("ai-index-select 1");
  eca.command("ai-get-format");
  std::string str = eca.last_string();
  eca.command("cs-disconnect");
  eca.command("c-remove");
  eca.command("cs-remove");

  std::replace(str.begin(), str.end(), ',', ' ');
  std::istringstream iss(str);
  std::string format;
  iss >> format >> channels >> sample_rate;
  if (iss.fail())
  {
    throw soundfile_error("Couldn't convert format string!");
  }
  assert(sample_rate >= 1);

  return format;
}

size_t
AudioPlayer::Soundfile::_get_jack_sample_rate()
{
  ECA_CONTROL_INTERFACE eca;
  eca.command("cs-add dummy_chainsetup");
  eca.command("c-add dummy_chain");

  eca.command("ai-add jack");
  eca.command("ao-add null");
  eca.command("cs-connect");
  if (eca.error())
  {
    throw soundfile_error("_get_jack_sample_rate(): " + eca.last_error());
  }
  eca.command("ai-get-format");
  std::string str = eca.last_string();
  eca.command("cs-disconnect");
  eca.command("c-remove");
  eca.command("cs-remove");

  std::replace(str.begin(), str.end(), ',', ' ');
  std::istringstream iss(str);
  std::string format;
  size_t channels, sample_rate;
  iss >> format >> channels >> sample_rate;
  if (iss.fail())
  {
    throw soundfile_error("Couldn't convert string for getting sample rate!");
  }
  assert(sample_rate >= 1);

  return sample_rate;
}

/** ctor.
 * @param filename name of soundfile
 * @param loop enable loop mode
 * @param prefix prefix used for channel names
 * @throw soundfile_error
 * @attention The soundfile must have the same sampling rate as the JACK server.
 * @warning If the name of the soundfile (including path) is longer than the
 * maximum allowed JACK client-name, it gets truncated. But if another file is
 * truncated to the same client-name, ecasound probably wants to add "_2" to
 * distinguish them but this will then be longer than the allowed length.
 * \par
 * UPDATE: This is already taken care of, but only up to "_99". So if you have
 * more than 99 equal client-names, you will get a problem. Hopefully, you
 * don't have that many ...
 **/
AudioPlayer::Soundfile::Soundfile(const std::string& filename, bool loop,
    const std::string& prefix) :
  output_prefix(prefix),
  _filename(filename),
  _client_name(""),
  _channels(0)
{
  _sample_format = get_format(_filename, _channels, _sample_rate);
  size_t jack_sample_rate = _get_jack_sample_rate();

  _eca.command("cs-add real_chainsetup");
  _eca.command("c-add real_chain");
  _eca.command("cs-set-audio-format ,"
      + apf::str::A2S(_channels) + "," + apf::str::A2S(jack_sample_rate));

  std::string ai_add = "ai-add ";
  if (loop)
  {
    ai_add += "audioloop,";
  }

  if (_sample_rate != jack_sample_rate)
  {
    WARNING("'" + _filename
        + "' has a different sample rate than JACK! ("
        + apf::str::A2S(_sample_rate) + " vs. "
        + apf::str::A2S(jack_sample_rate) + ")");
    ai_add += "resample-hq,auto,";
  }

  ai_add += "sndfile,";
  ai_add += posixpathtools::get_escaped_filename(filename);

  _eca.command(ai_add);
  _eca.command("ao-add jack_generic," + this->output_prefix);

  // check if filename is too long for a JACK portname.
  // if yes, truncate filename (keep the end)
  int max_size = jack_client_name_size();  // #include <jack/jack.h>
#ifdef __APPLE__
  // TODO this is a workaround, and might not work on every system or with every version of jack.
  // on OS X jack_client_name_size() returns 64, but 52 seems to be the max supported size.
  //_client_name = "a123456789a123456789a123456789a123456789a123456789a"; // length = 51 + \0 = 52
  max_size = 31;
#endif
  max_size--; // max_size includes the terminating \0 character!
  max_size -= 3; // to allow ecasound to append a number up to "_99"
  max_size--; // we will add a special character at the beginning (maybe '['?)
  _client_name = _filename;
  assert(max_size >= 0);
  if (_filename.size() > static_cast<size_t>(max_size))
  {
    _client_name = _filename.substr(_filename.size() - max_size);
    // to visualize the truncation
    _client_name[0] = '<';
  }
  // to group the inputs in an alphabetic list of clients (e.g. in qjackctl)
  _client_name.insert(0,"["); // see max_size-- above!
  std::replace(_client_name.begin(), _client_name.end(), '/', '_');

  // set the name of the JACK client (using the truncated audio-filename)
  // do not send, only receive transport msg.
  // this must be done AFTER adding a chain
  _eca.command("-G:jack," + _client_name + ",recv");

  _eca.command("cs-connect");
  if (_eca.error())
  {
    throw soundfile_error("AudioPlayer::Soundfile: " + _eca.last_error());
  }

  // after cs-connect, get information about the input file:
  //_eca.command("ai-get-length");
  //_length = _eca.last_float();
  _eca.command("ai-get-length-samples");
  _length_samples = _eca.last_long_integer();

  _eca.command("engine-launch");
  if (_eca.error())
  {
    _eca.command("cs-disconnect");
    throw soundfile_error("AudioPlayer::Soundfile: " + _eca.last_error());
  }
  // It takes a little time until the client is available
  // This is a little ugly, but I don't know a better way to do it.
  // If you know one, tell me, please!
  std::this_thread::sleep_for(std::chrono::microseconds(ssr::usleeptime));
  VERBOSE2("Added '" + _filename
      + "', format: '" + apf::str::A2S(_sample_format)
      + "', channels: " + apf::str::A2S(_channels)
      + ", sample rate: " + apf::str::A2S(_sample_rate) + ".");
}

/// disconnects from ecasound.
AudioPlayer::Soundfile::~Soundfile()
{
  // TODO: check if ecasound is really running.
  _eca.command("cs-disconnect"); // implies "stop" and "engine-halt"
  VERBOSE2("AudioPlayer::Soundfile: '" + _filename + "' disconnected.");
}

AudioPlayer::Soundfile::ptr_t AudioPlayer::Soundfile::create(
    const std::string& filename, bool loop)
{
  ptr_t temp; // temp = NULL
  try
  {
    temp.reset(new Soundfile(filename, loop));
  }
  catch(soundfile_error& e)
  {
    ERROR(e.what());
  }
  return temp;
}

int AudioPlayer::Soundfile::get_channels() const
{
  return _channels;
}

std::string AudioPlayer::Soundfile::get_client_name() const
{
  return _client_name;
}

long int AudioPlayer::Soundfile::get_length() const
{
  return _length_samples;
}
