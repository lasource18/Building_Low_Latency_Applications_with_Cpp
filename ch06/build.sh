#!/bin/bash

CMAKE=$(which cmake)
NINJA=$(which ninja)

mkdir -p cmake-build-release
$CMAKE -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=$NINJA -G Ninja -S "." -Bcmake-build-release

$CMAKE --build cmake-build-release --target clean -j$(nproc)
$CMAKE --build cmake-build-release --target all -j$(nproc)

mkdir -p cmake-build-debug
$CMAKE -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=$NINJA -G Ninja -S "." -Bcmake-build-debug
$CMAKE --build cmake-build-debug --target clean -j$(nproc)
$CMAKE --build cmake-build-debug --target all -j$(nproc)