#!/bin/bash

# See the documentation, Sec. "Use Cases", for an explanation of what this script is doing.

echo IMPORTANT: Make sure that you adapt the paths to the asdf files inside this shell script to your local setup!

# Start SSR 1, loudspeakers 1-2
open -n -a SoundScapeRenderer --args --brs "--fudi-server=1147 --no-websocket-server --input-prefix=XXX:XXX --output-prefix=YYY:YYY --name=loudspeakers_1-2 ~/Documents/misc/ssr/loudspeakers_1-2.asd"

# Start SSR 2, loudspeakers 3-4
open -n -a SoundScapeRenderer --args --brs "--fudi-server=1148 --no-websocket-server --input-prefix=XXX:XXX --output-prefix=YYY:YYY --name=loudspeakers_3-4 ~/Documents/misc/ssr/loudspeakers_3-4.asd"

# Start SSR 3, loudspeakers 4-5
open -n -a SoundScapeRenderer --args --brs "--fudi-server=1149 --no-websocket-server --input-prefix=XXX:XXX --output-prefix=YYY:YYY --name=loudspeakers_5-6 ~/Documents/misc/ssr/loudspeakers_5-6.asd"

# Start SSR 4, loudspeakers 6-7
open -n -a SoundScapeRenderer --args --brs "--fudi-server=1150 --no-websocket-server --input-prefix=XXX:XXX --output-prefix=YYY:YYY --name=loudspeakers_7-8 ~/Documents/misc/ssr/loudspeakers_7-8.asd"

sleep 5 

jack_connect loudspeakers_1-2:out_1 system:playback_1
jack_connect loudspeakers_1-2:out_2 system:playback_2 

jack_connect loudspeakers_3-4:out_1 system:playback_3
jack_connect loudspeakers_3-4:out_2 system:playback_4

jack_connect loudspeakers_5-6:out_1 system:playback_5
jack_connect loudspeakers_5-6:out_2 system:playback_6

jack_connect loudspeakers_7-8:out_1 system:playback_7
jack_connect loudspeakers_7-8:out_2 system:playback_8


exit 0