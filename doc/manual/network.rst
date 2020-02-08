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

.. _network:

Network Interfaces
==================

.. warning::

    We did not evaluate any of the network interfaces in terms of security.
    So please be sure that you are in a safe network when using them.

The *SoundScape Renderer* has no less than three network interfaces using
different protocols:

* The :doc:`WebSocket protocol <network-websocket>` has built-in support in all
  modern web browsers and can be used to control the SSR from a web browser.

* The :doc:`FUDI protocol <network-fudi>` is the native protocol of the visual
  programming language Pure Data.

* The :doc:`legacy XML-based protocol <network-legacy>` is still supported for
  backwards-compatibility but should not be used for new projects.

.. toctree::
    :hidden:

    network-websocket
    network-fudi
    network-legacy
