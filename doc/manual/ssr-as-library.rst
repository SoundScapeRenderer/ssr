SSR as a Library
================

Until now, we were talking about the stand-alone SSR application
that uses the JACK audio backend.
But all renderers can also be used as a library in any C++ project,
using any audio backend (or none).

Only the bare renderers are available;
no GUI, no network interface, no scene files, no head trackers.
Multi-threading is still available!

The easiest way to use an SSR renderer as a library
is to include the SSR repository as a Git submodule into your own project.
For more details, have a look at the examples in the SSR repository:

* Pure Data externals,
  see `flext <https://github.com/SoundScapeRenderer/ssr/tree/master/flext>`_
  directory
* MEX files for Octave/Matlab,
  see `mex <https://github.com/SoundScapeRenderer/ssr/tree/master/mex>`_
  directory
