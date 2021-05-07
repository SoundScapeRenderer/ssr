#!/usr/bin/env bash

set -euo pipefail

git clone https://gitlab.gnome.org/GNOME/libxml2.git
cd libxml2
git checkout v2.9.10
./autogen.sh \
  --without-c14n \
  --without-catalog \
  --without-debug \
  --without-docbook \
  --without-fexceptions \
  --without-ftp \
  --without-history \
  --without-html \
  --without-http \
  --without-iconv \
  --without-icu \
  --without-iso8859x \
  --without-legacy \
  --with-minimum \
  --with-output \
  --without-pattern \
  --without-push \
  --without-python \
  --without-reader \
  --without-readline \
  --without-regexps \
  --with-sax1 \
  --with-schemas \
  --without-schematron \
  --without-threads \
  --without-tree \
  --without-valid \
  --without-writer \
  --without-xinclude \
  --with-xpath \
  --without-xptr \
  --without-modules \
  --without-zlib \
  --without-lzma \
  --
make
make install
cd ..
