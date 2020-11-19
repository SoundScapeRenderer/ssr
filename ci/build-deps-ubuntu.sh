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

# libfmt-dev >= 5 is needed, which is in Ubuntu disco
git clone https://github.com/fmtlib/fmt.git
cd fmt
mkdir build
cd build
cmake ..
make
sudo make install
cd ..
cd ..
