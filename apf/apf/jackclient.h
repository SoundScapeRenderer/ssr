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
/// JACK client (C++ wrapper for JACK).

#ifndef APF_JACKCLIENT_H
#define APF_JACKCLIENT_H

#include <jack/jack.h>
#include <jack/thread.h>
#include <jack/types.h>
#include <string>
#include <vector>
#include <utility>    // for std::pair
#include <stdexcept>  // for std::runtime_error
#include <cassert>    // for assert()
#include <errno.h>    // for EEXIST

#ifdef APF_JACKCLIENT_DEBUG
#include <iostream>
#include <jack/statistics.h>  // for jack_get_xrun_delayed_usecs()
#define APF_JACKCLIENT_DEBUG_MSG(str) \
  do { std::cout << "apf::JackClient: " << str << std::endl; } while (false)
#else
#define APF_JACKCLIENT_DEBUG_MSG(str) do { } while (false)
#endif

namespace apf
{

/** C++ wrapper for a JACK client.
 * @warning When several JACK clients are running and one of them is closed,
 * this can lead to a segmentation fault in the callback function
 * (jack_port_get_buffer() delivers bad data). We couldn't really track down the
 * error, but to be on the sure side, delete your JackClients only on the very
 * end.
 * \par
 * A related issue may be that after calling jack_client_close() the
 * corresponding thread is not closed, in the end it becomes a "zombie thread".
 * But maybe this is a different story ...
 * \par
 * Any comments on this topic are very welcome!
 **/
class JackClient
{
  public:
    typedef jack_default_audio_sample_t sample_t;
    typedef jack_nframes_t              nframes_t;
    typedef jack_port_t                 port_t;

    /// Select if JACK's audio callback function shall be called
    enum callback_usage_t
    {
      /// JACK audio callback is never called
      dont_use_jack_process_callback = 0,
      /// JACK audio callback (jack_process_callback()) is called after activate()
      use_jack_process_callback
    };

    /// exception to be thrown at various occasions.
    struct jack_error : std::runtime_error
    {
      jack_error(const std::string& s)
        : std::runtime_error("JackClient: " + s)
      {}

      jack_error(jack_status_t s)
        : std::runtime_error(std::string("JackClient: ")
        + ((s & JackInvalidOption)
                ? "The operation contained an invalid or unsupported option!"
        : (s & JackShmFailure) ? "Unable to access shared memory!"
        : (s & JackVersionError) ? "Client's protocol version does not match!"
        : (s & JackClientZombie) ? "Zombie!"
        : (s & JackNoSuchClient) ? "Requested client does not exist!"
        : (s & JackLoadFailure) ? "Unable to load internal client!"
        : (s & JackInitFailure) ? "Unable to initialize client!"
        : (s & JackBackendError) ? "Backend error!"
        : (s & JackNameNotUnique & JackFailure)
                ? "The client name is not unique!"
        : (s & JackServerFailed) ? "Unable to connect to the JACK server!"
        : (s & JackServerError) ? "Communication error with the JACK server!"
        : (s & JackFailure) ? "Overall operation failed!"
        : "Unknown error!"))
      {}
    };

    explicit inline JackClient(const std::string& name = "JackClient"
        , callback_usage_t callback_usage = dont_use_jack_process_callback);

    virtual ~JackClient()
    {
      if (_client)
      {
        // this->deactivate() has to be called in the derived dtor or earlier!
        jack_client_close(_client);
        // return value is ignored, because we're shutting down anyway ...
      }
    }

    /// Activate JACK client.
    /// @return @b true on success
    /// @see jack_activate()
    bool activate() const
    {
      if (!_client || jack_activate(_client)) return false;

      this->connect_pending_connections();

      // TODO: Still pending connections are ignored!

      return true;
    }

    /// Deactivate JACK client.
    /// @return @b true on success
    /// @see jack_deactivate()
    bool deactivate() const
    {
      APF_JACKCLIENT_DEBUG_MSG("pending connections @ deactivate(): "
          << _pending_connections.size());
      return _client ? !jack_deactivate(_client) : false;
    }

    /// @name manage JACK ports
    //@{

    /// Register JACK input port.
    /// @param name desired port name
    /// @return JACK port
    /// @see register_port()
    port_t* register_in_port(const std::string& name) const
    {
      return this->register_port(name, JackPortIsInput);
    }

    /// Register JACK output port.
    /// @param name desired port name
    /// @return JACK port
    /// @see register_port()
    port_t* register_out_port(const std::string& name) const
    {
      return this->register_port(name, JackPortIsOutput);
    }

    /// Register JACK port (input or output).
    /// @param name desired port name
    /// @param flags JACK port flags
    /// @return JACK port
    /// @see jack_port_register()
    port_t* register_port(const std::string& name, unsigned long flags) const
    {
      return jack_port_register(_client, name.c_str(), JACK_DEFAULT_AUDIO_TYPE
          , flags, 0);
    }

    /// Unregister JACK port.
    /// @param port JACK port
    /// @return @b true on success
    /// @see jack_port_unregister()
    bool unregister_port(port_t* port) const
    {
      if (!jack_port_unregister(_client, port))
      {
        port = nullptr; // port still points somewhere. Set to NULL.
        return true;
      }
      else
      {
        return false;
      }
    }

    /// Connect two JACK ports.
    /// @param source source port name
    /// @param destination destination port name
    /// @return @b true on success
    bool connect_ports(const std::string& source
        , const std::string& destination) const
    {
      return _connect_ports_helper(source, destination, _pending_connections);
    }

    /// Disconnect two JACK ports.
    /// @param source source port name
    /// @param destination destination port name
    /// @return @b true on success
    bool disconnect_ports(const std::string& source
        , const std::string& destination) const
    {
      return !jack_disconnect(_client, source.c_str(), destination.c_str());
    }

    /// Make connections which are still pending from a previous
    /// call to connect_ports(). This is needed if connect_ports() has been
    /// called while the JackClient wasn't activated yet.
    bool connect_pending_connections() const
    {
      _pending_connections_t still_pending_connections;

      APF_JACKCLIENT_DEBUG_MSG("Connecting " << _pending_connections.size()
          << " pending connections ...");
      while (_pending_connections.size() > 0)
      {
        _connect_ports_helper(_pending_connections.back().first
            , _pending_connections.back().second, still_pending_connections);
        _pending_connections.pop_back();
      }
      APF_JACKCLIENT_DEBUG_MSG("Still pending connections: "
          << still_pending_connections.size());

      _pending_connections.swap(still_pending_connections);
      return _pending_connections.empty();
    }

    //@}  // manage JACK ports

    /// @name manage JACK transport
    //@{

    /// Start JACK transport
    void transport_start() const
    {
      if (_client) jack_transport_start(_client);
    }

    /// Stop JACK transport
    void transport_stop() const
    {
      if (_client) jack_transport_stop(_client);
    }

    /// Set JACK transport location.
    /// @param frame location
    /// @return @b true on success
    /// @see get_transport_state(), jack_transport_locate()
    bool transport_locate(nframes_t frame) const
    {
      return _client ? !jack_transport_locate(_client, frame) : false;
    }

    /// Get JACK transport state.
    /// @return a pair: first element is @b true if transport is rolling,
    ///   second is the current position.
    std::pair<bool, nframes_t> get_transport_state() const
    {
      std::pair<bool, nframes_t> result(false, 0);

      if (_client)
      {
        jack_position_t jp;
        jack_transport_state_t jts = jack_transport_query(_client, &jp);
        result.first  = (jts == JackTransportRolling);
        result.second =  jp.frame;
      }
      return result;
    }

    //@}

    /// Set JACK freewheeling mode.
    /// @param onoff non-zero: start; zero: stop
    /// @return @b true on success
    bool set_freewheel(int onoff) const
    {
      return _client ? !jack_set_freewheel(_client, onoff) : false;
    }

    /// @return JACK client name
    std::string client_name() const { return _client_name; }
    /// @return JACK sample rate
    nframes_t   sample_rate() const { return _sample_rate; }
    /// @return JACK buffer size
    nframes_t   buffer_size() const { return _buffer_size; }

    bool is_realtime() const
    {
      return _client ? jack_is_realtime(_client) : false;
    }

    int get_real_time_priority() const
    {
      return _client ? jack_client_real_time_priority(_client) : -1;
    }

    float get_cpu_load() const
    {
      return _client ? jack_cpu_load(_client) : 100.0f;
    }

    pthread_t client_thread_id() const
    {
      return _client ? jack_client_thread_id(_client) : 0;
    }

#ifdef APF_DOXYGEN_HACK
  protected:
#else
  private:
#endif

    /// @name callback functions
    /// can be overwritten in derived classes
    //@{

    /// JACK process callback function.
    /// This function is empty in the JackClient base class. Derived classes
    /// should overwrite it if needed.
    /// @param nframes Number of frames (~samples) in the current block. This
    ///   value is delivered by the JACK server.
    /// @return message to JACK: 0 means call me again, 1 don't call me anymore.
    /// @throw jack_error if not implemented
    /// @see callback_usage_t
    virtual int jack_process_callback(nframes_t nframes)
    {
      (void)nframes;  // avoid "unused parameter" warning
      throw jack_error("jack_process_callback() not implemented!");
    }

    /// JACK shutdown callback.
    /// By default, this is throwing a jack_error exception. If you don't like
    /// this, you can overwrite this function in your derived class.
    /// @param code status code, see JackInfoShutdownCallback
    /// @param reason a string describing the shutdown reason
    /// @see JackInfoShutdownCallback and jack_on_info_shutdown()
    /// @note There is also JackShutdownCallback and jack_on_shutdown(), but
    ///   this one is more useful.
    virtual void jack_shutdown_callback(jack_status_t code, const char* reason)
    {
      (void)code;  // avoid "unused parameter" warning
      throw jack_error("JACK shutdown! Reason: " + std::string(reason));
    }

    /// JACK sample rate callback.
    /// @param sr new sample rate delivered by JACK
    /// @throw jack_error if not implemented
    /// @return 0 on success.
    virtual int jack_sample_rate_callback(nframes_t sr)
    {
      (void)sr;
      throw jack_error("Sample rate changes are not supported!");
    }

    /// JACK buffer size callback.
    /// @throw jack_error if not implemented
    /// @return 0 on success.
    virtual int jack_buffer_size_callback(nframes_t bs)
    {
      (void)bs;
      throw jack_error("Buffer size changes are not supported!");
    }

    /// JACK xrun callback.
    /// @return zero on success, non-zero on error
    virtual int jack_xrun_callback()
    {
      APF_JACKCLIENT_DEBUG_MSG("JACK server reports xrun of "
        << jack_get_xrun_delayed_usecs(_client)/1000.0f << " msecs.");
      return 0;
    }

    //@}

  private:
    // Internal redirection functions for callbacks
    // Map void pointers to class instances and call member functions

    static int _jack_process_callback(nframes_t nframes, void* arg)
    {
      return static_cast<JackClient*>(arg)->jack_process_callback(nframes);
    }

    static void _jack_shutdown_callback(jack_status_t code
        , const char* reason, void* arg)
    {
      static_cast<JackClient*>(arg)->jack_shutdown_callback(code, reason);
    }

    static int _jack_sample_rate_callback(nframes_t sr, void* arg)
    {
      return static_cast<JackClient*>(arg)->jack_sample_rate_callback(sr);
    }

    static int _jack_buffer_size_callback(nframes_t bs, void* arg)
    {
      return static_cast<JackClient*>(arg)->jack_buffer_size_callback(bs);
    }

    static int _jack_xrun_callback(void* arg)
    {
      return static_cast<JackClient*>(arg)->jack_xrun_callback();
    }

    typedef std::vector<std::pair<std::string, std::string>>
      _pending_connections_t;

    bool _connect_ports_helper(const std::string& source
        , const std::string& destination
        , _pending_connections_t& pending_connections) const
    {
      if (_client == nullptr) return false;
      int success = jack_connect(_client, source.c_str(), destination.c_str());
      switch (success)
      {
        case 0:
          break;
        case EEXIST:
          APF_JACKCLIENT_DEBUG_MSG("Connection already exists! ("
              << source << " -> " << destination << ")");
          break;
        default:
          APF_JACKCLIENT_DEBUG_MSG("Unable to connect "
              << source << " -> " << destination
              << "! Adding this to pending connections ...");
          pending_connections.push_back(std::make_pair(source, destination));
          // TODO: return something else than true/false?
          return false;
      }
      return true;
    }

    std::string    _client_name;  // Name of JACK client
    jack_client_t* _client;       // Pointer to JACK client.
    nframes_t      _sample_rate;  // sample rate of JACK server
    nframes_t      _buffer_size;  // buffer size of JACK server

    mutable _pending_connections_t _pending_connections;

    JackClient(const JackClient&);             // deactivated
    JackClient& operator=(const JackClient&);  // deactivated
};

/** Constructor.
 * @param name client name of the JACK client to be created.
 * @param callback_usage if @p use_jack_process_callback, the member
 * function jack_process_callback() is called by JACK in each audio cycle.
 * @warning @p name should not include a colon. This doesn't cause
 *   an error directly, but it messes up the JACK client- and portnames.
 * @throw jack_error if something goes wrong
 **/
JackClient::JackClient(const std::string& name
    , callback_usage_t callback_usage)
  : _client_name(name)
  , _client(nullptr) // will be set after connecting to JACK
  , _sample_rate(0)  //             -- " --
  , _buffer_size(0)  //             -- " --
{
  // check if client name is too long
  // jack_client_name_size() returns the size including the terminating \0
  // character, hence the ">=".
  if (name.size()
      >= static_cast<std::string::size_type>(jack_client_name_size()))
  {
    throw jack_error("Client name is too long! ('" + name + "')");
  }

  // TODO: make parameters for these options:
  //jack_options_t options = JackNoStartServer;
  jack_options_t options = JackUseExactName;
  //jack_options_t options = JackNullOption;

  // JackNoStartServer: see also JACK_NO_START_SERVER
  // TODO: JackServerName? needs extra parameter in jack_client_open()!

  jack_status_t status;
  _client = jack_client_open(name.c_str(), options, &status);
  if (!_client) throw jack_error(status);

  if (options & JackUseExactName)
  {
    assert(_client_name == jack_get_client_name(_client));
  }
  else
  {
    _client_name = jack_get_client_name(_client);
  }

  // TODO: error callback
  //jack_set_error_function(default_jack_error_callback);

  if (callback_usage == use_jack_process_callback)
  {
    if (jack_set_process_callback(_client, _jack_process_callback, this))
    {
      throw jack_error("Could not set process callback function for '"
          + _client_name + "'!");
    }
  }

  jack_on_info_shutdown(_client, _jack_shutdown_callback, this);

  if (jack_set_xrun_callback(_client, _jack_xrun_callback, this))
  {
    throw jack_error("Could not set xrun callback function for '"
        + _client_name + "'!");
  }

  // TODO: is the following still valid?
  // sometimes, jack_activate() returns successful although an error occured and
  // the thing "zombified". if the shutdown handler is called, _client is reset
  // to zero which we can check now:
  if (!_client)
  {
    throw jack_error("\"" + _client_name + "\" was killed somehow!");
  }

  _sample_rate = jack_get_sample_rate(_client);
  _buffer_size = jack_get_buffer_size(_client);
}

}  // namespace apf

#undef APF_JACKCLIENT_DEBUG_MSG

#endif
