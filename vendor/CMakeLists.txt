include(FetchContent)

FetchContent_Declare(
  cute
  GIT_REPOSITORY https://github.com/RandyGaul/cute_framework
  GIT_TAG fb85bc0e9f3b6504a0bfa107bfc35be63a8daf8b
)

# Settings

set(CF_FRAMEWORK_BUILD_SAMPLES OFF CACHE BOOL "")
set(CF_FRAMEWORK_BUILD_TESTS OFF CACHE BOOL "")

if (${RELOADABLE})
    set(CF_FRAMEWORK_STATIC OFF CACHE BOOL "")
else ()
    set(CF_FRAMEWORK_STATIC ON CACHE BOOL "")
endif ()

FetchContent_MakeAvailable(cute)

