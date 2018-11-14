Using the SSR as external(s) in Pure Data (and possibly Max)
============================================================

[Pure Data (Pd)][Pd] is a visual programming environment for audio and more.
[Max][] is basically the same thing, except much shinier and non-free.

[Pd]: http://puredata.info/
[Max]: http://cycling74.com/products/max/

The SSR externals use exactly the same audio processing code as the stand-alone
SSR application and they support multi-threading.
However, they don't have a GUI, they don't have a network interface and they
cannot load audio scenes.
Source positions and other parameters can be controlled by sending messages to
the leftmost inlet of the external.

Each renderer is available as a separate external, namely
`ssr_binaural~`, `ssr_dca~`, `ssr_aap~`, `ssr_wfs~` and `ssr_vbap~`.

Requirements
------------

Before being able to compile the SSR externals, you need to download and install
the [flext][] library.
The flext library is really great, but it can be a bit complicated to install.
See the files [readme.txt][] and [build.txt][] for instructions.

[flext]: http://grrrr.org/research/software/flext/
[readme.txt]: https://svn.grrrr.org/ext/trunk/flext/readme.txt
[build.txt]: https://svn.grrrr.org/ext/trunk/flext/build.txt

Once flext is built, the SSR externals can be compiled with:

    make

This assumes that flext lives in `/usr/local/src/flext`. If that's not the case,
you can specify the path with:

    make FLEXTPATH="/path/to/flext"

By default, all available renderers are built. If you only want some of them,
you can use something like this:

    make EXTERNALS="ssr_binaural ssr_wfs"

Usage
-----

See the files `ssr_*-help.pd` for how to use the SSR externals.

The externals support up to two numeric arguments and up to two string
arguments.
The first numeric argument specifies the number of sources (default 1), the
second argument sets the number of threads to be used (default 1).
The string arguments are renderer-specific. The loudspeaker-based renderers
expect the name for the reproduction setup file, the binaural renderer
(`ssr_binaural~`) expects the name of the HRIR file.
The WFS renderer (`ssr_wfs~`) expects a second string argument with the name of
the prefilter file.

Supported Platforms
-------------------

Currently, the externals were only used with Pd on Linux.
With minor adjustments in the Makefile they should also work for Pd on Mac OS X.
Theoretically they could also work on Windows, but probably some further
modifications are necessary.
With a bit of luck, the externals might even work on Max.

<!--
vim:textwidth=80
-->
