Compiling and running the SSR
=============================

Mac OSX App-Bundle
------------------

The following sections are relevant if you want to build the SSR from
its source code. This is the default procedure for GNU/Linux systems. If
you want to use the SSR on a Mac, you can also use the pre-compiled app
bundle. For further information visit the SSR website .

Getting the Source
------------------

If you didn't receive this manual with the source code of the SSR, you
can download it from the SSR website . After downloading, you can unpack
the tarball with the command ``tar xvzf ssr-x.x.x.tar.gz`` in a shell.
This will extract the source code to a directory of the form
``ssr-x.x.x`` where "x" stands for the version numbers. ``cd`` to this
directory and proceed with section :ref:`Configuring <configuring>` to configure the
SSR.

.. _configuring:

Configuring
-----------

To build the SSR from source you have to configure first. Open a shell
and ``cd`` to the directory containing the source code of the package
and type:

::

    ./configure

This script will check your system for dependencies and prepare the
``Makefile`` required for compilation. Section :ref:`Dependencies <dependencies>` lists
the dependencies that must be installed on your system. The
``configure`` script will signal if dependencies are missing. At
successful termination of the ``configure`` script a summary will show
up.

Section :ref:`Hints Configuration <hints_conf>` is intended to help you
troubleshooting.

.. _dependencies:

Dependencies
~~~~~~~~~~~~

At least the following software (libraries and headers) including their development packages (*dev* or *devel*), where
available, are required for a full installation of the SSR:

-  JACK Audio Connection Kit

-  FFTW3 compiled for single precision (``fftw3f``) version 3.0 or
   higher

-  libsndfile

-  Ecasound

-  Trolltech's Qt 4.2.2 or higher with OpenGL (QtCore, QtGui and
   QtOpenGL)

-  libxml2

-  Boost.Asio , included since Boost version 1.35.

We provide a simple integration of several head tracking systems. Please
read section :ref:`Head Tracking <head_tracking>` for further informations
about head tracking.

.. _hints_conf:

Hints on Configuration
~~~~~~~~~~~~~~~~~~~~~~

If you encounter problems configuring the SSR these hints could help:

-  Ensure that you really installed all libraries (``lib``) with
   devel-package (``devel`` or ``dev``, where available) mentioned in
   section :ref:`Dependencies <dependencies>`.

-  It may be necessary to run ``ldconfig`` after installing new
   libraries.

-  Ensure that ``/etc/ld.so.conf`` or ``LD_LIBRARY_PATH`` are set
   properly, and run ``ldconfig`` after changes.

-  If a header is not installed in the standard paths of your system you
   can pass its location to the configure script using
   ``./configure CPPFLAGS=-Iyourpath``.

Note that with ``./configure --help`` all configure-options are
displayed, e.g. in section "Optional Features" you will find how to
disable compilation of the head trackers and many other things. Setting
the influential environment variables with ``./configure VARNAME=value``
can be useful for debugging dependencies.

.. _comp_inst:

Compiling and Installing
------------------------

If the configure script terminates with success, it creates a file named
``Makefile``. You can build the SSR by typing ::

    make
    make install

This will compile the SSR and install it to your system.

Uninstalling
------------

If the SSR didn't meet your expectations, we are very sorry, and of
course you can easily remove it from your system with ::

    make uninstall


.. _running_ssr:

Running the SSR
---------------

Before you start the SSR, start JACK , e.g. by typing
``jackd -d alsa -r 44100`` in a shell or using the graphical user
interface "qjackctl" . Now, the easiest way to get a signal out of the
SSR is by passing a sound-file directly::

    ssr YOUR_AUDIO_FILE

By default, the SSR starts with the binaural renderer; please use
headphones for listening with this renderer. Type ``ssr --help`` to get
an overview of the command line options and various renderers::

    USAGE: ssr [OPTIONS] <scene-file>

    The SoundScape Renderer (SSR) is a tool for real-time spatial audio reproduction
    providing a variety of rendering algorithms.

    OPTIONS:

    Choose a rendering algorithm:
        --binaural         Binaural (using HRIRs)
        --brs              Binaural Room Synthesis (using BRIRs)
        --wfs              Wave Field Synthesis
        --aap              Ambisonics Amplitude Panning
        --vbap             Stereophonic (Vector Base Amplitude Panning)
        --generic          Generic Renderer
        --nfc-hoa          Near-field-corrected Higher Order Ambisonics (experimental!)

    Renderer-specific options:
        --hrirs=FILE       Load the HRIRs for binaural renderer from FILE
        --hrir-size=VALUE  Maximum IR length (binaural and BRS renderer)
        --prefilter=FILE   Load WFS prefilter from FILE
    -o, --ambisonics-order=VALUE Ambisonics order to use (default: maximum)
        --in-phase-rendering     Use in-phase rendering for Ambisonics

    JACK options:
    -n, --name=NAME        Set JACK client name to NAME
        --input-prefix=PREFIX    Input  port prefix (default: "system:capture_")
        --output-prefix=PREFIX   Output port prefix (default: "system:playback_")
    -f, --freewheel        Use JACK in freewheeling mode

    General options:
    -c, --config=FILE      Read configuration from FILE
    -s, --setup=FILE       Load reproduction setup from FILE
        --threads=N        Number of audio threads (default N=1)
    -r, --record=FILE      Record the audio output of the renderer to FILE
        --loop             Loop all audio files
        --master-volume-correction=VALUE
                           Correction of the master volume in dB (default: 0 dB)
    -i, --ip-server[=PORT] Start IP server (default on)
                           A port can be specified: --ip-server=5555
    -I, --no-ip-server     Don't start IP server
    -g, --gui              Start GUI (default)
    -G, --no-gui           Don't start GUI
    -t, --tracker=TYPE     Start tracker, possible value(s): polhemus vrpn razor
        --tracker-port=PORT
                           A serial port can be specified, e.g. /dev/ttyS1
    -T, --no-tracker       Don't start tracker

    -h, --help             Show this very help information. You just typed that!
    -v, --verbose          Increase verbosity level (up to -vvv)
    -V, --version          Show version information and exit

Choose the appropriate arguments and make sure that your amplifiers are
not turned too loud…

To stop the SSR use either the options provided by the GUI (section
:ref:`GUI <gui>`) or type ``Crtl+c`` in the shell in which you started the SSR.

Keyboard actions in non-GUI mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you start SSR without GUI (option ``--no-gui``), it starts
automatically replaying the scene you have loaded. You can have some
interaction via the shell. Currently implemented actions are (all
followed by ``Return``):

-  ``c``: calibrate tracker (if available)

-  ``p``: start playback

-  ``q``: quit application

-  ``r``: "rewind"; go back to the beginning of the current scene

-  ``s``: stop (pause) playback

Note that in non-GUI mode, audio processing is always taking place. Live
inputs are processed even if you pause playback.

Recording the SSR output
~~~~~~~~~~~~~~~~~~~~~~~~

You can record the audio output of the SSR using the ``--record=FILE``
command line option. All output signals (i.e. the loudspeaker signals)
will be recorded to a multichannel wav-file named ``FILE``. The order of
channels corresponds to the order of loudspeakers specifed in the
reproduction setup (see sections :ref:`Reproduction Setups <reproduction_setups>` and
:ref:`ASDF <asdf>`). The recording can then be used to analyze the SSR output or
to replay it without the SSR using a software player like  "ecaplay".

.. _ssr_configuration_file:

Configuration File
------------------

The general configuration of the SSR (if GUI is enabled, which tracker
to use etc.) can be specified in a configuration file (e.g.
``ssr.conf``). By specifying your settings in such a file, you avoid
having to give explicit command line options every time you start the
SSR. We have added the example ``data/ssr.conf.example`` which mentions
all possible parameters. Take a look inside, it is rather
self-explanatory. There are three possibilities to specify a
configuration file:

-  put it in ``/etc/ssr.conf``

-  put it in your home directory in ``$HOME/.ssr/ssr.conf``

-  specify it on the command line with ``ssr -c my_config.file``

We explicitly mention one parameter here which might be of immediate
interest for you: ``MASTER_VOLUME_CORRECTION``. This a correction in
dB (!) which is applied -- as you might guess -- to the master volume. The
motivation is to have means to adopt the general perceived loudness of
the reproduction of a given system. Factors like the distance of the
loudspeakers to the listener or the typical distance of virtual sound
sources influence the resulting loudness which can be adjusted to the
desired level by means of the ``MASTER_VOLUME_CORRECTION``. Of course,
there's also a command line alternative (``--master-volume-correction``).

.. _head_tracking:

Head Tracking
-------------

We provide integration of the *InterSense InertiaCube3* tracking sensor
and the *Polhemus Fastrak*. They are used to update the orientation of
the reference (in binaural reproduction this is the listener) in
real-time. Please read sections :ref:`Preparing Intersense <prep_isense>` and :ref:`Preparing Polhemus <prp_pol>` if you want to compile the SSR with the support
for these trackers.

Note that on startup, the SSR tries to find the tracker. If it fails, it
continues without it. If you use a tracker, make sure that you have the
appropriate rights to read from the respective port.

You can calibrate the tracker while the SSR is running by pressing
``Return``. The instantaneous orientation will then be interpreted as
straight forward (:math:`\alpha = 90^\circ`\ ).

.. _prep_isense:

Preparing InterSense InertiaCube3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you want to compile the SSR with support for the *InterSense
InertiaCube3* tracking sensor , please download the *InterSense Software
Development Kit* (SDK) from the InterSense website . Unpack the archive
and place the files

-  ``isense.h`` and ``types.h`` to ``/usr/local/include``, and

-  ``libisense.so`` (the version appropriate for your processor type) to
   ``usr/local/lib``.

The SSR ``configuration`` script will automatically detect the presence
of the files described above and if they are found, enable the
compilation for the support of this tracker. To disable this tracker,
use ``./configure --disable-intersense`` and recompile.

If you encounter an error-message similar to
``libisense.so: cannot open shared object file: No such file or directory``,
but the file is placed correctly, run ``ldconfig``.

.. _prp_pol:

Preparing Polhemus Fastrack
~~~~~~~~~~~~~~~~~~~~~~~~~~~

For incorporation of the *Polhemus Fastrack*
with serial connection, no additional libraries are required. If you
want to disable this tracker, use ``./configure --disable-polhemus``
and recompile.

Using the SSR with DAWs
-----------------------

As stated before, the SSR is currently not able to dynamically replay
audio files (refer to section :ref:`ASDF <asdf>`). If your audio scenes are
complex, you might want to consider using the SSR together with a
digital audio work station (DAW). To do so, you simply have to create as
many sources in the SSR as you have audio tracks in your respective DAW
project and assign live inputs to the sources. Amongst the ASDF examples
we provide at  you find an example scene description which does exactly
this.

DAWs like Ardour  support JACK and their use is therefore
straightforward. DAWs which do not run on Linux or do not support JACK
can be connected via the input of the sound card.

In the future we will provide a VST plug-in which will allow you to
dynamically operate all virtual source's properties (like e.g. a
source's position or level etc.). You will then be able to have the full
SSR functionality controlled from your DAW.
