#!/bin/bash

pushd build
rm -rf *

cmake -G Ninja ../
ninja

popd
