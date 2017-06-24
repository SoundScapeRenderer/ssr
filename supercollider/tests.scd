// tests

// evaluate block to add the test functions
(

~processingTestRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  20.do({address.sendMsg("/processing/state", [$F, $T].choose)});
};

~referenceTestRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  100.do({address.sendMsg("/reference/orientation", 360.0.rand2)});
  20.do({address.sendMsg("/reference/position", 10.0.rand2, 10.0.rand2)});
  100.do({address.sendMsg("/reference_offset/orientation", 360.0.rand2)});
  20.do({address.sendMsg("/reference_offset/position", 10.0.rand2, 10.0.rand2)});
};

~sceneTestRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  20.do({address.sendMsg("/scene/amplitude_reference_distance", 10.0.rand2)});
  10.do({address.sendMsg("/scene/auto_rotate_sources", [$F, $T].choose)});
  address.sendMsg("/scene/clear");
  20.do({address.sendMsg("/scene/decay_exponent", rrand(-1, 10))});
  address.sendMsg("/scene/load", "some_file.asdf");
  address.sendMsg("/scene/save", "some_file.asdf");
  100.do({address.sendMsg("/scene/master_signal_level", rrand(-100.0, 40.0))});
  100.do({address.sendMsg("/scene/volume", rrand(-100.0, 40.0))});
};

~sourceTestRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  20.do({address.sendMsg("/source/delete", 10.rand2)});
  20.do({address.sendMsg("/source/file_channel", 10.rand2, 10.rand)});
  20.do({address.sendMsg("/source/file_name_or_port_number", 10.rand2, 10.rand2.asString)});
  20.do({address.sendMsg("/source/gain", 10.rand2, rrand(-100.0, 40.0))});
  100.do({address.sendMsg("/source/level", 10.rand2, rrand(-100.0, 40.0))});
  20.do({address.sendMsg("/source/model", 10.rand2, ["unknown", "point", "plane", "line", "directional", "extended"].choose)});
  20.do({address.sendMsg("/source/mute", 10.rand2, [$F, $T].choose)});
  20.do({address.sendMsg("/source/name", 10.rand2, ("source_"++10.rand2.asString))});
  20.do({address.sendMsg("/source/new", ("source_"++10.rand2.asString), ["unknown",
    "point", "plane", "line", "directional", "extended"].choose,
    10.rand2.asString, 10.0.rand2, 10.0.rand2, rrand(-100.0, 40.0), rrand(-100.0, 40.0),
    [$F, $T].choose, [$F, $T].choose, [$F, $T].choose)
  });
  20.do({address.sendMsg("/source/new", ("source_"++10.rand2.asString), ["unknown",
    "point", "plane", "line", "directional", "extended"].choose,
    10.rand2.asString, 10.0.rand2, 10.0.rand2, rrand(-100.0, 40.0), rrand(-100.0, 40.0),
    10.rand2, ("file_"++10.rand2.asString), [$F, $T].choose, [$F, $T].choose,
    [$F, $T].choose)
  });
  20.do({address.sendMsg("/source/orientation", 10.rand2, 360.0.rand2)});
  20.do({address.sendMsg("/source/port_name", 10.rand2, 10.rand2.asString)});
  20.do({address.sendMsg("/source/position", 10.rand2, 10.0.rand2, 10.0.rand2)});
  20.do({address.sendMsg("/source/position_fixed", 10.rand2, [$F, $T].choose)});
  20.do({address.sendMsg("/source/properties_file", 10.rand2, 10.rand2.asString)});
};

~trackerTestRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  10.do({address.sendMsg("/tracker/reset")});
};

~transportTestRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  100.do({address.sendMsg("/transport/rewind")});
  100.do({address.sendMsg("/transport/seek", (10.rand2.asString++":"++59.rand2.asString++":"++59.0.rand2.asString))});
  100.do({address.sendMsg("/transport/state", [$F, $T].choose)});
};

~updateTestRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  100.do({address.sendMsg("/update/cpu_load", 100.0.rand)});
  100.do({address.sendMsg("/update/processing/state", [$F, $T].choose)});
  100.do({address.sendMsg("/update/reference/orientation", 360.0.rand2)});
  20.do({address.sendMsg("/update/reference/position", 10.0.rand2, 10.0.rand2)});
  20.do({address.sendMsg("/update/reference_offset/orientation", 360.0.rand2)});
  20.do({address.sendMsg("/update/reference_offset/position", 10.0.rand2, 10.0.rand2)});
  20.do({address.sendMsg("/update/scene/amplitude_reference_distance", 10.0.rand2)});
  20.do({address.sendMsg("/update/scene/auto_rotate_sources", [$F, $T].choose)});
  20.do({address.sendMsg("/update/scene/decay_exponent", rrand(-1.0, 10.0))});
  20.do({address.sendMsg("/update/scene/master_signal_level", rrand(-100.0, 40.0))});
  20.do({address.sendMsg("/update/scene/sample_rate", [22050, 44100, 48000, 88200, 96000].choose)});
  20.do({address.sendMsg("/update/scene/volume", rrand(-100.0, 40.0))});
  20.do({address.sendMsg("/update/source/delete", 10.rand2)});
  20.do({address.sendMsg("/update/source/file_channel", 10.rand2, 10.rand2)});
  20.do({address.sendMsg("/update/source/file_name_or_port_number", 10.rand2, 10.rand2.asString)});
  20.do({address.sendMsg("/update/source/gain", 10.rand2, rrand(-100.0, 40.0))});
  20.do({address.sendMsg("/update/source/level", 10.rand2, rrand(-100.0, 40.0))});
  20.do({address.sendMsg("/update/source/model", 10.rand2, ["unknown", "point", "plane", "line", "directional", "extended"].choose)});
  20.do({address.sendMsg("/update/source/mute", 10.rand2, [$F, $T].choose)});
  20.do({address.sendMsg("/update/source/name", 10.rand2, ("source_"++10.rand2.asString))});
  20.do({address.sendMsg("/update/source/new", 10.rand2)});
  20.do({address.sendMsg("/update/source/orientation", 10.rand2, 360.0.rand2)});
  20.do({address.sendMsg("/update/source/port_name", 10.rand2, ("in_"++10.rand2))});
  20.do({address.sendMsg("/update/source/position", 10.rand2, 10.0.rand2, 10.0.rand2)});
  20.do({address.sendMsg("/update/source/position_fixed", 10.rand2, [$F, $T].choose)});
  20.do({address.sendMsg("/update/source/properties_file", 10.rand2, ("file_"++10.rand2.asString))});
  20.do({address.sendMsg("/update/transport/seek", (10.rand2.asString++":"++59.rand2.asString++":"++59.0.rand2.asString))});
  100.do({address.sendMsg("/update/transport/state", [$F, $T].choose)});
};

~messageLevelTestServerRandomAll = { arg address = NetAddr("127.0.0.1", 50001), otherClient = NetAddr("127.0.0.1", 50002);
  20.do({address.sendMsg("/message_level", 10.rand2)});
  20.do({address.sendMsg("/message_level", otherClient.ip.asString, otherClient.port.asString, 10.rand2)});
};

~subscribeTestOtherClient = { arg address = NetAddr("127.0.0.1", 50001), otherClient = NetAddr("127.0.0.1", 50002);
  10.do({
    address.sendMsg("/subscribe", [$F, $T].choose, otherClient.ip, otherClient.port);
    address.sendMsg("/subscribe", $T, otherClient.ip, otherClient.port, 10.rand2);
    address.sendMsg("/subscribe", [$F, $T].choose);
    address.sendMsg("/subscribe", $T, 10.rand2);
    address.sendMsg("/subscribe", $F);
  });
};

~cpuLoadTestClient = { arg address = NetAddr("127.0.0.1", 50001);
  100.do({address.sendMsg("/cpu_load", 100.0.rand)});
};

~messageLevelTestClientRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  20.do({address.sendMsg("/message_level", 10.rand2)});
};

~pollTestClientRandomAll = { arg address = NetAddr("127.0.0.1", 50001);
  100.do({address.sendMsg("/poll")});
};

~sourceTestMoving = { arg address = NetAddr("127.0.0.1", 50001), amountOfSources = 2;
  amountOfSources.do({|item,i|
    address.sendMsg("/source/new", ("source_"++(i+1).asString), ["unknown",
      "point", "plane", "line", "directional", "extended"].choose,
      (i+1).asString, 10.0.rand2, 10.0.rand2, rrand(-100.0, 40.0),
      rrand(-100.0, 40.0), [$F, $T].choose, [$F, $T].choose, [$F,
        $T].choose);
  });
  amountOfSources.do({ |item, i|
    address.sendMsg("/source/position_fixed", i+1, $F);
    address.sendMsg("/source/mute", i+1, $F);
  });
  100.do({
    amountOfSources.do({|item, i|
      address.sendMsg("/source/position", i+1, 10.0.rand2, 10.0.rand2);
    });
  });
};

)


// sclang tries to control/send to server (not subscribed)
(
  // print all OSC messages sent to sclang
  OSCFunc.trace(true, true);
  ~messageLevelTestServerRandomAll.value;
  ~sourceTestRandomAll.value;
  ~updateTestRandomAll.value;
  ~subscribeTestOtherClient.value;
  ~processingTestRandomAll.value;
  ~transportTestRandomAll.value;
  ~trackerTestRandomAll.value;
  ~referenceTestRandomAll.value;
  ~sceneTestRandomAll.value;
)

// sclang controls server (subscribed)
(
  // print all OSC messages sent to sclang
  OSCFunc.trace(true, true);
  // set address of the server instance
  ~address = NetAddr("127.0.0.1", 50001);
  // subscribe to server with MessageLevel::SERVER
  ~address.sendMsg("/subscribe", $T, 2);
  // send alive message on subsequent poll
  ~responder_poll = OSCFunc(
    { |msg, time, addr, recvPort|
      ~address.sendMsg("/alive");
    }, '/poll'
    , ~address
  );
  ~messageLevelTestServerRandomAll.value;
  ~sourceTestRandomAll.value;
  ~updateTestRandomAll.value;
  ~processingTestRandomAll.value;
  ~transportTestRandomAll.value;
  ~trackerTestRandomAll.value;
  ~referenceTestRandomAll.value;
  ~sceneTestRandomAll.value;
)

// sclang tries to control client (not polled)
(
  // print all OSC messages sent to sclang
  OSCFunc.trace(true, true);
  ~messageLevelTestClientRandomAll.value;
  ~cpuLoadTestClient.value;
  ~sourceTestRandomAll.value;
  ~processingTestRandomAll.value;
  ~transportTestRandomAll.value;
  ~trackerTestRandomAll.value;
  ~referenceTestRandomAll.value;
  ~sceneTestRandomAll.value;
)

// sclang tries to control client (polled)
(
  // print all OSC messages sent to sclang
  OSCFunc.trace(true, true);
  // set address of the server instance
  ~address = NetAddr("127.0.0.1", 50001);
  ~pollTestClientRandomAll.value;
  ~messageLevelTestClientRandomAll.value;
  ~cpuLoadTestClient.value;
  ~sourceTestRandomAll.value;
  ~processingTestRandomAll.value;
  ~transportTestRandomAll.value;
  ~trackerTestRandomAll.value;
  ~referenceTestRandomAll.value;
  ~sceneTestRandomAll.value;
)

// sclang controls a client (polled), adds sources and moves them
(
  // print all OSC messages sent to sclang
  OSCFunc.trace(true, true);
  // set address of the server instance
  ~address = NetAddr("127.0.0.1", 50001);
  // poll client instance to make it subscribe
  ~address.sendMsg("/poll");
  ~sourceTestMoving.value(amountOfSources: 20);
)

