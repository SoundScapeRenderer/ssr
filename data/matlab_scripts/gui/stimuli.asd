<?xml version="1.0" encoding="utf-8"?>
<asdf>
  <header>
    <name>User Study</name>
    <description>
      Contains 2 stimuli. Stimulus 2 is identical to 1 but 6 dB quieter.
    </description>
  </header>

  <scene_setup>
    <volume>-6</volume>

    <source name="Stimulus 1" properties_file="brirs.wav" volume="0" mute="true">
      <port>3</port>
      <position x="-1" y="3" fixed="true"/>
    </source>
    
    <source name="Stimulus 2" properties_file="brirs.wav" volume="-6" mute="true">
      <port>3</port>
      <position x="1" y="3" fixed="true"/>
    </source>
 
  </scene_setup>
</asdf>