#!/usr/bin/env bash

set -euo pipefail

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  brew install \
    asio \
    autoconf \
    ecasound \
    fftw \
    jack \
    liblo \
    libsndfile \
    libxml2 \
    qt
  # using brew, it's not possible to install needed perl dependencies for
  # help2man (Locale::gettext), therefore best disabled for now
#    doxygen \
#    help2man \
fi

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  # autotools, automake, make are present in the trusty image
  sudo apt-get install -y \
    libasio-dev \
    qt5-default \
    libqt5opengl5-dev \
    libecasoundc-dev \
    doxygen \
    ecasound \
    libxml2-dev \
    libfftw3-dev \
    libsndfile1-dev \
    libjack-dev \
    libjack0 \
    help2man \
    jackd1 \
    pkg-config \
    libtool

  # force qt 5
  export QT_SELECT=qt5
fi

exit 0
