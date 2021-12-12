#!/bin/bash -eu

pushd build > /dev/null
rm -rf ./*

cmake -G Ninja ../
ninja

popd > /dev/null
