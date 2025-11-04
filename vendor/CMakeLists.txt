include(FetchContent)

FetchContent_Declare(
  cute
  GIT_REPOSITORY https://github.com/RandyGaul/cute_framework
  GIT_TAG 5485791078f6f21b8d7d38676e40dbdf2a96eeb4
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

