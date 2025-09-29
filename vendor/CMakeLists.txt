include(FetchContent)

FetchContent_Declare(
  cute
  GIT_REPOSITORY https://github.com/RandyGaul/cute_framework
  GIT_TAG 72ee80556db038a7d17786cc8a069fca1cfb7434
)

# Settings

set(CF_FRAMEWORK_BUILD_SAMPLES OFF)
set(CF_FRAMEWORK_BUILD_TESTS OFF)
set(CF_FRAMEWORK_STATIC OFF)

FetchContent_MakeAvailable(cute)

