FUDI-based Network Interface (for Pure Data)
============================================

.. warning::

    We did not evaluate any of the network interfaces in terms of security.
    So please be sure that you are in a safe network when using them.

The FUDI network interface can be activated with::

    ssr-binaural --fudi-server

This of course works for all renderers, not only ``ssr-binaural``.

The default port is 1174; you can choose a different port with::

    ssr-binaural --fudi-server=4321

The relevant settings in the :ref:`ssr_configuration_file` are::

    FUDI_INTERFACE = on
    FUDI_PORT = 4321


Controlling the SSR from Pure Data
----------------------------------

Have a look at the Pd patch :download:`ssrclient.pd <../../pd/ssrclient.pd>`
in the ``pd/`` directory.

Controlling the SSR from a Terminal
-----------------------------------

The FUDI interface is very simple and thanks to that
any tool that can send TCP/IP messages can be used to control the SSR.
One such tool is netcat_.

.. _netcat: https://en.wikipedia.org/wiki/Netcat

While the SSR is running (with the FUDI interface activated),
launch this command in a separate terminal window::

    nc localhost 1174

Now you can type FUDI messages (don't forget the semicolon at the end!)
which will be sent to the SSR when you press :kbd:`Return`.

If you want to receive messages from the SSR, use something like this::

    nc localhost 1174 <<< "subscribe scene; subscribe renderer;"
