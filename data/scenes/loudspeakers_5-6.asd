<?xml version="1.0" encoding="utf-8"?>
<asdf>
  <header>
    <name>.</name>
    <description>
      See the Sec. "Use cases" -> "Multiple Input Multiple Output" for an explanation of what this file does.
    </description>
  </header>

  <scene_setup>
    <volume>0</volume>

    <source name="left - ls 5,6" properties_file="impulse_responses/irs56_l.wav">
      <port>99</port>
      <position x=".25" y="2" fixed="true"/>
    </source>

    <source name="right - ls 5,6" properties_file="impulse_responses/irs56_r.wav">
      <port>99</port>
      <position x=".75" y="2" fixed="true"/>
    </source>
  </scene_setup>
</asdf>
