# #######
# Copyright (C) 2021 Stellantis/MobileDrive and/or their affiliated companies. All rights reserved.
#
# This software, including documentation, is protected by copyright and controlled by
# Stellantis/MobileDrive. All rights are reserved. Copying, including reproducing, storing,
# adapting or translating any or all of this material requires the prior written
# consent of Stellantis/MobileDrive jointly. This material also contains confidential information,
# which may not be disclosed to others without the prior and joint written consent of Stellantis/MobileDrive.
#
# ####### About File ########
# root CMakeLists.txt
# #######
cmake_minimum_required(VERSION 3.22)
project(ModeManagerService VERSION 223.01.00003)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_STANDARD 17)

find_package(Apmf REQUIRED)
find_package(Kepler REQUIRED)

# Include the auto generated code from IPCF
include_directories(
  ./build/gen/src/modemanager-service/common/include
  ./build/gen/src/modemanager-service/proxy/include
  ./build/gen/src/modemanager-service/skeleton/include
)

# CMake for generating the IPC proxy for ModeManager service
add_subdirectory(common/ipc)

if(${ENABLE_UNITTEST})
  message(STATUS "Linking SDK LIBS")
  SET(CMAKE_SKIP_BUILD_RPATH FALSE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  SET(CMAKE_INSTALL_RPATH "${CMAKE_SYSROOT}/usr/lib/;${CMAKE_SYSROOT}/lib/")
endif()

# Used to generate the VPKG
include(vpt)

set(LIBRARY modemanager-service)

set(IDL_DIR ${CMAKE_SOURCE_DIR}/common/api)

# find_package(Apmf REQUIRED)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_MODULE_PATH} ${CMAKE_FIND_ROOT_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Generates a CMake-level library target that encapsulates the code generation for IDLs
kepler_add_idl_import_library(
  idl_pkg
  PACKAGES
  com.stellantis.projectionIface.carplay
  com.stellantis.projectionIface.androidAuto
)

# Specify the IDL locations under the include directories
target_include_directories(idl_pkg PUBLIC
  ${IDL_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

# Create the ModeManager service application library target
if(${ENABLE_UNITTEST})
  add_library(${LIBRARY} SHARED
    src/ModeManagerServiceApp.cpp
    src/ModeManagerService.cpp
    src/Utility.cpp
  )
else()
  add_library(${LIBRARY} SHARED
    src/App.cpp
    src/ModeManagerServiceApp.cpp
    src/ModeManagerService.cpp
    src/Utility.cpp
  )
endif()

if(${ENABLE_UNITTEST})
  target_link_libraries(${LIBRARY} PUBLIC

    # Library target corresponding to the IDL auto gen
    idl_pkg

    # cjson
    cjson2

    # LCM library
    lcm_client

    # Logging library
    logging

    # Auto generated ModeManager service skeleton
    modemanager-service-skeleton

    fmt

    "-Wl,--no-undefined"
  )
else()
  target_link_libraries(${LIBRARY} PUBLIC

    # Library target corresponding to the IDL auto gen
    idl_pkg

    # cjson
    cjson2

    # LCM library
    lcm_client

    # Logging library
    logging

    # Auto generated ModeManager service skeleton
    modemanager-service-skeleton

    "-Wl,--no-undefined"
  )
endif()

# Create the VPKG file
vpt_create_package(NAME ${PROJECT_NAME} DEPENDS ${LIBRARY})
