Building the SSR from Source
============================

Tarball
-------

The so-called *tarball* -- a file named like :file:`ssr-{x}.{y}.{z}.tar.gz` --
can be downloaded from http://spatialaudio.net/ssr/download/.


After downloading the tarball, unpack it and
change to the newly created directory::

    tar xvzf ssr-*.tar.gz
    cd ssr-*


The tarball has the advantage that
it already contains a few auto-generated files and
therefore needs fewer dependencies.
However, if you want to use the latest development version,
you should get the code from the git repository:


Git Repository
--------------

The source code repository of the SSR is located at
https://github.com/SoundScapeRenderer/ssr/.

You can *clone* it and change to the newly created directory like this::

    git clone https://github.com/SoundScapeRenderer/ssr.git --recursive
    cd ssr

.. note::

    The SSR repository uses multiple `Git submodules`__.
    If you use the ``--recursive`` flag as shown above,
    they will be automatically downloaded.
    If you didn't use the ``--recursive`` flag, you can get them
    by running the command::

        git submodule update --init

    When switching branches or when pulling changes from the server,
    the local submodules might get outdated.
    In such a case they can be updated with::

        git submodule update

    __ https://git-scm.com/book/en/v2/Git-Tools-Submodules

To generate the ``configure`` script
(which is already contained in the tarball),
run the command::

    ./autogen.sh

For this step, you'll need those extra dependencies (see below for how to
install them for both Linux and macOS):

- ``libtool``
- ``automake``
- ``autoconf``

If you want to generate the ``man`` pages
(which are also contained in the tarball), you'll need:

- ``help2man``


.. _dependencies:

Dependencies
------------

To enable support for the newest `ASDF version 0.4`__,
the `C-bindings of the asdf-rust library`__ have to be installed
before compiling the SSR.

__ https://AudioSceneDescriptionFormat.readthedocs.io/
__ https://github.com/AudioSceneDescriptionFormat/asdf-rust#building-the-c-api


Linux
^^^^^

The following list of packages needs to be installed on your system
to be able to build the SSR.
The recommended way of installing those packages is to use your distribution's
`package manager`__.
On Debian/Ubuntu you can use ``apt-get``, ``aptitude``, ``synaptic`` etc.
However, if you prefer, you can of course also download everything as source
code and compile each program yourself.

__ https://en.wikipedia.org/wiki/List_of_software_package_management_systems


.. note::

    The package names may vary slightly depending on your distribution or might
    not be shipped in separate ``lib`` or ``dev`` packages!


- ``make``
- ``g++`` or ``clang``
- ``pkg-config``
- ``libxml2-dev``
- ``libfftw3-dev``
- ``libsndfile1-dev``
- ``libjack-jackd2-dev``
- ``jackd2``

For playing/recording audio files:

- ``ecasound``
- ``libecasoundc-dev``

For the GUI:

- ``libqt5-opengl-dev``

For all network interfaces:

- ``libasio-dev``

For the WebSocket interface:

- ``libwebsocketpp-dev``

For the FUDI network interface:

- ``libfmt-dev``

For SOFA support:

- ``libmysofa-dev``

For VRPN tracker support:

- ``vrpn`` on Homebrew (has to be compiled from source on Linux)

For support of the *InterSense IntertiaCube3* head tracker:

- See the CI configuration file
  :download:`.github/workflows/main.yml <../../.github/workflows/main.yml>`
  for instructions.

For a concrete list of Ubuntu and Homebrew packages,
see the CI configuration file
:download:`.github/workflows/main.yml <../../.github/workflows/main.yml>`.

If the Qt5 library cannot be found during configuration, try using ::

    export QT_SELECT=qt5

If there are problems with Qt5's ``moc`` during the build,
you might need to add the corresponding folder
(like ``/usr/local/opt/qt/bin``) to your ``PATH``.
It might also help to install the package ``qt5-default``
to select Qt5 as default Qt version.

On Linux, it may be necessary to run ``ldconfig`` after installing new libraries.
Ensure that ``/etc/ld.so.conf`` or ``LD_LIBRARY_PATH`` are set properly
and run this after any changes::

    sudo ldconfig


.. _dependencies_on_macos:

macOS
^^^^^

We recommend installing all dependencies from Homebrew_::

    brew install make automake libtool pkg-config help2man fftw asio fmt vrpn freeglut yarn node ecasound jack libsndfile websocketpp qt@5 SoundScapeRenderer/ssr/libmysofa llvm

You might be able to skip installing llvm if you have Xcode installed.

And then::

    brew link qt5 --force

However, if you already have a newer version of Qt installed (for example if
you installed the very useful package ``qjackctl``), you have to run this
first::

    brew unlink qt

Once SSR has compiled successfully, you can switch back to the newer Qt
version (otherwise ``qjackctl`` will not work anymore)::

    brew link qt

If you want to use ``help2man`` on macOS, you have to install a Perl package::

    cpan Locale::gettext

.. _Homebrew: https://brew.sh


.. _configuring:

Configuring
-----------

Once all dependencies are installed, the SSR can be configured by running::

    ./configure

This script will check your system for dependencies and prepare the
``Makefile`` required for compilation. If any of the required software,
mentioned in section :ref:`dependencies` is missing, the
``configure`` script will signal that.

At successful termination of the ``configure`` script a summary will show
up and you are ready to compile.

The ``configure`` script accepts many parameters and options,
all of which can be listed with::

    ./configure --help

For example, certain feature can be disabled like this::

    ./configure --disable-ip-interface
    ./configure --disable-websocket-interface --disable-gui

The ``configure`` script also recognizes many environment variables.
For example, to use a different compiler, you can specify it with ``CXX``::

    ./configure CXX=clang++

If a header is not installed in the standard paths of your system you
can pass its location to the configure script using ::

    ./configure CPPFLAGS=-Iyourpath

If you are using an ``arm64`` CPU (i.e. M1, M2 or newer) on macOS
(without Rosetta emulation),
you might have to explicitly add some paths
to be able to find the libraries installed with ``brew``::

    ./configure CPPFLAGS=-I/opt/homebrew/include LDFLAGS=-L/opt/homebrew/lib

Building
--------

If everything went smoothly so far, you can continue with the next step::

    make

This will take some time (maybe a few minutes). If you have a multi-core or
multi-processor computer you can speed things up by specifying the number of
processes you want to use with ``make -j8`` (or any other number that you
choose).

If there are errors, double-check whether all :ref:`dependencies` are installed
and whether the :ref:`configuration options <configuring>` are correct.


Installing
----------

Until now, everything was happening in the source directory.
To be able to use the SSR system-wide, it has to be installed like this::

    make install

.. note::

    To execute this step, you might need superuser_ privileges.
    Depending on your system setup,
    these might be acquired with the help of ``sudo``.

    Alternatively, you can give your own user account
    the right to install programs.
    For example, on Debian and Ubuntu this can be done by adding your user to
    the ``staff`` group like this (assuming your user name is ``myuser``)::

        sudo adduser myuser staff

    For the change to take effect, you might have to log out and log in again.

.. _superuser: https://en.wikipedia.org/wiki/Superuser


Uninstalling
------------

If the SSR didn't meet your expectations, we are very sorry, but of course you
can easily remove it from your system again using::

    make uninstall
