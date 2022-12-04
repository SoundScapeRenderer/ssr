#!/usr/bin/env bash

set -euo pipefail

git clone https://github.com/libsndfile/libsndfile.git
cd libsndfile
git checkout 1.1.0
autoreconf -vif
./configure \
  --disable-sqlite \
  --disable-alsa \
  --disable-external-libs \
  --disable-octave \
  --disable-full-suite \
  --disable-test-coverage \
  --disable-ossfuzzers \
  --
make
make install
cd ..
