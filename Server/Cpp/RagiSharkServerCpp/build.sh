#!/bin/bash -e

pushd build
rm -rf ./*

if [[ "$OS" == "Windows_NT" ]]; then
    # Windows環境ならMSVCを明示的に指定
    cmake -G "Visual Studio 17 2022" -A x64 ../
    cmake --build . --config Debug
else
    # それ以外はデフォルト
    cmake -G Ninja ../
    ninja
fi

popd