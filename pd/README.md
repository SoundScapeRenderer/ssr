Controlling the SSR with Pure Data
==================================

Pure Data (Pd) is a visual programming environment for audio and more.
If you don't know it, check it out: http://puredata.info/.

The SSR can be controlled from Pd via the built-in `[netsend]` object, which
uses the FUDI protocol (see https://en.wikipedia.org/wiki/FUDI).

For an example patch, see `ssrclient.pd`.

**Note**: The individual renderers of the SSR can also be directly used within
Pd as externals.  See the directory [../flext/](../flext/) for details.

The SSR is automatically compiled with FUDI support if the *Asio* and *fmt*
libraries are available.
To explicitly enable the feature, use:

    ./configure --enable-fudi-interface

The SSR has to be started with the `--fudi-server` flag, e.g.:

    ssr-binaural --fudi-server

By default, the port number 1174 is used, but you can specify a different port
with e.g.:

    ssr-binaural --fudi-server=3000

Alternatively, you can specify the settings in the SSR configuration file, e.g.:

    FUDI_INTERFACE = on
    FUDI_PORT = 3000

SSR Client for the Legacy IP-Interface (deprecated)
---------------------------------------------------

In this directory, there is a Pd external named `[legacy_ssrclient]`
(written in [Lua][])
that can be used to remote-control a running SSR instance from Pd using
the legacy network interface.

[Lua]: http://www.lua.org/

This is only provided for historic reference, you should really use the
aforementioned FUDI interface instead.

### Requirements

Pure Data with [pdlua][].

[pdlua]: http://puredata.info/downloads/pdlua

The `ssrclient` external uses the
[SLAXML XML parser for Lua](http://github.com/Phrogz/SLAXML)
(which is included).

### Usage

See the Pd patch `legacy_ssrclient-help.pd` for documentation.

The `[legacy_ssrclient]` external does not connect directly to the network,
because this
would need an external Lua library (which would have to be installed
separately).

Instead, it receives bytes (or lists of bytes) in its right inlet and produces
lists of bytes on its right outlet, which can be connected to the built-in
object `[netsend -b]`.
Alternatively, the `[tcpclient]`
external from `iemnet`/`mrpeach` (or any other external that has a similar
interface) can be used.

For documentation about the SSR's legacy network protocol, have a look at the
[SSR manual](https://ssr.readthedocs.io/network.html).
