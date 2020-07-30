#!/bin/bash

cd $(dirname "${BASH_SOURCE[0]}")/..
ROOT=$PWD

mkdir -p build-appimage && cd build-appimage || exit 1

cmake \
	-DCUTTER_USE_BUNDLED_RADARE2=ON \
	-DCUTTER_ENABLE_PYTHON=ON \
	-DCUTTER_ENABLE_PYTHON_BINDINGS=ON \
	-DCMAKE_INSTALL_PREFIX="$ROOT/appdir" \
	../src || exit 1

make -j8 || exit 1
make install || exit 1
