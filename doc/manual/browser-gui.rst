.. _browser-based_gui:

Browser-based GUI
=================

.. warning::

    Currently, the browser GUI is only a highly experimental prototype.
    It should be usable, but there are many missing features and other
    possibilities for improvements.

    As always, contributions are very welcome!

Using the SSR's :doc:`network-websocket`,
it is possible to control the SSR from an off-the-shelf web browser.
Just make sure the SSR is running (with the WebSocket feature enabled)
and visit the URL http://localhost:9422.

If you just want to test the WebSocket connection,
you can visit http://localhost:9422/test.

You can of course also access the SSR if it is running on a different computer
in the network, by simply using the appropriate host name or IP address instead
of ``localhost``.
