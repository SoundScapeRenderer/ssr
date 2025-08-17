#!/usr/bin/env bash

set -euo pipefail

git clone https://github.com/zaphoyd/websocketpp.git
cd websocketpp
# https://github.com/zaphoyd/websocketpp/pull/1164
git fetch origin pull/1164/head
git checkout FETCH_HEAD
mkdir build
cd build
cmake ..
make
sudo make install
cd ..
cd ..
