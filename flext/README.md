Using the SSR as external(s) in Pure Data
=========================================

[Pure Data (Pd)][Pd] is a visual programming environment for audio and more.

[Pd]: http://puredata.info/

The SSR externals use exactly the same audio processing code as the stand-alone
SSR application and they support multi-threading.
However, they don't have a GUI, they don't have a network interface and they
cannot load audio scenes.
Source positions and other parameters can be controlled by sending messages to
the leftmost inlet of the external.

Each renderer is available as a separate external, namely
`ssr_binaural~`, `ssr_brs~`, `ssr_dca~`, `ssr_aap~`, `ssr_wfs~` and `ssr_vbap~`.


Requirements
------------

Get the submodules:

    git submodule update --init

Once those are available, the SSR externals can be compiled with:

    make

By default, all available renderers are built. If you only want some of them,
you can use something like this:

    make ssr_binaural~ ssr_wfs~


Usage
-----

See the files `ssr_*-help.pd` for how to use the SSR externals.

The externals support up to two numeric arguments and up to two string
arguments (`ssr_brs~` expects different arguments, see below).
The first numeric argument specifies the number of sources (default: 1),
the second argument sets the number of threads to be used
(default: as many as processor cores).
The string arguments are renderer-specific. The loudspeaker-based renderers
expect the name of the reproduction setup file, the binaural renderer
(`ssr_binaural~`) expects the name of the HRIR file.
The WFS renderer (`ssr_wfs~`) expects a second string argument with the name of
the prefilter file.

The `ssr_brs~` external does *not* expect a numeric argument for the number of
sources.  Instead, a list of BRIR file names can be specified, each of which
will create a source.
The number of threads can be specified as an optional first argument
(again, by default as many threads as available processors are used).


Supported Platforms
-------------------

All externals *should* work with Pd on Linux, macOS and Windows.  If not,
please open an issue at <https://github.com/SoundScapeRenderer/ssr/issues>.
