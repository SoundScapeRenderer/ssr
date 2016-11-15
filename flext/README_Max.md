Note that the linked libraries (libxml2, libsndfile and flext) and the SSR external have to be compiled with the same compiler and the same standard library.
In the following instructions GCC is used for everything.

* Install homebrew like described on http://brew.sh/

* Install the GNU compiler GCC

      brew install gcc

* Compile and Install SSR dependencies (using GCC):

      export HOMEBREW_CC=gcc-6 HOMEBREW_CXX=g++-6
      brew install --build-from-source libxml2
      brew install --build-from-source libsndfile

* Get the Max SDK from https://cycling74.com/downloads/sdk/
  Newer versions might not work with flext, so you should choose an old one,
  e.g. version 5.x: http://www.cycling74.com/download/MaxSDK-5.1.7.zip

* Unpack the Max SDK to $HOME/Applications (or wherever)

* Get flext from Github: https://github.com/grrrr/flext

* Build flext:

      cd flext
      export CC=gcc-6 CXX=g++-6
      bash build.sh max gcc

  Change MAXSDKPATH in buildsys/config-mac-max-gcc.txt:

      MAXSDKPATH=$(HOME)/Applications/MaxSDK-5.1.7/c74support

  In the same file, change INSTPATH, INITPATH and HELPPATH:

      INSTPATH=$(HOME)/Documents/Max\ 7/Library
      INITPATH=$(HOME)/Documents/Max\ 7/Library
      HELPPATH=$(HOME)/Documents/Max\ 7/Library

  Also change FLEXTPREFIX if you don't want it to be installed to /usr/local.

  Save the file and run again:

      bash build.sh max gcc

  If necessary, edit config.txt, then run yet again:

      bash build.sh max gcc

  Finally, install it with

      sudo bash build.sh max gcc install

  If $FLEXTPREFIX is writable for your user account, you don't need "sudo".




    LDFLAGS:  -L/usr/local/opt/libxml2/lib
    CPPFLAGS: -I/usr/local/opt/libxml2/include
    export CC=gcc-6 CXX=g++-6








