#!/bin/bash

set -eu

APP=./Server/CSharp/RagiSharkServerCS

dotnet build -c Debug --force --no-incremental --no-restore --nologo -v normal $APP
