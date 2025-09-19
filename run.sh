#!/bin/bash

set -e

BINARY="build/example_mac/example_mac"

if [ ! -x "$BINARY" ]; then
    echo "Build the project first by running ./build.sh"
    exit 1
fi

"$BINARY" "$@"
