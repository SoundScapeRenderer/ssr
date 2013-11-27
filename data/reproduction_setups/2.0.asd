<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet type="text/xsl" href="asdf2html.xsl"?>
<asdf xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:noNamespaceSchemaLocation="asdf.xsd">
  <header>
    <name>2.0 stereo setup</name>
    <description>
      standard stereo setup at 1.5m distance
      output channel 1: left speaker
      output channel 2: right speaker
    </description>
  </header>

  <reproduction_setup>
    <loudspeaker>
      <!-- position is relative to the reference point! -->
      <position x="1.3" y="+0.75"/>
      <orientation azimuth="-150"/>
    </loudspeaker>
    <loudspeaker>
      <!-- position is relative to the reference point! -->
      <position x="1.3" y="-0.75"/>
      <orientation azimuth="-210"/>
    </loudspeaker>
  </reproduction_setup>
</asdf>
