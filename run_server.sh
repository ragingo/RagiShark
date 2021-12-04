#!/bin/bash -eu

pushd ./Server > /dev/null

if [[ ! -e .env ]]; then
    echo ".env.sample を参考にして .env を作って！"
    exit 1
fi

# shellcheck disable=SC1091
source .env

run_cs() {
    readonly APP=./CSharp/RagiSharkServerCS
    readonly APP_ARGS=(
        "-l" "$LISTEN_ADDR"
        "-p" "$LISTEN_PORT"
    )
    dotnet run --project $APP -- "${APP_ARGS[@]}"
}

case "$IMPL_LANG" in
    CS) run_cs ;;
    CPP) echo "C++" ;;
    RS) echo "Rust" ;;
    SW) echo "Swift" ;;
esac

popd > /dev/null
