#!/bin/bash
set -x

ARCH_ABI=${1:-aarch64}

KEPLER_SDK_INSTALL_PATH="${HOME}/kepler"
KEPLER_VERSION="qdk-9.3"
KEPLER_SDK_VERSION="0.2.18865.0"

KEPLER_SDK_PATH=$(find "${KEPLER_SDK_INSTALL_PATH}/${KEPLER_VERSION}" -type d -name "${KEPLER_SDK_VERSION}" -print)

rm -rf ./build/*

cmake -DKEPLER_ARCH_ABI="$ARCH_ABI" \
	-DKEPLER_MIN_API_VERSION="0.2" \
	-DCMAKE_TOOLCHAIN_FILE="$KEPLER_SDK_PATH/kndk/build-utils/cmake/KeplerToolchain.cmake" \
	-DKEPLER_PREFIX_PATH="" \
	-DCMAKE_INSTALL_PREFIX="$KEPLER_SDK_PATH/kndk/toolchains/$ARCH_ABI/usr/" \
	-DCMAKE_BUILD_TYPE="debug" \
	-B build/"$ARCH_ABI"

cmake --build build/"$ARCH_ABI"

mkdir -p ./build

cp build/$ARCH_ABI/ModeManagerService_$ARCH_ABI.vpkg ./build/
