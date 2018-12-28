Controlling the SSR with Pure Data
==================================

Pure Data (Pd) is a visual programming environment for audio and more.
If you don't know it, check it out: http://puredata.info/.

In this directory, there is a Pd external named `ssrclient` (written in [Lua][])
that can be used to remote-control a running SSR instance from within Pd using
the network interface.

**Note**: The individual renderers of the SSR can also be directly used within
Pd as externals.  See the directory [`../flext/`](../flext/) for details.

[Lua]: http://www.lua.org/

Requirements
------------

Pure Data with [pdlua][] and the `tcpclient` external from [iemnet][] or
[mrpeach][].
If you use [Pd-extended][], no further installation is necessary because
those libraries are already included.

[pdlua]: http://puredata.info/downloads/pdlua
[iemnet]: http://puredata.info/downloads/iemnet
[mrpeach]: http://pure-data.cvs.sourceforge.net/pure-data/externals/mrpeach/net/
[Pd-extended]: http://puredata.info/downloads/pd-extended

The `ssrclient` external uses the
[SLAXML XML parser for Lua](http://github.com/Phrogz/SLAXML).

Usage
-----

See the Pd patch `ssrclient-help.pd` for documentation.

The `ssrclient` external does not connect directly to the network, because this
would need an external Lua library (which would have to be installed
separately).

Instead, it receives bytes (or lists of bytes) in its right inlet and produces
lists of bytes on its right outlet, which can be connected to the `tcpclient`
external from `iemnet`/`mrpeach` (or any other external that has a similar
interface).

For documentation about the SSR's network protocol, have a look at the
[SSR manual](http://ssr.rtfd.org/en/latest/network.html).
