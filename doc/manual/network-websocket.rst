WebSocket-based Network Interface
=================================

.. warning::

    We did not evaluate any of the network interfaces in terms of security.
    So please be sure that you are in a safe network when using them.

The WebSocket network interface is started by default when starting the SSR.

Once the SSR is running, you can use your browser to connect to
http://localhost:9422/test.
This will show a simple test client which you can use to check whether
everything is working.

If you want to try out the :doc:`browser-gui`, use the address
http://localhost:9422.

By default, the port number 9422 is used.
You can choose a different port with::

    ssr-binaural --websocket-server=4321

This of course works for all renderers, not only ``ssr-binaural``.

The relevant settings in the :ref:`ssr_configuration_file` are::

    WEBSOCKET_INTERFACE = on
    WEBSOCKET_PORT = 4321
