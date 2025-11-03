#!/bin/bash

CMAKE=$(which cmake)
NINJA=$(which ninja)

mkdir -p cmake-build-release
$CMAKE -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=$NINJA -G Ninja -S "." -Bcmake-build-release

$CMAKE --build cmake-build-release --target clean -j$(nproc)
$CMAKE --build cmake-build-release --target all -j$(nproc)