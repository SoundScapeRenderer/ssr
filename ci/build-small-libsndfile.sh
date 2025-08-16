#!/usr/bin/env bash

set -euo pipefail

git clone https://github.com/libsndfile/libsndfile.git
cd libsndfile
git checkout 1.2.2
# https://github.com/libsndfile/libsndfile/issues/1049
git config user.name "Dummy Name for cherry-pick"
git config --global user.email "cherry-pick@example.com"
git cherry-pick 2251737b3b175925684ec0d37029ff4cb521d302
autoreconf -vif
./configure \
  --disable-sqlite \
  --disable-alsa \
  --disable-external-libs \
  --disable-octave \
  --disable-full-suite \
  --disable-test-coverage \
  --disable-ossfuzzers \
  --disable-dependency-tracking \
  --
make
make install
cd ..
