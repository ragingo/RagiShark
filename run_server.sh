#!/bin/bash

set -eu

LISTEN_ADDR=127.0.0.1
LISTEN_PORT=8080

APP=./Server/CSharp/RagiSharkServerCS
APP_ARGS=(
    "-l" "$LISTEN_ADDR"
    "-p" "$LISTEN_PORT"
)

dotnet run --project $APP -- "${APP_ARGS[@]}"
