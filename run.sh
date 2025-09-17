#!/bin/bash

set -e 

if [ ! -f "build/main" ]; then
	echo "Build the project first by running ./build.sh"
	exit 1
fi

./build/main "$@"
