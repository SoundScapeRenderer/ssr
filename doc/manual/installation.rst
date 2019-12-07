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

.. _installation:

Installation
============

The following sections describe how to build and/or install the SSR on your
computer. The SSR can be used on GNU/Linux and Mac OS X and (experimentally) on
MS Windows.

GNU/Linux
---------

There are `precompiled packages`_ available for `Arch Linux`_, Debian_ and
Ubuntu_ (or their respective derivatives) and a package script for the
development version of the SSR on `Arch Linux`_.

However, it is possible to build the SSR from source on any modern
GNU/Linux (see `Building from source on Linux`_).

.. _`precompiled packages`: https://repology.org/project/soundscaperenderer/versions
.. _`Arch Linux`: https://www.archlinux.org
.. _Debian: https://www.debian.org
.. _Ubuntu: https://www.ubuntu.com

.. _arch_linux_package:

Arch Linux
__________

The SSR is available in the official repositories and can be installed using:

  .. code:: shell

    sudo pacman -S ssr

A `package script`_ (`PKGBUILD`_) for the development version of the SSR,
tracking the master branch, is available in the `Arch User Repository`_
(`AUR`_). This requires to be built manually, as is explained in the following
subsection.

.. _`package script`: https://aur.archlinux.org/packages/ssr-git/
.. _`PKGBUILD`: https://wiki.archlinux.org/index.php/PKGBUILD
.. _`Arch User Repository`: https://wiki.archlinux.org/index.php/Arch_User_Repository

Building and installing with makepkg
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The recommended way of building package scripts from the `AUR`_ and installing
the resulting packages is to use `makepkg`_ in a `manual build process`_.

  .. code:: shell

    # make sure to have the base-devel group installed
    $ pacman -Sy base-devel
    # clone the sources of the package script
    $ git clone https://aur.archlinux.org/ssr-git
    # go to the directory with the package script
    $ cd ssr-git
    # make and install the package and all of its dependencies
    $ makepkg -csi

.. _AUR: https://aur.archlinux.org
.. _makepkg: https://wiki.archlinux.org/index.php/Makepkg
.. _`manual build process`: https://wiki.archlinux.org/index.php/Arch_User_Repository#Installing_packages

.. _debian_package:

Debian/ Ubuntu
______________

The SSR is available as `Debian package`_ and `Ubuntu package`_. It is called
*soundscaperenderer* and you can install it using your favorite package
manager/ package manager frontend (*apt-get*, *aptitude*, *synaptic*, ...), all
dependencies should be installed automatically.

  .. code:: shell

    # install example using aptitude
    $ sudo aptitude update
    $ sudo aptitude install soundscaperenderer

.. _`Debian package`: https://packages.debian.org/search?keywords=soundscaperenderer
.. _`Ubuntu package`: http://packages.ubuntu.com/search?keywords=soundscaperenderer

Building from source on Linux
_____________________________

If you didn't receive this manual with the SSR source code, you can download it
from the official website http://spatialaudio.net/ssr or from the upstream `git
repository`_.

You can downloaded the tarball (the file that ends with ``.tar.gz``), unpack it
and change to the newly created directory:

  .. code:: shell

    tar xvzf ssr-*.tar.gz
    cd ssr-*

Alternatively, if you wish to build from source, you can clone directly from
one of the upstream repositories and change to the newly created directory:

  .. code:: shell

    git clone https://github.com/SoundScapeRenderer/ssr.git
    cd ssr

.. note::

    If using the `git repository`_ directly, make sure to generate the
    configure script, before proceeding with `Configuring and compiling`_:

  .. code:: shell

    autoreconf -vfi

.. _git repository: https://github.com/SoundScapeRenderer/ssr/

.. _dependencies:

Dependencies
~~~~~~~~~~~~

The following lists packages, that need to be installed on your system (i.e.
dependencies of the SSR).
The recommended way of doing that is to use your distribution`s `package
manager`_.
However, if you prefer, you can of course also download everything as source
code and compile each program yourself.

.. note::

    The package names may vary slightly depending on your distribution or might
    not be shipped in separate ``lib`` or ``dev`` packages!

- **make**
- **g++** (>=4.7.3) or **clang**
- **libasio-dev**
- **libqt5-dev** and **libqt5-opengl-dev** (for the GUI)
- **libecasoundc2.2-dev** or **libecasoundc-dev**
- **ecasound**
- **libxml2-dev**
- **libfftw3-dev**
- **libsndfile1-dev**
- **libjack-dev** or **libjack-jackd2-dev**
- **jackd** or **jackd1** or **jackd2**
- **pkg-config**

Extra dependencies for installing from the `git repository`_:

- **libtool**
- **automake**
- **autoconf**
- **help2man**

Note that there are additional dependencies for some of the tracking systems
that the SSR supports. Refer to Section :ref:`head_tracking` for further
information.

To use a different compiler, you can specify it with ``CXX``:

  .. code:: shell

    ./configure CXX=clang++

.. _`package manager`: https://en.wikipedia.org/wiki/List_of_software_package_management_systems#Linux

.. _configuring:

Configuring and compiling
~~~~~~~~~~~~~~~~~~~~~~~~~

To build the SSR from source you have to configure first. Open a shell
and change to the directory containing the source code of the package
and type:

  .. code:: shell

    ./configure

This script will check your system for dependencies and prepare the
``Makefile`` required for compilation. If any of the required software,
mentioned in section :ref:`Dependencies <dependencies>` is missing, the
``configure`` script will signal that.

At successful termination of the ``configure`` script a summary will show
up and you are ready to compile.

If dependencies for certain modules of the SSR are missing that you are not
going to use, then you can simply disable the according module by adding the
appropriate argument to the call to the `` configure`` script so that you do
not need to bother with the dependencies.
Examples are:

  .. code:: shell

    ./configure --disable-ip-interface
    ./configure --disable-gui

See Section :ref:`Hints on configuration <hints_conf>` for details.


If everything went smoothly, you can continue with the next step:

  .. code:: shell

    make

This will take some time (maybe a few minutes). If you have a multi-core or
multi-processor computer you can speed things up by specifying the number of
processes you want to use with ``make -j8`` (or any other number that you
choose).

.. _hints_conf:

Hints on configuration
~~~~~~~~~~~~~~~~~~~~~~

If you encounter problems configuring the SSR these hints could help:

-  Ensure that you really installed all libraries (``lib``) with
   devel-package (``devel`` or ``dev``, where available) mentioned in
   Section :ref:`Dependencies <dependencies>`.

-  If your QT5 library cannot be found during configuration,
   try running ``export QT_SELECT=qt5``.
   If there are problems with qt5's ``moc`` during the build, you might need to add
   the corresponding folder (like ``/usr/local/opt/qt/bin``) to your ``PATH``.

-  It may be necessary to run ``ldconfig`` after installing new
   libraries.

-  Ensure that ``/etc/ld.so.conf`` or ``LD_LIBRARY_PATH`` are set
   properly, and run ``ldconfig`` after changes.

-  If a header is not installed in the standard paths of your system you
   can pass its location to the configure script using
   ``./configure CPPFLAGS=-Iyourpath``.

Note that with ``./configure --help`` all configure-options are
displayed, e.g. in Section "Optional Features" you will find how to
disable compilation of the head trackers and many other things. Setting
the influential environment variables with ``./configure VARNAME=value``
can be useful for debugging dependencies.

Installing
~~~~~~~~~~

Until now, everything was happening in the source directory (something like ssr
-x.x.x/). To be used by other users on the system, the SSR has to be installed
system-wide, using:

  .. code:: shell

    make install

.. note::

    To execute this step, you will need superuser_ privileges. Depending on
    your system setup, these might be acquired with the help of ``sudo``.


Uninstalling
~~~~~~~~~~~~

If the SSR didn't meet your expectations, we are very sorry, but of course you
can easily remove it from your system again using:

  .. code:: shell

    make uninstall

.. _superuser: https://en.wikipedia.org/wiki/Superuser

.. _mac_os_x:

Mac OS X
--------

JACK on Mac OS X
________________

Tested with version 0.87 (64 bit) which includes:

- Jackdmp 1.9.6
- JackRouter 0.9.3
- JackPilot 1.7.0

Note that the site http://www.jackosx.com/ is outdated. The latest version of JACK is
available from http://jackaudio.org/downloads/.

Or, you can install JACK using Homebrew_.

If you are using OS X El Capitan or newer, make sure that you are installing
the version "jackOSX 0.92_b3" from http://jackaudio.org/downloads/. JACK
versions installed from other sources tend not to work on these versions of OS
X.

Application Bundle
__________________

This assumes that you are using the precompiled SSR application bundle for Mac OS
X. If you want to build the SSR yourself, have a look at `Building from Source`_.

You can download the application bundle from http://spatialaudio.net/ssr. You will need
JACK as prerequisite. Refer to `JACK on Mac OS X`_ for instructions how to obtain and
install it.

The installation should be straightforward. Just double-click on the ``.dmg`` file and
drag the ``SoundScapeRenderer-x.x.x`` folder to your ``Applications`` folder. Done. When
double-clicking the SSR application bundle, it will indicate to you that you should
download and install JACK (that's what this JACK.webloc thingy wants). If JACK is
installed on your computer, then simply ignore this step and drag the SSR folder to your
``Applications`` folder.

The application bundle can be placed anywhere, but spaces in path names might
cause trouble, so it's better to avoid them. Another thing is that Mac OS X
sort of adds applications placed in the ``/Applications`` folder to the
environment, so lets assume you put the SSR there (This also works for
``$HOME/Applications``).

Under new versions of Mac OS (Sierra and up) the SoundScapeRenderer executable might be stuck in quarantine resulting in files not being loaded properly. The corresponding error message might look like this::

/private/var/folders/_t/67rf88lx507btn91x6g_vfk40000gp/T/AppTranslocation/42F7F94E-AED9-4F39-8647-41D898CCE032/d/SoundScapeRenderer.app/Contents/MacOS/ssr: line 48: 36804 Abort trap: 6           $SSR_EXECUTABLE "${OPTIONS[@]}"

You can allow the SoundScapeRender to run from its original location by running::

  xattr -d com.apple.quarantine SoundScapeRenderer.app

The following warning can occur on High Sierra when using the file dialog::

  objc[50474]: Class FIFinderSyncExtensionHost is implemented in both /System/Library/PrivateFrameworks/FinderKit.framework/Versions/A/FinderKit (0x7fffa1883c90) and /System/Library/PrivateFrameworks/FileProvider.framework/OverrideBundles/FinderSyncCollaborationFileProviderOverride.bundle/Contents/MacOS/FinderSyncCollaborationFileProviderOverride (0x11f84ccd8). One of the two will be used. Which one is undefined.

It is a bug outside of SSR.

Building from Source
____________________

The following is an overview on how to set up the build environment for SSR on Mac OS X.

What to install first?
~~~~~~~~~~~~~~~~~~~~~~

You can make your life much easier with a decent package manager, name Homebrew (https://brew.sh/) or MacPorts (https://www.macports.org/). Both greatly simplify the process of installing and managing dependencies.

.. _Homebrew:

Homebrew (recommended)
**********************

After installing homebrew, you can simply run the following line to update homebrew's
internal repository, upgrade itself and install all necessary dependencies::

  brew update && brew upgrade && brew install autoconf fftw libsndfile jack ecasound qt asio --c++11

If Qt is not found by the build system, i.e., if the build system proposes to compile without GUI, then run the following commands (using the according paths on your system) or add them to your ``~/.bash_profile`` file::

  export PATH=/usr/local/opt/qt/bin:$PATH
  export PKG_CONFIG_PATH=/usr/local/opt/qt/lib/pkgconfig

To build the manual and documentation, you can also install help2man and doxygen::

  brew install help2man doxygen
  export LC_CTYPE=en_US.UTF-8

On El Capitan and newer OS X versions, it has happened that only the help2man version installed through MacPorts worked properly.

MacPorts (not recommended)
**************************

Tested with version 1.9.2

Download here: https://www.macports.org/

Then open a terminal and do an initial ports tree update ::

  sudo port selfupdate

If that is not working it's most likely because the router won't let you use
rsync. So we switch to http::

  sudo nano /opt/local/etc/macports/sources.conf

Comment out the rsync entry ::

  #rsync://rsync.macports.org/release/ports/ [default]

And add a line ::

  http://www.macports.org/files/ports.tar.gz [default]

Now save the file and try the selfupdate again.

MacPorts ports
**************

These ports have to be installed (dependencies are installed automatically)

- gcc49 (or some other version of GCC, but at least gcc47)
- pkgconfig
- libsndfile
- libsamplerate
- fftw-3-single
- qt5-mac
- libxml2

If you want, you can also use clang instead of GCC to compile the SSR.

If you want to install the newest SSR development version directly from the Git repository, you'll need those as well:

- autoconf
- automake
- help2man

Ports are installed using ::

  sudo port install <portname>

Because ports are compiled locally, it may take a long time to install all
ports. Issuing one command to install all ports might be more convenient::

  sudo sh -c "port install gcc49 && port install pkgconfig && port install libsndfile && port install libsamplerate && port install fftw-3-single && port install qt5-mac && port install libxml2"

Lastly, you need to install the asio library if you want to compile with the network
interface. You can download it from: http://think-async.com

.. _ecasound:

Ecasound
********

Tested with version 2.7.2

If you don't want to get Ecasound from Homebrew_, then download the source code from
http://www.eca.cx/ecasound/. (If you choose to use Homebrew and you're experiencing
problems, then you might want to take a look at :ref:`ecasound_cannot_open_a_jack_port`).

It didn't work with 2.9.0 for us, older versions can be found there:
https://ecasound.seul.org/download/.

In Terminal go into the unpacked ecasound folder and do::

  ./configure CPPFLAGS=-I/opt/local/include LIBS=-L/opt/local/lib

If JACK cannot be found, you can also try this::

  ./configure CPPFLAGS="-I/opt/local/include -I/usr/local/include" LIBS=-L/opt/local/lib

When the configure script is finished, check if libsndfile, libsamplerate and
JACK are enabled. It should look something like that::

  ...
  -----------------------------------------------------------------
  Following features were selected:
  ...
  Libsndfile:             yes
  ...
  JACK support:           yes
  Libsamplerate support   yes
  ...
  -----------------------------------------------------------------
  ...

If not, check that JACK and all MacPort packages mentioned above are installed.
If everything looks OK, continue with::

  make
  make install

For the last step you need write access to ``/usr/local``.
If it doesn't work, use this instead::

  sudo make install

Ecasound -- git version
***********************

Note: if you successfully installed Ecasound 2.7.2, you *don't* need this!

If you want to use the newest Ecasound version from their git repository
(https://ecasound.seul.org/ecasound.git) with OS X 10.9 (Mavericks),
you can try this::

  ./configure CXX="clang++ -stdlib=libc++" CPPFLAGS=-D_DARWIN_C_SOURCE

Note, however, that you will have to use the same -stdlib options when
configuring the SSR.

Standard build
**************

Get the SSR Source tarball and unpack it where ever you wish, then open a
Terminal window and ``cd`` into the newly created folder.

First of all, you have to issue this command in order for ``pkg-config`` to
find the installed JACK version::

  export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig

You now have two options:

\1) If you want to build a clickable application bundle and wrap it into a
``.dmg image``, you can build the SSR in *Mac OS X* style::

  ./configure --enable-app-bundle
  make
  make dmg

You can also name the ``.dmg``::

  ./configure
  --enable-app-bundle="MyVeryOwnSoundScapeRenderer.dmg"
  make
  make dmg

The resulting ``.dmg`` is output to the SSR source folder. The application
bundle inside contains all non-system dynamic libraries needed by the SSR,
except the JACK libs. So it should run on every Mac OS X 10.6.x with JACK
installed.

\2) If you want to build and install in *Linux* style::

  ./configure
  make
  make install

For the last step you need write access in ``/usr/local``. If it doesn't work,
use this instead::

  sudo make install

The binaries do not get wrapped in an application bundle but they will be
installed in ``/usr/local/bin`` and some files will be installed in ``/usr/
local/share/ssr``. If you want to remove all these file again, just do::

  make uninstall

or (if you used ``sudo`` before)::

  sudo make uninstall

You can start the SSR and pass arguments to the SSR the same way you would do
it on Linux.

.. _mac_intersense_support:

Build with InterSense tracker support
*************************************

Tested with IntertiaCube3, software version 4.17

Get the SDK from http://www.intersense.com/. It should contain a dynamic
library called ``libisense.dylib`` and two header files called ``isense.h``
and ``types.h``, respectively.

Put ``libisense.dylib`` into ``/usr/local/lib`` and the header files into ``/
usr/local/include/intersense``.

Then build like described above, but add ``CPPFLAGS=-I/usr/local/include/
intersense`` to the configure parameters::

  export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
  ./configure CPPFLAGS=-I/usr/local/include/intersense

For some strange reason the full path of ``libisense.dylib`` is not written to
the header of the binary. So if you configure with ``--enable-app-bundle`` and
then do ``make dmg`` to build an application bundle, a tool called
``dylibbundler`` will ask you to enter its path (``/usr/local/lib``) several
times.

MS Windows
----------

The MS Windows version of SSR is experimental at this stage. Find the
pre-release of the executables at https://github.com/chris-hld/ssr/releases.
Note that this SSR version only works with live inputs currently (it cannot
play audio files). It has no limitation otherwise.

