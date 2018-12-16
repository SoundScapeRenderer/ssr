// workflows

// sclang is a client, controlling a SSR server instance
(
  // set address of the client instance
  ~address = NetAddr("localhost", 50001);
  // print all OSC messages sent to sclang
  OSCFunc.trace(true, true);
  // subscribe to server with MessageLevel::SERVER
  ~address.sendMsg("/subscribe", $T, 2);
  // add new source with standard input at -1.0/1.0
  ~address.sendMsg("/source/new", "in_1", "point", "1",
    -1.0, 1.0, 0.1, 0.1, 0, "1", $F, $F, $T);
  // add new source with standard input at 1.0/1.0
  ~address.sendMsg("/source/new", "in_2", "point", "2",
    1.0, 1.0, 0.1, 0.1, 0, "1", $F, $F, $T);
  // unmute source 1
  ~address.sendMsg("/source/mute", 1, $F);
  // unmute source 2
  ~address.sendMsg("/source/mute", 2, $F);
  // move source 1 to -2.0/2.0
  ~address.sendMsg("/source/position", 1, -2.0, 2.0);
  // move source 2 to 2.0/2.0
  ~address.sendMsg("/source/position", 2, 2.0, 2.0);
  // remove all sources
  ~address.sendMsg("/scene/clear");
  // unsubscribe from server
  ~address.sendMsg("/subscribe", $F);
)

// sclang is a server, controlling a SSR client instance
(
  // set address of the client instance
  ~address = NetAddr("localhost", 50001);
  // print all OSC messages sent to sclang
  OSCFunc.trace(true, true);
  // poll client instance to make it subscribe
  ~address.sendMsg("/poll");
  // subsequent poll makes client emit /alive message
  ~address.sendMsg("/poll");
  // add new source with standard input at -1.0/1.0
  ~address.sendMsg("/source/new", "in_1", "point", "1",
    -1.0, 1.0, 0.1, 0.1, 0, "1", $F, $F, $T);
  // add new source with standard input at 1.0/1.0
  ~address.sendMsg("/source/new", "in_2", "point", "2",
    1.0, 1.0, 0.1, 0.1, 0, "1", $F, $F, $T);
  // unmute source 1
  ~address.sendMsg("/source/mute", 1, $F);
  // unmute source 2
  ~address.sendMsg("/source/mute", 2, $F);
  // set message level to GUI_SERVER (a lot of messages!)
  ~address.sendMsg("/message_level", 3);
  // move source 1 to -2.0/2.0
  ~address.sendMsg("/source/position", 1, -2.0, 2.0);
  // move source 2 to 2.0/2.0
  ~address.sendMsg("/source/position", 2, 2.0, 2.0);
  // set message level back to SERVER
  ~address.sendMsg("/message_level", 1);
  // remove all sources
  ~address.sendMsg("/scene/clear");
)

// sclang is a client, controlling a SSR server instance, while forwarding OSC
// commands from a smartphone, that uses Sensors2OSC (https://sensors2.org/osc/)
(
  // set address of the client instance
  ~address = NetAddr("localhost", 50001);
  // print all OSC messages sent to sclang
  // subscribe to server with MessageLevel::SERVER
  ~address.sendMsg("/subscribe", $T, 2);
  // add new source with standard input at -1.0/1.0
  ~address.sendMsg("/source/new", "in_1", "point", "1", -1.0, 1.0, 0.0, 0.99,
    $F, $F, $F);
  // add new source with standard input at 1.0/1.0
  ~address.sendMsg("/source/new", "in_2", "point", "2", 1.0, 1.0, 0.0, 0.99,
    $F, $F, $F);
  // vectors for holding source position
  ~source1 = [-1.0, -1.0];
  ~source2 = [-1.0, -1.0];
  // OSC functions to receive from Sensors2OSC and send to SSR
  OSCFunc(
    {
      arg msg, time, addr, recvPort;
      ~address.sendMsg("/alive");
    },
    '/poll'
  );
  OSCFunc(
    {
      arg msg, time, addr, recvPort;
      ~address.sendMsg("/reference/orientation", msg[1]+90);
    },
    '/orientation/X'
  );
  OSCFunc(
    {
      arg msg, time, addr, recvPort;
      if(msg[1]>=0.0, {
        ~source1[0] = msg[1];
      });
      if(msg[0] != (-1.0) && msg[1] != (-1.0), {
        ~address.sendMsg("/source/position", 1,
          ~source1[0].linlin(0.0, 1.0, -10.0, 10.0),
          ~source1[1].linlin(0.0, 1.0, -10.0, 10.0)
        );
      });
    },
    '/touch1/X'
  );
  OSCFunc(
    {
      arg msg, time, addr, recvPort;
      if(msg[1]>=0.0, {
        ~source1[1] = msg[1];
      });
      if(msg[0] != (-1.0) && msg[1] != (-1.0), {
        ~address.sendMsg("/source/position", 1,
          ~source1[0].linlin(0.0, 1.0, -10.0, 10.0),
          ~source1[1].linlin(0.0, 1.0, -10.0, 10.0)
        );
      });
    },
    '/touch1/Y'
  );
  OSCFunc(
    {
      arg msg, time, addr, recvPort;
      if(msg[1]>=0.0, {
        ~source2[0] = msg[1];
      });
      if(msg[0] != (-1.0) && msg[1] != (-1.0), {
        ~address.sendMsg("/source/position", 2,
          ~source2[0].linlin(0.0, 1.0, -10.0, 10.0),
          ~source2[1].linlin(0.0, 1.0, -10.0, 10.0)
        );
      });
    },
    '/touch2/X'
  );
  OSCFunc(
    {
      arg msg, time, addr, recvPort;
      if(msg[1]>=0.0, {
        ~source2[1] = msg[1];
      });
      if(msg[0] != (-1.0) && msg[1] != (-1.0), {
        ~address.sendMsg("/source/position", 2,
          ~source2[0].linlin(0.0, 1.0, -10.0, 10.0),
          ~source2[1].linlin(0.0, 1.0, -10.0, 10.0)
        );
      });
    },
    '/touch2/Y'
  );
)

// clear scene and unsubscribe from server instance
(
  // remove all sources
  ~address.sendMsg("/scene/clear");
  // unsubscribe from server
  ~address.sendMsg("/subscribe", $F);
)

