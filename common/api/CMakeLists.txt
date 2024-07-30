
cmake_minimum_required(VERSION 3.22)
# project(projectionlib-interface VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# Generates a CMake-level library target that encapsulates the code generation for IDLs
kepler_add_idl_import_library(
  idl_pkg
  PACKAGES
  com.stellantis.projectionIface.carplay
  com.stellantis.projectionIface.androidAuto
)

# Installs the IDL files for use by other components
kepler_install_idl(idl_pkg
  PACKAGES
  com.stellantis.projectionIface.carplay
  com.stellantis.projectionIface.androidAuto
)
