#!/usr/bin/env bash

set -euo pipefail



if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  # brew rightfully abandoned qt4
  # https://github.com/cartr/homebrew-qt4
  brew tap cartr/qt4
  brew tap-pin cartr/qt4
  brew install --c++11 asio \
    autoconf \
    doxygen \
    ecasound \
    fftw \
    help2man \
    jack \
    libsndfile \
    qt@4
fi

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  # autotools, automake, make are present in the trusty image
  sudo apt-get install -y \
    g++ \
    libasio-dev \
    libqt4-dev \
    libqt4-opengl-dev \
    libecasoundc-dev \
    ecasound \
    libxml2-dev \
    libfftw3-dev \
    libsndfile1-dev \
    libjack-dev \
    libjack0 \
    jackd1 \
    pkg-config \
    libtool
fi

exit 0
