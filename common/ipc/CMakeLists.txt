#
# Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
# PROPRIETARY/CONFIDENTIAL. USE IS SUBJECT TO LICENSE TERMS.
#
cmake_minimum_required(VERSION 3.22)
project(modemanager-common LANGUAGES CXX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${KEPLER_SDK_ROOT_DIR}/kndk/toolchains/${KEPLER_ARCH_ABI}/usr/share/cmake/Modules/")

include(ipcf2)

# Auto generates the IPCF proxy from the ACS file
ipcf_autogen(
  NAME ModeManagerService
  SERVICE_NAME modemanager-service
  OUTPUT_DIRECTORY ../../build/gen/src
)
