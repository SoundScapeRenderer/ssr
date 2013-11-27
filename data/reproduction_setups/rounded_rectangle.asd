<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet type="text/xsl" href="asdf2html.xsl"?>
<asdf xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:noNamespaceSchemaLocation="asdf.xsd"
      version="0.1">
  <header>
    <name>A rounded rectangle array</name>
    <description>
      This is not an actually existing loudspeaker array. It is only used to
      show the possibility of constructing an array from linear and circle
      segment pieces.
    </description>
  </header>

  <reproduction_setup>
    <linear_array number="8" name="linear segment front left">
      <first>
        <position x="1.4775" y="0"/>
        <orientation azimuth="180"/>
      </first>
      <second>
        <position x="1.4775" y=".25"/>
      </second>
    </linear_array>
    <circular_array number="4" name="quarter circle front left">
      <center>
        <position x="1" y="2"/>
      </center>
      <first>
        <position x="1.4775" y="2"/>
        <orientation azimuth="-180"/>
      </first>
      <last>
        <angle azimuth="90"/>
      </last>
    </circular_array>
    <linear_array number="7" name="linear segment left">
      <first>
        <position x=".75" y="2.4775"/>
        <orientation azimuth="-90"/>
      </first>
      <second>
        <position x=".5" y="2.4775"/>
      </second>
    </linear_array>
    <circular_array number="4" name="quarter circle rear left">
      <center>
        <position x="-1" y="2"/>
      </center>
      <first>
        <position x="-1" y="2.4775"/>
        <orientation azimuth="-90"/>
      </first>
      <last>
        <angle azimuth="90"/>
      </last>
    </circular_array>
    <linear_array number="15" name="linear segment rear">
      <first>
        <position x="-1.4775" y="1.75"/>
        <orientation azimuth="0"/>
      </first>
      <second>
        <position x="-1.4775" y="1.5"/>
      </second>
    </linear_array>
    <circular_array number="4" name="quarter circle rear right">
      <center>
        <position x="-1" y="-2"/>
      </center>
      <first>
        <position x="-1.4775" y="-2"/>
        <orientation azimuth="0"/>
      </first>
      <last>
        <angle azimuth="90"/>
      </last>
    </circular_array>
    <linear_array number="7" name="linear segment right">
      <first>
        <position x="-.75" y="-2.4775"/>
        <orientation azimuth="90"/>
      </first>
      <second>
        <position x="-.5" y="-2.4775"/>
      </second>
    </linear_array>
    <circular_array number="4" name="quarter circle front right">
      <center>
        <position x="1" y="-2"/>
      </center>
      <first>
        <position x="1" y="-2.4775"/>
        <orientation azimuth="90"/>
      </first>
      <last>
        <angle azimuth="90"/>
      </last>
    </circular_array>
    <linear_array number="7" name="linear segment front right">
      <first>
        <position x="1.4775" y="-1.75"/>
        <orientation azimuth="180"/>
      </first>
      <second>
        <position x="1.4775" y="-1.5"/>
      </second>
    </linear_array>
  </reproduction_setup>
</asdf>
