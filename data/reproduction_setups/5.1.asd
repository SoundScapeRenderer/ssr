<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet type="text/xsl" href="asdf2html.xsl"?>
<asdf xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:noNamespaceSchemaLocation="asdf.xsd">
  <header>
    <name>5.1 surround setup</name>
    <description>
      standard 5.1 setup
    </description>
  </header>

  <reproduction_setup>
    <loudspeaker>
      <!-- left, position is relative to the reference point! -->
      <position x="1.3" y="+0.75"/>
      <orientation azimuth="-150"/>
    </loudspeaker>
    <loudspeaker>
      <!-- center, position is relative to the reference point! -->
      <position x="1.5" y="0"/>
      <orientation azimuth="-180"/>
    </loudspeaker>
    <loudspeaker>
      <!-- right, position is relative to the reference point! -->
      <position x="1.3" y="-0.75"/>
      <orientation azimuth="-210"/>
    </loudspeaker>
    <loudspeaker>
      <!-- left sur, position is relative to the reference point! -->
      <position x="-0.5" y="+1.41"/>
      <orientation azimuth="-70"/>
    </loudspeaker>
    <loudspeaker>
      <!-- right sur, position is relative to the reference point! -->
      <position x="-0.5" y="-1.41"/>
      <orientation azimuth="+70"/>
    </loudspeaker>
    <loudspeaker model="subwoofer">
      <position x="0" y="0"/>
      <orientation azimuth="0"/>
    </loudspeaker>
  </reproduction_setup>
</asdf>
