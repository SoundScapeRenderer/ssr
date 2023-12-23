.. ****************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2014 Quality & Usability Lab,                             *
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
 ******************************************************************************

.. _running_ssr:

Running the SSR
===============


Starting the JACK Audio Server
------------------------------

Before you start the SSR, start JACK, e.g. by typing
``jackd -d alsa -r 44100`` in a shell or using the graphical user
interface ``qjackctl``.


SSR binaries
------------

After installing the SSR, each renderer (see :doc:`renderers`)
is available as a separate binary:
``ssr-binaural``,
``ssr-brs``,
``ssr-vbap``,
``ssr-wfs``,
``ssr-aap``,
``ssr-dca`` (the renderer formerly known as ``ssr-nfc-hoa``)
and
``ssr-generic``.

The following examples use ``ssr-binaural``,
but you can of course you any renderer you want!

Note that the SSR renderers are also available as externals for
`Pure Data <https://puredata.info/>`_. Refer to :ref:`SSR in Pure
Data<ssr_in_pure_data>`.


Loading a Single Audio File
---------------------------

The easiest way to get a signal out of the
SSR is by passing a sound-file directly::

    ssr-binaural YOUR_AUDIO_FILE

Make sure that your amplifiers are not turned too loud…

To stop the SSR use either the options provided by the GUI (Section
:ref:`gui`) or type ``Crtl+c`` in the shell in which you started the SSR.


Loading an Audio Scene File
---------------------------

You can also load :ref:`audio_scenes`::

    ssr-binaural YOUR_AUDIO_SCENE_FILE.asd


Command Line Options
--------------------

There are a lot of options that are available for all renderers
and only a few that are only available for certain renderers.

Type ``ssr-binaural --help`` to get
an overview of the command line options
(the help text is the same for all renderers):

.. code-block:: none

    Usage: ssr-binaural [OPTIONS] <scene-file>

    The SoundScape Renderer (SSR) is a tool for real-time spatial audio reproduction
    providing a variety of rendering algorithms.

    Options:

    Renderer-specific options:
          --hrirs=FILE    Load HRIRs for binaural renderer from FILE
          --hrir-size=N   Truncate HRIRs to length N
          --prefilter=FILE
                          Load WFS prefilter from FILE
      -o, --ambisonics-order=VALUE
                          Ambisonics order to use for AAP (default: maximum)
          --in-phase-rendering
                          Use in-phase rendering for AAP renderer

    JACK options:
      -n, --name=NAME     Set JACK client name to NAME
          --input-prefix=PREFIX
                          Input port prefix (default: "system:capture_")
          --output-prefix=PREFIX
                          Output port prefix (default: "system:playback_")
      -f, --freewheel     Use JACK in freewheeling mode

    General options:
      -c, --config=FILE   Read configuration from FILE
      -s, --setup=FILE    Load reproduction setup from FILE
          --threads=N     Number of audio threads (default: auto)
      -r, --record=FILE   Record the audio output of the renderer to FILE
          --decay-exponent=VALUE
                          Exponent that determines the amplitude decay (default: 1)
          --loop          Loop all audio files
          --master-volume-correction=VALUE
                          Correction of the master volume in dB (default: 0 dB)
          --auto-rotation Auto-rotate sound sources' orientation toward the
                          reference
          --no-auto-rotation
                          Don't auto-rotate sound sources' orientation toward the
                          reference
      -i, --ip-server[=PORT]
                          Start IP server (default off),
                          a port number can be specified (default 4711)
      -I, --no-ip-server  Don't start IP server (default)
          --end-of-message-character=VALUE
                          ASCII code for character to end messages with
                          (default 0 = binary zero)
          --websocket-server[=PORT]
                          Start WebSocket server (default on),
                          a port number can be specified (default 9422)
          --no-websocket-server
                          Don't start WebSocket server
          --fudi-server[=PORT]
                          Start FUDI server (default off),
                          a port number can be specified (default 1174)
          --no-fudi-server
                          Don't start FUDI server (default)
          --follow        Wait for another SSR instance to connect
          --no-follow     Don't follow another SSR instance (default)
      -g, --gui           Start GUI (default)
      -G, --no-gui        Don't start GUI
      -t, --tracker=TYPE  Select head tracker, possible value(s):
                          fastrak patriot vrpn intersense razor
          --tracker-port=PORT
                          Port name/number of head tracker, e.g. /dev/ttyS1
      -T, --no-tracker    Don't use a head tracker (default)

      -h, --help          Show help and exit
      -v, --verbose       Increase verbosity level (up to -vvv)
      -V, --version       Show version information and exit

Use ``$HOME`` to refer to your home directory in the case that SSR does not
resolve the tilde ``~``.


.. _ssr_configuration_file:

Configuration Files
-------------------

The general configuration of the SSR (whether GUI is enabled, which tracker
to use, and most other command line arguments) can be specified in a
configuration file (e.g.
``ssr.conf``). By specifying your settings in such a file, you avoid
having to give explicit command line options every time you start the
SSR. We have added the example
:download:`data/ssr.conf.example <../../data/ssr.conf.example>`,
which mentions
all possible parameters. Take a look inside, it is rather
self-explanatory.

Configuration files are loaded in the following order, if certain options are
specified more than once, the last occurrence counts. This means that it is
not the last file that is loaded that counts but rather the last occurrence at
which a given setting is specified.

1. ``/Library/SoundScapeRenderer/ssr.conf``
2. ``/etc/ssr.conf``
3. ``$HOME/Library/SoundScapeRenderer/ssr.conf``
4. ``$HOME/.ssr/ssr.conf``
5. the path(s) specified with the ``--config``/``-c`` option(s) (e.g.,
   ``ssr-binaural -c my_config.file``)

We explicitly mention one parameter here that might be of immediate
interest for you: ``MASTER_VOLUME_CORRECTION``. This a correction in
dB (!) that is applied -- as you might guess -- to the master volume. The
motivation is to have means to adopt the general perceived loudness of
the reproduction of a given system. Factors like the distance of the
loudspeakers to the listener or the typical distance of virtual sound
sources influence the resulting loudness, which can be adjusted to the
desired level by means of the ``MASTER_VOLUME_CORRECTION``. Of course,
there's also a command line alternative (``--master-volume-correction``).


Keyboard Actions in Non-GUI Mode
--------------------------------

If you start SSR without GUI (option ``--no-gui``), it starts
automatically replaying the scene that you have loaded. You can have some
interaction via the shell. Currently implemented actions are (all
followed by ``Return``):

-  ``c``: calibrate tracker (if available)

-  ``p``: start playback

-  ``q``: quit application

-  ``r``: "rewind"; go back to the beginning of the current scene

-  ``s``: stop (pause) playback

Note that in non-GUI mode, audio processing is always taking place. Live
inputs are processed even if you pause playback.


Recording the SSR Output
------------------------

You can record the audio output of the SSR using the
``--record=FILE`` command line option. All output signals
(i.e. the loudspeaker signals) will be recorded to a multichannel wav-file
named ``FILE``. The order of channels corresponds to the order of loudspeakers
specifed in the reproduction setup (see Sections
:ref:`Reproduction Setups <reproduction_setups>` and
:ref:`ASDF <asdf>`). The recording can then be used to analyze the SSR output or
to replay it without the SSR using a software player like ``ecaplay``
(http://nosignal.fi/ecasound/).


.. _head_tracking:

Head Tracking
-------------

We provide integration of the *InterSense InertiaCube3* tracking sensor,
the *Polhemus Fastrak* and the *Polhemus Patriot* as well as all trackers
supported by *VRPN*. The *Supperware* head tracker is supported indirectly via
the Pd patch ``pd/supperware_head_tracker_to_ssr.pd``. The head trackers are
used to update the orientation of the reference (in binaural reproduction this
is the listener) in real-time.

See :ref:`dependencies` for how to compile the SSR with head tracking support.

Note that on startup, the SSR tries to find the tracker. If it fails, it
continues without it. If you use a tracker, make sure that you have the
appropriate rights to read from the respective port.

You can calibrate the tracker while the SSR is running by pressing
``Return``. The instantaneous orientation will then be interpreted as
straight forward, i.e. upwards on the screen (:math:`\alpha = 90^\circ`\ ).

SSR is progressively (and silently) moving from 2D scenes to 3D scenes. The
:ref:`binaural renderer<binaural_renderer>` can handle head tracking about all
three axes of rotation if the HRIRs are provided in SOFA. We recommend using
the :ref:`browser-based GUI<browser-based_gui>` to monitor the tracking as the
built-in GUI only visualizes tracking along the azimuth.

.. _prep_isense:

Preparing InterSense InertiaCube3
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Make sure that you have the required access rights to the tracker before
starting SSR. For you are using the USB connection type ::

  sudo chmod a+rw /dev/ttyUSBX

whereby ``X`` can be any digit or number. If you are not sure which port is
the tracker then unplug the tracker, type ::

  ls /dev/ttyUSB*

replug the tracker, execute above command again and see which port was added.
That one is the tracker. It's likely that it is the one whose name contains
the highest number.

.. _prp_pol:

Preparing Polhemus Fastrak/Patriot
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Make sure that
you have the required access rights to the tracker before starting SSR by
typing something like ::

  sudo chmod a+rw /dev/ttyS0

or ::

  sudo chmod a+rw /dev/ttyS1

or so.

If you want to disable this tracker, use ``./configure --disable-polhemus``
and recompile.

If you are using head tracking about all three axes of rotation, make sure
that the tracking sensor is mounted on the headphones such that the cable
leaves the sensor towards the left relative to the look direction of the user.

Preparing VRPN
^^^^^^^^^^^^^^

In order to use *Virtual Reality Peripheral Network* (VRPN_) compatible
trackers create a config file ``vrpn.cfg`` with one of the following lines (or
similar)

.. _VRPN: http://www.cs.unc.edu/Research/vrpn/index.html

::

  vrpn_Tracker_Fastrak MyFastrak /dev/ttyUSB0 115200
  vrpn_Tracker_Fastrak MyOtherFastrak COM1 115200
  vrpn_Tracker_Liberty MyPatriot /dev/ttyUSB1 115200

... and start ``vrpn_server``. You can choose the name of the Tracker
arbitrarily. Then, start the SSR with the given Tracker name, e.g.::

  ssr-binaural --tracker=vrpn --tracker-port=MyFastrak@localhost

If the tracker runs on a different computer, use its hostname (or IP address)
instead of localhost. You can of course select your head tracker settings by
means of :ref:`Configuration Files<ssr_configuration_file>`.

Using the SSR with DAWs
-----------------------

As stated before, the SSR is currently not able to dynamically replay
audio files (refer to Section :ref:`ASDF <asdf>`). If your audio scenes are
complex, you might want to consider using the SSR together with a
digital audio work station (DAW). To do so, you simply have to create as
many sources in the SSR as you have audio tracks in your respective DAW
project and assign live inputs to the sources. Amongst the ASDF examples
we provide on SSR website http://spatialaudio.net/ssr/ you'll find an scene
description that does exactly this.

DAWs like Ardour (https://ardour.org) support JACK and their use is therefore
straightforward. DAWs which do not run on Linux or do not support JACK
can be connected via the input of the sound card.

In the future we will provide a VST plug-in which will allow you to
dynamically operate all virtual source's properties (like e.g. a
source's position or level etc.). You will then be able to have the full
SSR functionality controlled from your DAW.

Using the SSR with different audio clients
------------------------------------------

This page contains some short description how to connect your own audio files
with the SSR using different audio players.

VLC Media Player
^^^^^^^^^^^^^^^^

How to connect the SSR in binaural playback mode with the own audio library
using Jack and VLC Media Player:

After installing Jack and the SSR (with all needed components: see
:ref:`configuring`) it is necessary to install the VLC
Media Player with its Jack plugin (for example UBUNTU):

1. ``sudo apt-get install vlc vlc-plugin-jack``

    (or use the packet manager of your choice instead of the command line and
    install: vlc and vlc-plugin-jack)

2. After installing open VLC Media Player and navigate to Tools->Preferences
Select "All" on the bottom left corner In the appearing menu on the left
navigate to "Audio"->"Output Module" and extend it by using "+"

3. In the submenu of "Output Module" select "JACK" and replace "system" by "
Binaural-Renderer" in the "Connect to clients matching"-box. Do not forget to
enable "Automatically connect to writable clients" above. (Otherwise you have
to connect the audio output of vlc with the SSR input after every played audio
file using jack.)

  (*Note*: If you want to use another Renderer, e.g. for WFS, you have to
  enter "WFS-Renderer" in the box)

  .. figure:: images/screenshot_vlc.png
    :align: center

4. Save your changes.

5. Start everything together using the command line::

    qjackctl -s & vlc & ssr-binaural --gui /"path_of_your_scene_file(s)"/stereo.asd &

    This will start jack, vlc and the ssr with the GUI and a provided stereo
    scene (TODO: LINK) (stereo.asd)

6. Open an audio file in vlc and press play


Using a Head-Tracker
^^^^^^^^^^^^^^^^^^^^

Running with InterSense tracker support
_______________________________________

Due to copyright reasons, the SSR does not come with a built-in InterSense
tracker support. So first you have to build the SSR with InterSense
tracker support yourself (see the CI configuration file
:download:`.github/workflows/main.yml <../../.github/workflows/main.yml>`
for instructions).

If you are using a USB-to-Serial interface with your tracker, you need to
install drivers for that. This seems to work fine for the interface made by
InterSense: https://ftdichip.com/drivers/vcp-drivers/

To check if the system sees the tracker do::

    ls -l /dev/tty.usb*

On the MacBooks tested, the serial ports were called ``/dev/tty.usbserial-
00001004`` or ``/dev/tty.usbserial-00002006`` depending on which USB port was
used.

To make the SSR use the InterSense tracker with these ports, you have two
options:

Using the command line (only one port can be specified)::

    open -a SoundScapeRenderer --args --binaural "--tracker=intersense
    --tracker-port=/dev/tty.usbserial-XXXXXXXX"

... or using config files:

Add these lines to a config file (multiple ports can be specified)::

    TRACKER = intersense
    TRACKER_PORTS = /dev/tty.usbserial-XXXXXXXX /dev/tty.usbserial-YYYYYYYY

It's recommended to use the config file approach - best use a global :ref:`
config file<ssr_configuration_file>`.

Running with Razor AHRS tracker support
_______________________________________

If you happen not to own a Polhemus or InterSense tracker to do your head-
tracking, an alternative would be to use our DIY low-cost `Razor AHRS tracker`_.

.. _`Razor AHRS tracker`:
  https://github.com/Razor-AHRS/razor-9dof-ahrs/wiki/Tutorial

If you have Arduino installed on you machine, FTDI drivers will be there too.
Otherwise get the driver from https://ftdichip.com/drivers/vcp-drivers/.

To check if the system sees the tracker do::

    ls -l /dev/tty.usb*

This should give you something like ``/dev/tty.usbserial-A700eEhN``.

To make the SSR use this Razor AHRS tracker, you have two options:

Using the command line::

    open -a SoundScapeRenderer --args --binaural "--tracker=razor
    --tracker-port=/dev/tty.usbserial-XXXXXXXX"

... or using config files:

Add these lines to a config file::

    TRACKER = intersense
    TRACKER_PORTS = /dev/tty.usbserial-XXXXXXXX

It's recommended to use the config file approach - best use a global
:ref:`config file<ssr_configuration_file>`.
