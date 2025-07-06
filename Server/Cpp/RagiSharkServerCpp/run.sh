#!/bin/bash

./build.sh

if [[ "$OS" == "Windows_NT" ]]; then
    ./build/Debug/RagiSharkServerCpp.exe
else
    ./build/RagiSharkServerCpp
fi
