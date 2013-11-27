<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet type="text/xsl" href="asdf2html.xsl"?>
<asdf xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:noNamespaceSchemaLocation="asdf.xsd">
  <header>
    <name>Loudspeaker setup using all available features</name>
    <description>
      This arrangement doesn't represent a useful loudspeaker setup, its whole
      purpose is to illustrate all available features.
    </description>
    <version>0.1</version>
    <date>2010-05-15T12:00:00+02:00</date>
  </header>

  <!-- NOTE: all coordinates and angles are relative to the reference point!
       By default, the reference point is turned to 90 degree, so take care to
       specify your coordinates and angles accordingly.
       For easier input, rotate the reference point manually to 0 degree
       (normally, a clockwise quarter rotation) -->
  <reproduction_setup>
    <!-- A reglar loudspeaker at output channel 1 -->
    <loudspeaker>
      <position x="1.111" y="2"/>
      <orientation azimuth="-130"/>
    </loudspeaker>

    <!-- skip 4 output channels of the soundcard -->
    <skip number="4"/>

    <!-- Due to the skip element the following circular loudspeaker arrangement
         occupies output channels 6 to 13.
         The attribute "name" is optional. -->
    <circular_array number="8" name="round array">
      <!-- if no center is given, the circle is centered around the origin -->
      <first>
        <!-- This is the first loudspeaker of the circular setup given relative
             to the origin.
             All other loudspeakers will be automatically placed equiangularly
             and counterclockwise along a circle centered at the origin. -->
        <position x="1.4" y="0"/>
        <orientation azimuth="180"/>
      </first>
    </circular_array>

    <!-- The following loudspeaker is a subwoofer at output channel 14 -->
    <loudspeaker model="subwoofer">
      <position x="1" y="-2"/>
      <orientation azimuth="-22184"/> <!-- angles are not limited to 0..360 -->
    </loudspeaker>

    <!-- A linear array at output channels 15 to 19 -->
    <linear_array number="5" name="linear array">
      <first>
        <position x="3" y="1"/>
        <orientation azimuth="180"/>
      </first>
      <last>
        <position x="3" y="-1"/>
        <!-- If omitted, the orientation of the first loudspeaker is used -->
      </last>
    </linear_array>

    <!-- A linear array with strange loudspeaker angles -->
    <linear_array number="5" name="strange linear array">
      <first>
        <position x="-1.21" y="-4.4"/>
        <orientation azimuth="80"/>
      </first>
      <!-- You can specify either the second or the last loudspeaker, the
           positions and orientations are interpolated (resp. extrapolated)
           accordingly -->
      <second>
        <position x="-1" y="-4"/>
        <orientation azimuth="20"/> <!-- optional -->
      </second>
    </linear_array>

    <!-- A circular arc -->
    <circular_array number="20" name="circular arc">
      <center>
        <position x="-1.5" y="0"/>
      </center>
      <first>
        <!-- The first loudspeaker must have position and orientation -->
        <position x="-3" y="-1.5"/>
        <orientation azimuth="45"/>
      </first>
      <!-- If no full circle is required, the angle of either the second or the
           last loudspeaker can be specified. The position and orientation of
           the remaining loudspeakers is determined automatically. -->
      <last>
        <angle azimuth="-90"/>
      </last>
    </circular_array>

  </reproduction_setup>
</asdf>
