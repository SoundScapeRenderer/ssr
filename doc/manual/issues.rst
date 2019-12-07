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

.. _issues:

Issues
------

Also visit https://github.com/SoundScapeRenderer/ssr/wiki/Known-Issues for
updated known issues.

``make dmg`` on Mac OS X chokes on symbolic links
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

On some file system (e.g. network shares with ACLs) you might get an error
like this::

  ... copy helper ... copy error (canceling): /Volumes/SoundScape Renderer ...
  ... Operation not supported ... could not access /Volumes/...
  hdiutil: create failed - Operation not supported
  make[1]: *** [dmg] Error 1
  make: *** [dmg] Error 2

This has something to do with symbolic links and the way how ``hdiutil``
handles them. If you get this error, just try to compile the SSR from a
different location. You can do this by either moving all the source files
somewhere else, or by doing something like this::

  cd /tmp
  mkdir ssr-bundle
  cd ssr-bundle
  export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
  $PATH_TO_SSR_SOURCE/configure --enable-app-bundle
  make
  make dmg

In this example, ``$PATH_TO_SSR_SOURCE`` is the directory where you put the
SSR source files. Instead of ``/tmp`` you can of course use something else,
but with ``/tmp`` it should work on most systems out there.

If you don't like this work-around, you may also play around with ``fsaclctl``.
Only ``WAVE_FORMAT_PCM`` and ``WAVE_FORMAT_IEEE_FLOAT`` are supported.

Multi-channel WAV files would normally use the format
``WAVE_FORMAT_EXTENSIBLE``, see
http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html.

However, Ecasound doesn't know this format, that's why we have to use one of
the above mentioned formats, although for files with more than 2 channels this
is not compliant to the WAV standard.

To check the exact format of your WAV files, you can use sndfile-info (Debian
package sndfile-programs), and to convert your files, you can use, for
example, sox (Debian package sox) with the wavpcm option::

  sox old.wav new.wavpcm
  mv new.wavpcm new.wav

SSR crashes with a segmentation fault on Max OS X
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If this happens whenever you are opening an audio file or loading a scene that involves opening an audio file, then this might be due to Ecasound. We've seen this with the app bundle. Try the following:

Download the Ecasound source code from http://nosignal.fi/ecasound/download.php. ``Cd`` into the folder and compile Ecasound with::

  ./configure
  make

Refer also to :ref:`ecasound` for instructions on how to compile Ecasound. The executable ``ecasound/ecasound`` will be created.

Finally, replace the Ecasound executable in the SSR bundle with something like this::

  sudo cp ecasound/ecasound /Applications/SoundScapeRenderer-0.4.2-74-gb99f8b2/SoundScapeRenderer.app/Contents/MacOS/

You might have to modify the name of the SSR folder in the above command as you're likely to use a different version.

A file that can't be loaded results in a connection to a live input
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If there is an error in loading a specified audio file, the corresponding
source is still created and (unexpectedly) connected to the first soundcard
input channel.

We believe this is a bug in the JACK system related to the API function
jack_connect()_: If the ``destination_port`` argument is an empty string, the
function returns (correctly) with an error. However, if the ``source_port``
argument is an empty string, the port is connected to the first "system" port (
or the first port at all, or who knows ...). And this is the case if the SSR
cannot open a specified audio file.

.. _jack_connect():
  http://jackaudio.org/files/docs/html/group__PortFunctions.html

If you also think that's a bug, feel free to report it to the JACK developers.

Conflicting JACK and Ecasound versions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is a problem due to a special combination of ecasound and JACK versions
on 64 bit systems leading to an error (terminating the SSR) similar to this::

  (ecasoundc_sa) Error='read() error', cmd='cs-connect' last_error='' cmd_cnt=6
 last_cnt=5.

We experienced this error on 64 bit systems with ecasound version 2.4.6.1 and 2
.7.0 in combination with JACK version 0.118.0. A similar error occured on Mac
OS X with ecasound version 2.7.2 and JackOSX version 0.89 (with Jackdmp 1.9.8).

Please try to update to the newest ecasound and JACK versions.

Annoying Ecasound message
~~~~~~~~~~~~~~~~~~~~~~~~~

You may have seen this message::

  ***********************************************************************
  * Message from libecasoundc:
  *
  * 'ECASOUND' environment variable not set. Using the default value
  * value 'ECASOUND=ecasound'.
  ***********************************************************************

You can totally ignore this, but if it bothers you, you can disable it easily
by specifying the following line in ``/etc/bash.bashrc`` (system-wide setting)
or, if you prefer, you can put it into your ``$HOME/.bashrc``
(just for your user account)::

  export ECASOUND=ecasound

.. _ecasound_cannot_open_a_jack_port:

Ecasound cannot open a JACK port
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sometimes, when Ecasound is installed via Homebrew_, it can have trouble finding JACK. As
a result SSR displays the sound source symbols in the GUI, but they don't play audio, and
an according error message is posted in the SSR terminal.

Type ``ecasound -c`` in a terminal to start Ecasound in interactive mode.
Then type ``aio-register`` to list all available outputs that Ecasound has recognized. If
JACK is not listed, then download the Ecasound source code from
http://nosignal.fi/ecasound/download.php, and ::

  ./configure --enable-jack
  make
  make install

The last line might have to be ::

  sudo make install

Refer also to :ref:`ecasound` for instructions on how to compile Ecasound.

Using SSR on Mac OS X El Capitan
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SSR works well on El Capitan. JACK is what can cause headache. See `JACK on Mac OS X`_ .

Long paths to audio files on Mac OS X
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It can happen that SSR displays this error message when loading audio files directily::

  Error: AudioPlayer::Soundfile: ERROR:  Connecting chainsetup failed: "Enabling chainsetup: AUDIOIO-JACK: Unable to open JACK-client" (audioplayer.cpp:310)
  Warning: AudioPlayer: Initialization of soundfile '/Users/YOUR_USERNAME/Documents/audio/YOUR_AUDIO_FILE.wav' failed! (audioplayer.cpp:87)

Opening such a file would result in a JACK port name that is too long. You can resolve
this limitation by moving the audio file to a location that produces a shorter (full) path
name or by wrapping the audio file in an asd-file.

Segmentation Fault when Opening a Scene
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This problem occured on some old SuSE systems.

When you start the SSR with GUI, everything is alright at first. As soon as
you open a scene, a segmentation fault arises. This is a problem in the
interaction between Qt and OpenGL. As a workaround, comment the line ::

  renderText(0.18f * scale, 0.13f * scale, 0.0f, source->name.c_str(), f);

in the file ``src/gui/qopenglrenderer.cpp`` and recompile the code. The
consequence is that the names of the sound sources will not be displayed
anymore.

Choppy Sound on Cheap (On-Board) Sound Cards
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sometimes JACK doesn't play well with those on-board sound cards. One
possibility to improve this, is to increase the frames/period setting from the
default 2 to a more generous 3. This can be done in the Settings dialog of
qjackctl or with the command line option ``-n``::

  jackd -n 3

``dylibbundler`` doesn't grok Qt Frameworks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If ``make dmg`` doesn't copy the Qt ``.dylib`` files into the application
bundle (to ``Contents/Libraries``), you might try the following commands (or
similar, depending on the exact Qt installation).

.. raw:: latex

    Go to the \href{http://ssr.rtfd.org/en/latest/operation.html#dylibbundler-doesn-t-grok-qt-frameworks}
    {online manual} to copy and paste them.

::

  install_name_tool -id /opt/local/lib/libQtCore.dylib /opt/local/Library/Frameworks/QtCore.framework/QtCore
  install_name_tool -id /opt/local/lib/libQtGui.dylib /opt/local/Library/Frameworks/QtGui.framework/QtGui
  install_name_tool -change /opt/local/Library/Frameworks/QtCore.framework/Versions/5/QtCore /opt/local/lib/libQtCore.dylib /opt/local/Library/Frameworks/QtGui.framework/QtGui
  install_name_tool -id /opt/local/lib/libQtOpenGL.dylib /opt/local/Library/Frameworks/QtOpenGL.framework/QtOpenGL
  install_name_tool -change /opt/local/Library/Frameworks/QtCore.framework/Versions/5/QtCore /opt/local/lib/libQtCore.dylib /opt/local/Library/Frameworks/QtOpenGL.framework/QtOpenGL
  install_name_tool -change /opt/local/Library/Frameworks/QtGui.framework/Versions/5/QtGui /opt/local/lib/libQtGui.dylib /opt/local/Library/Frameworks/QtOpenGL.framework/QtOpenGL

You need the appropriate rights to change the library files, so you probably
need to use ``sudo`` before the commands.

*WARNING*: You can totally ruin your Qt installation with this stuff!

To get some information about a library, you can try something like those::

  otool -L /opt/local/Library/Frameworks/QtOpenGL.framework/QtOpenGL
  otool -l /opt/local/Library/Frameworks/QtOpenGL.framework/QtOpenGL
  otool -D /opt/local/Library/Frameworks/QtOpenGL.framework/QtOpenGL

SSR for Mac OS X: qt_menu.nib not found
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This was fixed in MacPorts, see https://trac.macports.org/ticket/37662. Thanks to Chris Pike!
Since version 0.5 (switching to qt5), qt_menu.nib is not needed any more.

Compilation Error on Ubuntu and Archlinux
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This issue was resolved in version 0.3.4. Some newer distributions got more
picky about the necessary ``#include`` commands. If the SSR refuses to
compile, add this to the file ``src/gui/qopenglplotter.h`` (somewhere at the
beginning)::

    #include <GL/glu.h>

On Mac OS X you'll need this instead::

    #include <OpenGL/glu.h>

Polhemus tracker does not work with SSR
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This issue was resolved in version 0.3.3, where we changed the tracker
selection. Use ``--tracker=fastrak``, ``--tracker=patriot`` and ``--tracker=intersense``,
respectively. The serial port can be specified with ``--tracker-port=/dev/
ttyUSB0`` (or similar).

This can happen when both the Intersense tracker as well as the Polhemus
tracker are compiled and the file ``isports.ini`` is present. The latter tells
the Intersense tracker which port to use instead of the standard serial port
``/dev/ttyS0``. If the ``isports.ini`` file lists the port to which the
Polhemus tracker is connected, it can happen that something that we have not
fully understood goes wrong and the Pohlemus data can not be read. In this
case you can either rename the file isports.ini or change its content.

It might be necessary to execute ``echo C > /dev/ttyS0`` several times in
order to make Polhemus Fastrak operational again. Use ``echo -e "C\r" > /dev/ttyS0`` for Polhemus Patriot. You can check with ``cat /dev/ttyS0`` if it delivers data.

Missing GUI Buttons and Timeline
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This issue was resolved in version 0.3.2, the default setting for ``--enable-floating-control-panel`` is chosen depending on the installed Qt version.
As of version 0.5 (switching to qt5), the floating control panel is always enabled.

Different versions of Qt show different behaviour regarding OpenGL Overlays
and as a result, the GUI buttons are not shown in newer Qt versions.

To overcome this limitation, we provide two GUI variants:

- Traditional GUI, can be used up to Qt 4.6.x
- Floating control panel, which is used with Qt 4.7 and above

The floating control panel is the default setting on Mac OS X, for Linux it
can be activated with::

    ./configure --enable-floating-control-panel

OpenGL Linker Error
~~~~~~~~~~~~~~~~~~~

This issue was resolved in version 0.3.2.

On some systems, after running make, you'll get an error mentioning "
glSelectBuffer".

For now, this is the solution (see also the issue below)::

    ./configure LIBS=-lGL

IP interface isn't selected although boost libraries are installed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This issue was resolved with dropping boost::asio for asio in version 0.5.0.

For older builds, you might need to add the ``-lpthread`` flag::

  ./configure LIBS=-lpthread

Second instance of SSR crashes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This happens when two or more instances of the SSR are started with the IP server enabled.
Start all (or at least all instances higher than 1) with the ``-I`` flag to disable the
IP interface.

Audio files with spaces
~~~~~~~~~~~~~~~~~~~~~~~
This issue was resolved in version 0.3.2.

Please do not use audio files with spaces for scenes. Neither the filename nor
the directory referenced in the scene (asd-file) should contain spaces.

Error ``ValueError: unknown locale: UTF-8`` when building the manual
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This can happen on non-US Macs. Go to your home folder ``/Users/YOUR_USER_NAME``, open (or
create) the file ``.bash_profile`` and add the following to this file::

  export LC_ALL=en_US.UFT-8
  export LANG=en_US.UTF-8
  export LANGUAGE=en_US.UTF-8
  export LC_CTYPE=en_US.UTF-8

You might have to re-open the terminal or log out and in again to see the effect.
