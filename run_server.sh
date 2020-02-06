#!/bin/bash

set -eu

LISTEN_ADDR=127.0.0.1
LISTEN_PORT=8080

# mac pc 1
# CAPTURE_INTERFACE=1

# win10 pc 1
CAPTURE_INTERFACE=6

CAPTURE_FILTER=" ""port 80 or port 443"" "

APP=./Server/CSharp/RagiSharkServerCS
APP_ARGS=(
    "-l" "$LISTEN_ADDR"
    "-p" "$LISTEN_PORT"
    "-i" "$CAPTURE_INTERFACE"
    "-f" "$CAPTURE_FILTER"
)

dotnet run --project $APP -- "${APP_ARGS[@]}"
