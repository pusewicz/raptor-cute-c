cmake_minimum_required(VERSION 3.22)

project(Raptor
  VERSION 0.1
  LANGUAGES C CXX
)

set(PROJECT_COMPANY_NAME "layer|twenty|two|games")
set(PROJECT_COMPANY_NAMESPACE "com.layer22")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(cmake/StandardProjectSettings.cmake)
include(GNUInstallDirs)
# Link this "library" to use the warnings specified in CompilerWarnings.cmake.
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_subdirectory(vendor)
add_subdirectory(src)
