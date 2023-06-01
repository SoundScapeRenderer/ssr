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

.. _`Arch Linux`: https://archlinux.org
.. _`package script`: https://aur.archlinux.org/packages/ssr-git/
.. _`PKGBUILD`: https://wiki.archlinux.org/title/PKGBUILD
.. _`Arch User Repository`: https://wiki.archlinux.org/title/Arch_User_Repository

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
.. _makepkg: https://wiki.archlinux.org/title/Makepkg
.. _`manual build process`: https://wiki.archlinux.org/title/Arch_User_Repository#Installing_and_upgrading_packages


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
.. _`Ubuntu package`: https://packages.ubuntu.com/search?keywords=soundscaperenderer

If you don't need the :doc:`gui`, there is also the GUI-less package
``soundscaperenderer-nox`` with fewer dependencies.


macOS
-----

Independent of whether you compile SSR yourself or get it from Homebrew_ as
described below, make sure that you install JACK from Homebrew::

    brew install jack

We also recommed QJackCtl for convenient usage of JACK::

    brew install qjackctl

SSR can be installed via::

    brew install SoundScapeRenderer/ssr/ssr

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
