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

Network Interface
=================

This is just a short overview about the XML messages which can be sent
to the SSR via TCP/IP. By default, messages have to be terminated with a binary
zero (``\0``). This can be changed to, for example, a newline / line feed
(``\n``) or to a carriage return (``\r``) using one of the following ways:

-  Command line:
   Use the command line option ``--end-of-message-character=VALUE``
   VALUE is the ASCII code for the desired character (binary zero: 0; line
   feed: 10; carriage return: 13).

-  Configuration file:
   Here, there is the option END_OF_MESSAGE_CHARACTER. See the example
   ``data/ssr.conf.example``. Use ASCII codes here as well.

The choice of delimiter applies to, of course, both sent and received messages.

**WARNING:** We did not evaluate the network interface in terms of
security. So please be sure that you are in a safe network when using
it.

Scene
-----

-  Load Scene:
   ``<request><scene load="path/to/scene.asd"/></request>``

-  Clear Scene (remove all sources):
   ``<request><scene clear="true"/></request>``

-  Set Master Volume (in dB):
   ``<request><scene volume="6"/></request>``

State
-----

-  Start processing:
   ``<request><state processing="start"/></request>``

-  Stop processing:
   ``<request><state processing="stop"/></request>``

-  Transport Start (Play):
   ``<request><state transport="start"/></request>``

-  Transport Stop (Pause):
   ``<request><state transport="stop"/></request>``

-  Transport Rewind:
   ``<request><state transport="rewind"/></request>``

-  Transport Locate:
   ``<request><state seek="4:33"/></request>``
   ``<request><state seek="1.5 h"/></request>``
   ``<request><state seek="42"/></request>`` *(seconds)*
   ``<request><state seek="4:23:12.322"/></request>``

-  Reset/Calibrate Head-Tracker:
   ``<request><state tracker="reset"/></request>``

Source
------

-  Set Source Position (in meters):
   ``<request><source id="42"><position x="1.2" y="-2"/></source></request>``

-  Fixed Position (``true``/``false``):
   ``<request><source id="42"><position fixed="true"/></source></request>``

   ::

       <request><source id="42">
         <position x="1.2" y="-2" fixed="true"/>
       </source></request>


-  Set Source Orientation (in degrees, zero in positive x-direction):
   ``<request><source id="42"><orientation azimuth="93"/></source></request>``

-  Set Source Gain (Volume in dB):
   ``<request><source id="42" volume="-2"/></request>``

-  Set Source Mute (``true``/``false``):
   ``<request><source id="42" mute="true"/></request>``

-  Set Source Name:
   ``<request><source id="42" name="My first source" /></request>``

-  Set Source Model (``point``/``plane``):
   ``<request><source id="42" model="point"/></request>``

-  Set Source Port Name (any JACK port):
   ``<request><source id="42" port="system:capture_3"/></request>``

-  New Source (some of the parameters are optional):

   ::

       <request>
         <source new="true" name="a new source"
             file="path/to/audio.wav" channel="2">
           <postition x="-0.3" y="1" fixed="true"/>
           <orientation azimuth="99"/>
         </source>
       </request>


   ::

       <request>
         <source new="true" name="a source from pd"
             port="pure_data_0:output0" volume="-6">
           <postition x="0.7" y="2.3"/>
         </source>
       </request>


-  Delete Source:
   ``<request><delete><source id="42"/></delete></request>``

Reference
---------

-  Set Reference Position (in meters):
   ``<request><reference><position x="-0.3" y="1.1"/></reference></request>``

-  Set Reference Orientation (in degrees, zero in positive x-direction):
   ``<request><reference><orientation azimuth="90"/></reference></request>``


