include(FetchContent)

FetchContent_Declare(
  cute
  GIT_REPOSITORY https://github.com/RandyGaul/cute_framework
  GIT_TAG ba0d4d253646de811c3c0f5f123733114d18602d
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

FetchContent_MakeAvailable(cute)

