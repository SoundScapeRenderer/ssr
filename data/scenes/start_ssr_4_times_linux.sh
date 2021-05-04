#!/bin/bash

# See the documentation, Sec. "Use Cases", for an explanation of what this script is doing.


# Start SSR 1, loudspeakers 1-2
ssr-brs --fudi-server=1147 --no-websocket-server --input-prefix=XXX:XXX --output-prefix=YYY:YYY --name=loudspeakers_1-2 loudspeakers_1-2.asd &

# Start SSR 2, loudspeakers 3-4
ssr-brs --fudi-server=1148 --no-websocket-server --input-prefix=XXX:XXX --output-prefix=YYY:YYY --name=loudspeakers_3-4 loudspeakers_3-4.asd &

# Start SSR 3, loudspeakers 4-5
ssr-brs --fudi-server=1149 --no-websocket-server --input-prefix=XXX:XXX --output-prefix=YYY:YYY --name=loudspeakers_5-6 loudspeakers_5-6.asd &

# Start SSR 4, loudspeakers 6-7
ssr-brs --fudi-server=1150 --no-websocket-server --input-prefix=XXX:XXX --output-prefix=YYY:YYY --name=loudspeakers_7-8 loudspeakers_7-8.asd &

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