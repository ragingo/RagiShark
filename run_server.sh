#!/bin/bash -eux

pushd ./Server

if [[ ! -e .env ]]; then
    echo ".env.sample を参考にして .env を作って！"
    exit 1
fi

# shellcheck disable=SC1091
source .env

readonly APP=./CSharp/RagiSharkServerCS
readonly APP_ARGS=(
    "-l" "$LISTEN_ADDR"
    "-p" "$LISTEN_PORT"
)

dotnet run --project $APP -- "${APP_ARGS[@]}"

popd
