#!/usr/bin/env bash

set -euo pipefail

git clone https://github.com/hoene/libmysofa.git
cd libmysofa
cd build
cmake .. -D BUILD_TESTS=OFF -D BUILD_STATIC_LIBS=OFF
make
sudo make install
cd ..
cd ..
