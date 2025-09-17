#!/bin/bash

set -e

if [ ! -d "build" ]; then
	cmake -S . -B build -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

cmake --build build
