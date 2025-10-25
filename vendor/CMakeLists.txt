include(FetchContent)

FetchContent_Declare(
  cute
  GIT_REPOSITORY https://github.com/RandyGaul/cute_framework
  GIT_TAG 34ba97d409d312629a5b138213f9709762329c30
)

# Settings

set(CF_FRAMEWORK_BUILD_SAMPLES OFF)
set(CF_FRAMEWORK_BUILD_TESTS OFF)

# Build Cute Framework statically in Release mode
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CF_FRAMEWORK_STATIC ON)
else()
    set(CF_FRAMEWORK_STATIC OFF)
endif()

# Disable installation of dependencies
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(PHYSFS_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cute)

