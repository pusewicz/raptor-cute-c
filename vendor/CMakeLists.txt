include(FetchContent)

FetchContent_Declare(
  cute
  GIT_REPOSITORY https://github.com/RandyGaul/cute_framework
  GIT_TAG ba0d4d253646de811c3c0f5f123733114d18602d
)

# Settings

set(CF_FRAMEWORK_BUILD_SAMPLES OFF)
set(CF_FRAMEWORK_BUILD_TESTS OFF)
set(CF_FRAMEWORK_STATIC OFF)

FetchContent_MakeAvailable(cute)

