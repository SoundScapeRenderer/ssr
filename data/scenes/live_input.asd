<?xml version="1.0" encoding="utf-8"?>
<asdf
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:noNamespaceSchemaLocation="asdf.xsd"
  version="0.1">

  <header>
    <name>4 live inputs</name>
    <description>
      This scene creates 4 sound sources and connects them
      to the first 4 inputs of your sound card (if available).
    </description>
  </header>

  <scene_setup>
    <source name="live input 1" model="point">
      <port>1</port>
      <position x="-1.5" y="2"/>
    </source>
    <source name="live input 2" model="point">
      <port>2</port>
      <position x="-0.5" y="2"/>
    </source>
    <source name="live input 3" model="point">
      <port>3</port>
      <position x="0.5" y="2"/>
    </source>
    <source name="live input 4" model="point">
      <port>4</port>
      <position x="1.5" y="2"/>
    </source>
  </scene_setup>
</asdf>
