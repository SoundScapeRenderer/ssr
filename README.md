[![Build Status](https://travis-ci.com/chris-hld/ssr.svg?branch=ssr-windows)](https://travis-ci.com/chris-hld/ssr)

This is a fork of the SoundScape Renderer ([SSR](https://github.com/SoundScapeRenderer/ssr)), which aims at porting the source code to Windows.

The basic idea is to cross-compile on a linux machine an executable (.exe) for Windows.
We use the M cross environment ([MXE](http://mxe.cc/)), which provides cross-compilation scripts for the dependencies and the MinGW64 (GCC) compiler.

Pre-compiled binaries for Windows currently include:   
 `ssr-binaural.exe`, `ssr-brs.exe`, `ssr-vbap.exe`, `ssr-wfs.exe`, `ssr-aap.exe`, `ssr-dca.exe` and `ssr-generic.exe`.

They should run on all x86-64 CPUs. 
So far, only the Razor and Polhemus head-tracker are ported (and I can only test the recent Razor).


Getting started
---------------
* Install Jack Audio for Windows (64bit) from [Jack Audio Downloads](http://www.jackaudio.org/downloads/).
* Get it running by following the [Installation and Configuration](http://jackaudio.org/faq/jack_on_windows.html).
  Hint: If you plan on using other (ASIO) Audio-software with Jack Audio, don't forget to register the 64-bit JackRouter.
* Download and extract the latest [Release](https://github.com/chris-hld/ssr/releases).
It contains everything you need, including the cross-compiled executables.
* Adapt the configuration file `ssr.conf.windows` to your needs. Replace the paths (`Z:\SSR`) to yours. For now, please use absolute paths.

Running the SSR on Windows
--------------------------
* Have a look at the [Documentation](http://ssr.readthedocs.io/en/latest/operation.html#running-the-ssr)
* First, start the Jack Audio Server, e.g. with `qjackctl.exe`.
* Start the SSR. There is a simple example script `start-ssr.cmd`.
* Optional: Start your ASIO playback Software.
* Connect everything using Jack Audio Connection.

This could look like this:

![Jack](doc/manual/images/screenshot_jack.png)
![SSR-Windows](doc/manual/images/screenshot_ssr-windows.png)


---

This is the source distribution of SoundScape Renderer (SSR) licensed under the
GPLv3+. Please consult the file COPYING for more information about this license.
 
The user manual in the doc/ directory contains relevant informations about the
SSR, including installation instructions. Additional (very detailed)
installation instructions can be found in the file INSTALL.

For questions, bug reports and feature requests:

Contact:       ssr@spatialaudio.net
Website:       http://spatialaudio.net/ssr/
Documentation: http://ssr.readthedocs.io/en/latest/

Copyright (c) 2016-2018 Division of Applied Acoustics
                        Chalmers University of Technology
                        
Copyright (c) 2012-2014 Institut für Nachrichtentechnik, Universität Rostock

Copyright (c) 2006-2012 Quality & Usability Lab
Deutsche Telekom Laboratories, TU Berlin

