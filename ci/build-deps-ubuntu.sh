#!/usr/bin/env bash

set -euo pipefail

git clone https://github.com/vrpn/vrpn.git
cd vrpn
mkdir build
cd build
cmake -DVRPN_BUILD_JAVA=OFF ..
make
sudo make install
cd ..
cd ..
