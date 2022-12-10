#!/usr/bin/env bash

set -euo pipefail

git clone https://github.com/hoene/libmysofa.git
cd libmysofa
cd build
cmake ..
make
sudo make install
cd ..
cd ..
