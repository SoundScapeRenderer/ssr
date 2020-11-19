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

The SSR can be installed from packages that are available for
GNU/Linux, macOS and (experimentally) MS Windows.

If none of those packages work for you,
or if you want to use the latest development version,
see :doc:`building-from-source` for instructions.


Arch Linux
----------

The SSR is available in the official `Arch Linux`_ repositories and
can be installed using:

.. code:: shell

    sudo pacman -S ssr

A `package script`_ (`PKGBUILD`_) for the development version of the SSR,
tracking the master branch, is available in the `Arch User Repository`_
(`AUR`_). This requires to be built manually, as is explained in the following
subsection.

.. _`Arch Linux`: https://www.archlinux.org
.. _`package script`: https://aur.archlinux.org/packages/ssr-git/
.. _`PKGBUILD`: https://wiki.archlinux.org/index.php/PKGBUILD
.. _`Arch User Repository`: https://wiki.archlinux.org/index.php/Arch_User_Repository

Building and installing with makepkg
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The recommended way of building package scripts from the `AUR`_ and installing
the resulting packages is to use `makepkg`_ in a `manual build process`_.

.. code:: shell

    # make sure to have the base-devel group installed
    pacman -Sy base-devel
    # clone the sources of the package script
    git clone https://aur.archlinux.org/ssr-git
    # go to the directory with the package script
    cd ssr-git
    # make and install the package and all of its dependencies
    makepkg -csi

.. _AUR: https://aur.archlinux.org
.. _makepkg: https://wiki.archlinux.org/index.php/Makepkg
.. _`manual build process`: https://wiki.archlinux.org/index.php/Arch_User_Repository#Installing_packages


.. _debian_package:

Debian/Ubuntu
-------------

The SSR is available as `Debian package`_ and `Ubuntu package`_. It is called
``soundscaperenderer`` and you can install it using your favorite package
manager/ package manager frontend (*apt-get*, *aptitude*, *synaptic*, ...), all
dependencies should be installed automatically.

.. code:: shell

    sudo aptitude update
    sudo aptitude install soundscaperenderer

.. _`Debian package`: https://packages.debian.org/search?keywords=soundscaperenderer
.. _`Ubuntu package`: http://packages.ubuntu.com/search?keywords=soundscaperenderer

If you don't need the :doc:`gui`, there is also the GUI-less package
``soundscaperenderer-nox`` with fewer dependencies.


.. _mac_os_x:

macOS Application Bundle
------------------------

You can download the application bundle from http://spatialaudio.net/ssr/download/.
You will need
JACK as prerequisite. Refer to `JACK on macOS`_ for instructions how to obtain and
install it.

The installation should be straightforward. Just double-click on the ``.dmg`` file and
drag the ``SoundScapeRenderer-x.y.z`` folder to your ``Applications`` folder. Done. When
double-clicking the SSR application bundle, it will indicate to you that you should
download and install JACK (that's what this JACK.webloc thingy wants). If JACK is
installed on your computer, then simply ignore this step and drag the SSR folder to your
``Applications`` folder.

The application bundle can be placed anywhere, but spaces in path names might
cause trouble, so it's better to avoid them. Another thing is that macOS
sort of adds applications placed in the ``/Applications`` folder to the
environment, so lets assume you put the SSR there (This also works for
``$HOME/Applications``).

Under new versions of macOS (Sierra and up) the SoundScapeRenderer executable might be stuck in quarantine resulting in files not being loaded properly. The corresponding error message might look like this::

/private/var/folders/_t/67rf88lx507btn91x6g_vfk40000gp/T/AppTranslocation/42F7F94E-AED9-4F39-8647-41D898CCE032/d/SoundScapeRenderer.app/Contents/MacOS/ssr: line 48: 36804 Abort trap: 6           $SSR_EXECUTABLE "${OPTIONS[@]}"

You can allow the SoundScapeRender to run from its original location by running::

  xattr -d com.apple.quarantine SoundScapeRenderer.app

The following warning can occur on High Sierra when using the file dialog::

  objc[50474]: Class FIFinderSyncExtensionHost is implemented in both /System/Library/PrivateFrameworks/FinderKit.framework/Versions/A/FinderKit (0x7fffa1883c90) and /System/Library/PrivateFrameworks/FileProvider.framework/OverrideBundles/FinderSyncCollaborationFileProviderOverride.bundle/Contents/MacOS/FinderSyncCollaborationFileProviderOverride (0x11f84ccd8). One of the two will be used. Which one is undefined.

It is a bug outside of SSR.

.. _jack_mac_os_x:

JACK on macOS
^^^^^^^^^^^^^

Tested with version 0.87 (64 bit) which includes:

- Jackdmp 1.9.6
- JackRouter 0.9.3
- JackPilot 1.7.0

Note that the site http://www.jackosx.com/ is outdated. The latest version of JACK is
available from http://jackaudio.org/downloads/.

Or, you can install JACK using Homebrew_.

.. _Homebrew: https://brew.sh/


MS Windows
----------

The MS Windows version of SSR is experimental at this stage. Find the
pre-release of the executables at https://github.com/chris-hld/ssr/releases.
Note that this SSR version only works with live inputs currently (it cannot
play audio files). It has no limitation otherwise.

.. only:: html

    Others
    ------

    .. image:: https://repology.org/badge/vertical-allrepos/soundscaperenderer.svg
        :alt: SSR packaging status
        :target: https://repology.org/project/soundscaperenderer/versions
