cmake_minimum_required(VERSION 4.0)

project(Raptor
  VERSION 0.1
  LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CF_FRAMEWORK_BUILD_SAMPLES OFF)
set(CF_FRAMEWORK_BUILD_TESTS OFF)
set(CF_FRAMEWORK_STATIC OFF)

include(FetchContent)
FetchContent_Declare(
  cute
  GIT_REPOSITORY https://github.com/RandyGaul/cute_framework
  GIT_TAG master
  GIT_SHALLOW
)
FetchContent_MakeAvailable(cute)

if (MSVC)
    add_compile_options(
        /W4
        /D_CRT_SECURE_NO_WARNINGS
        /Zc:preprocessor
        $<$<CONFIG:DEBUG>:/RTCcsu>
        $<$<CONFIG:DEBUG>:/Zi>)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wstrict-prototypes
        -fPIC
        $<$<CONFIG:DEBUG>:-g>)
endif()

add_library(
    gamelib SHARED
    src/engine/arena.c
    src/game/game.c
)

add_executable(
  ${PROJECT_NAME}
  src/main.c
  src/platform/platform_cute.c
)

target_include_directories(gamelib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_link_libraries(gamelib cute)
target_compile_definitions(
    gamelib PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_link_libraries(${PROJECT_NAME} cute)
target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)

set(source "${CMAKE_SOURCE_DIR}/assets")
set(destination "${CMAKE_BINARY_DIR}/assets")
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
    COMMENT "Creating symbolic link for the assets folder in ${destination}"
)
