#!/bin/bash

set -eux

PROJ_DIR=./Server/CSharp/RagiSharkServerCS

pushd $PROJ_DIR

dotnet clean
dotnet restore
dotnet build -c Debug --force --no-incremental --no-restore --nologo -v normal

popd
