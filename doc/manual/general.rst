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

General
=======

Introduction
------------

The SoundScape Renderer (SSR) is a software framework for real-time
spatial audio reproduction running under GNU/Linux, macOS and
possibly some other UNIX variants. The MS Windows version is experimental.

The current implementation provides:

- :ref:`Binaural (HRTF-based) reproduction <binaural_renderer>`
- :ref:`Binaural room (re-)synthesis (BRTF-based reproduction) <brs>`
- :ref:`Vector Base Amplitude Panning (VBAP) <vbap>`
- :ref:`Wave Field Synthesis (WFS) <wfs>`
- :ref:`Ambisonics Amplitude Panning (AAP) <aap>`

There is also the slightly exotic :ref:`Generic Renderer <genren>`, which is essentially a MIMO convolution engine. For each rendering algorithm, there is a separate executable file.

The SSR is intended as versatile framework for the state-of-the-art
implementation of various spatial audio reproduction techniques. You may
use it for your own academic research, teaching or demonstration
activities or whatever else you like. However, it would be nice if you
would mention the use of the SSR by e.g. referencing [Geier2008a]_ or
[Geier2012]_.

Note that so far, the SSR only supports two-dimensional reproduction for
most renderers (the binaural renderer with SOFA files being the laudable
exception that already supports 3D).
For WFS principally any convex loudspeaker setup
(e.g. circles, rectangles) can be used. The loudspeakers should be
densely spaced. For VBAP circular setups are highly recommended. APA
does require circular setups. The binaural renderer can handle only one
listener at a time.

.. [Geier2008a] Matthias Geier, Jens Ahrens, and Sascha Spors. The SoundScape
    Renderer: A unified spatial audio reproduction framework for arbitrary
    rendering methods. In 124th AES Convention, Amsterdam, The Netherlands,
    May 2008 Audio Engineering Society (AES).

.. [Geier2012] Matthias Geier and Sascha Spors. Spatial audio reproduction
    with the SoundScape Renderer. In 27th Tonmeistertagung – VDT
    International Convention, 2012.

.. _quick_start:

Quick Start
-----------

After downloading the SSR package from http://spatialaudio.net/ssr/download/,
open a shell and use following commands:

::

    tar xvzf ssr-x.y.z.tar.gz
    cd ssr-x.y.z
    ./configure
    make
    sudo make install
    qjackctl &
    ssr-binaural my_audio_file.wav

You have to replace ``x.y.z`` with the current version number,
e.g. ``0.5.0``. With above commands you are performing the following
steps:

-  Unpack the downloaded tarball containing the source-code.

-  Go to the extracted directory  [1]_.

-  Configure the SSR.

-  Build the SSR.

-  Install the SSR.

-  Open the graphical user interface for JACK (``qjackctl``). Please
   click "Start" to start the server. As alternative you can start JACK
   with::

       jackd -d alsa -r 44100

   See section :ref:`running_ssr` and ``man jackd`` for further
   options.

-  Open the SSR with an audio file of your choice. This can be a
   multichannel file.

This will load the audio file ``my_audio_file.wav`` and create a virtual
sound source for each channel in the audio file.
Please use headphones to listen to the output generated by the binaural renderer!
If you want to use a different renderer, use
``ssr-brs``,
``ssr-wfs``,
``ssr-vbap``,
``ssr-aap``,
``ssr-dca`` or
``ssr-generic``
instead of ``ssr-binaural``.

If you don't need a graphical user interface and you want to dedicate
all your resources to audio processing, try::

    ssr-binaural --no-gui my_audio_file.wav

For further options, see the section :ref:`running_ssr` and
``ssr-binaural --help``.

.. _audio_scenes:

Audio Scenes
------------

Format
~~~~~~

The SSR can open ``.asd`` files (refer to the section :ref:`asdf`) as well as
normal audio files. If an audio file is opened, SSR creates an
individual virtual sound source for each channel which the audio file
contains. If a two-channel audio file is opened, the resulting virtual
sound sources are positioned like a virtual stereo loudspeaker setup
with respect to the location of the reference point. For audio files
with more (or less) channels, SSR randomly arranges the resulting
virtual sound sources. All types that ecasound and libsndfile can open
can be used. In particular this includes ``.wav``, ``.aiff``, ``.flac``
and ``.ogg`` files.

In the case of a scene being loaded from an ``.asd`` file, all audio
files which are associated to virtual sound sources are replayed in
parallel and replaying starts at the beginning of the scene.

Coordinate System
~~~~~~~~~~~~~~~~~

.. _coordinate_system:

.. figure:: images/coordinate_system.png
    :align: center

    The coordinate system used in the SSR.
    In ASDF :math:`\alpha` and :math:`\alpha'` are referred to as azimuth
    (refer to the section :ref:`asdf`).

Fig. :ref:`1.1 (a) <coordinate_system>` depicts the
global coordinate system used in the SSR. Virtual sound sources as well
as the reference are positioned and orientated with respect to this
coordinate system. For loudspeakers, positioning is a bit more tricky
since it is done with respect to a local coordinate system determined by
the reference. Refer to
Fig. :ref:`1.1 (b) <coordinate_system>`. The loudspeakers
are positioned with respect to the primed coordinates (:math:`x'`\ ,
:math:`y'`\ , etc.).

The motivation to do it like this is to have a means to virtually move
the entire loudspeaker setup inside a scene by simply moving the
reference. This enables arbitrary movement of the listener in a scene
independent of the physical setup of the reproduction system.

Please do not confuse the origin of the coordinate system with the
reference. The coordinate system is static and specifies absolute
positions.

The reference is movable and is always taken with respect to the current
reproduction setup. The loudspeaker-based methods do not consider the
orientation of the reference point but its location influences the way
loudspeakers are driven. E.g., the reference location corresponds to the
*sweet spot* in VBAP. It is therefore advisable to put the reference
point to your preferred listening position. In the binaural methods the
reference point represents the listener and indicates the position and
orientation of the latter. It is therefore essential to set it properly
in this case.

Note that the reference position and orientation can of course be
updated in real-time. For the loudspeaker-based methods this is only
useful to a limited extent unless you want to move inside the scene.
However, for the binaural methods it is essential that both the
reference position and orientation (i.e. the listener's position and
orientation) are tracked and updated in real-time. Refer also to
Sec. :ref:`Head-Tracking <head_tracking>`.

.. _asdf:

Audio Scene Description Format (ASDF)
-------------------------------------

Besides pure audio files, SSR can also read the current development
version of the *Audio Scene Description Format (ASDF)* [Geier2008b]_. Note,
however,
that so far we have only implemented descriptions of static features.
That means in the current state it is not possible to describe
e.g. movements of a virtual sound source.
See https://github.com/SoundScapeRenderer/ssr/pull/155 for latest developments.

As you can see in the example
audio scene below, an audio file can be assigned to each virtual sound
source. The replay of all involved audio files is synchronized to the
replay of the entire scene. That means all audio files start at the
beginning of the sound scene. If you fast forward or rewind the scene,
all audio files fast forward or rewind. **Note that it is significantly
more efficient to read data from an interleaved multichannel file
compared to reading all channels from individual files**.

.. [Geier2008b] Matthias Geier, Jens Ahrens, and Sascha Spors. ASDF: Ein XML
    Format zur Beschreibung von virtuellen 3D-Audioszenen. In 34rd German
    Annual Conference on Acoustics (DAGA), Dresden, Germany, March 2008.

Syntax
~~~~~~

The format syntax is quite self-explanatory. See the examples below.
Note that the paths to the audio files can be either absolute (not
recommended) or relative to the directory where the scene file is
stored. The exact format description of the ASDF can be found in the XML
Schema file ``asdf.xsd``.

Find below a sample scene description:

.. code-block:: xml

    <?xml version="1.0"?>
    <asdf version="0.1">
      <header>
        <name>Simple Example Scene</name>
      </header>
      <scene_setup>
        <source name="Vocals" model="point">
          <file>audio/demo.wav</file>
          <position x="-2" y="2"/>
        </source>
        <source name="Ambience" model="plane">
          <file channel="2">audio/demo.wav</file>
          <position x="2" y="2"/>
        </source>
      </scene_setup>
    </asdf>

The input channels of a soundcard can be used by specifying the channel
number instead of an audio file, e.g. ``<port>3</port>`` instead of
``<file>my_audio.wav</file>``.

Examples
~~~~~~~~

We provide an audio scene example in ASDF with this release. You find it
in :gh-link:`data/scenes/live_input.asd`. If you load this file into the SSR it
will create 4 sound sources which will be connected to the first four
channels of your sound card. If your sound card happens to have less
than four outputs, less sources will be created accordingly. More
examples for audio scenes can be downloaded from the SSR website 
http://spatialaudio.net/ssr/.

.. _ip_interface:

IP Interface
------------

One of the key features of the SSR is the ability to remotely control it
via a network interface.
This enables you to straightforwardly connect any type of
interaction tool from any type of operating system.
There are multiple network interfaces available, see :ref:`network`.

Bug Reports, Feature Requests and Comments
------------------------------------------

Please report any bugs, feature requests and comments at
https://github.com/SoundScapeRenderer/ssr/issues or send an e-mail to
``ssr@spatialaudio.net``. We will keep
track of them and will try to fix them in a reasonable time. The more
bugs you report the more we can fix. Of course, you are welcome to
provide bug fixes. 

Contributors
------------

.. include:: ../../AUTHORS

See also the Github repository https://github.com/soundscaperenderer/ssr for more contributors.

Your Own Contributions
----------------------

The SSR is thought to provide a state of the art implementation of
various spatial audio reproduction techniques. We therefore would like
to encourage you to contribute to this project since we can not assure
to be at the state of the art at all times ourselves. Everybody is
welcome to contribute to the development of the SSR. However, if you are
planning to do so, we kindly ask you to contact us beforehand (e.g. via
``ssr@spatialaudio.net``).
The SSR is in a rather temporary state and we might apply some
changes to its architecture. We would like to ensure that your own
implementations stay compatible with future versions.

.. [1]
   Note that most relative paths which are mentioned in this document
   are relative to this folder, which is the folder where the SSR
   tarball was extracted. Therefore, e.g. the ``src/`` directory could
   be something like ``$HOME/ssr-x.y.z/src/`` where "x.y.z" stands for the
   version number.
