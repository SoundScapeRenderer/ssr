.. _network:

Network Interface
=================

This is just a short overview about the XML messages which can be sent
to the SSR via TCP/IP. The messages have to be terminated with a binary
zero (``\0``).

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
   ``<request><source id="42" port_name="system:capture_3"/></request>``

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


