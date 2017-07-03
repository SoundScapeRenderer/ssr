Note that the linked libraries (libxml2, libsndfile and flext) and the SSR
external have to be compiled with the same compiler and the same C++ standard
library.
In the following instructions GCC is used for everything.

* Install homebrew like described on http://brew.sh/

* Install the GNU compiler GCC

      brew install gcc

* Get the Max SDK from https://cycling74.com/downloads/sdk/
  Newer versions might not work with flext, so you should choose an old one,
  e.g. version 5.x: http://www.cycling74.com/download/MaxSDK-5.1.7.zip

  flext doesn't support 64-bit yet, see https://github.com/grrrr/flext/pull/10

* Unpack the Max SDK to e.g. $HOME/Applications

* Get flext from Github: https://github.com/grrrr/flext

* Build flext:
  Note: MACOSX_DEPLOYMENT_TARGET=10.6 doesn't work with XCode compiler

      cd flext
      export CC=gcc-6 CXX=g++-6
      export MACOSX_DEPLOYMENT_TARGET=10.6
      bash build.sh max gcc

  Change MAXSDKPATH in buildsys/config-mac-max-gcc.txt:

      MAXSDKPATH=$(HOME)/Applications/MaxSDK-5.1.7/c74support
      #MAXSDKPATH=$(HOME)/Applications/MaxSDK-6.1.4/c74support
      #MAXSDKPATH=$(HOME)/Applications/max-sdk-7.1.0/source/c74support

  In the same file, change INSTPATH, INITPATH and HELPPATH:

      INSTPATH=$(HOME)/Documents/Max\ 7/Library
      INITPATH=$(HOME)/Documents/Max\ 7/Library
      HELPPATH=$(HOME)/Documents/Max\ 7/Library

  Also change FLEXTPREFIX if you don't want it to be installed to /usr/local.

  Keep ARCH=i386, ...?

  Save the file and run again:

      bash build.sh max gcc

  If necessary, edit config.txt, then run yet again:

      bash build.sh max gcc

  Finally, install it with

      sudo bash build.sh max gcc install

  If $FLEXTPREFIX is writable for your user account (like it typically is when
  you use Homebrew), you don't need "sudo".

* Compile and Install SSR dependencies (using GCC):

      export HOMEBREW_CC=gcc-6 HOMEBREW_CXX=g++-6
      export CFLAGS="-arch i386"
      brew install -v --build-from-source libxml2
      brew install -v --build-from-source libsndfile
      brew install -v --build-from-source fftw

  We have to build it from source to use the right compiler and standard
  library, and we have to use "-arch i386" because the Max SDKs are only
available for that architecture.



making the SSR external:


export FLEXTPATH=$HOME/git/flext


    LDFLAGS:  -L/usr/local/opt/libxml2/lib
    CPPFLAGS: -I/usr/local/opt/libxml2/include
    export PKG_CONFIG_PATH=/usr/local/opt/libxml2/lib/pkgconfig
    export CC=gcc-6 CXX=g++-6
    export MACOSX_DEPLOYMENT_TARGET=10.6








SDK 7.1.0:
could actually be compiled
after uncommenting the following line in flext/source/flbuf.cpp

    ::object_method((t_object *)p,(t_symbol *)sym_dirty);

but then, when connecting the external and switching on DSP processing:
msp object need to be updated for msp64

SDK 7.0.1 with XCode and command line tools:
flex compilation works, many warnings but no errors
make max CXXFLAGS=-stdlib=libc++
but: clang: error: invalid deployment target for -stdlib=libc++ (requires OS X 10.7 or later)

