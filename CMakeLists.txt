cmake_minimum_required(VERSION 4.0)

project(Raptor
  VERSION 0.1
  LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 23)

set(CF_FRAMEWORK_BUILD_SAMPLES OFF)
set(CF_FRAMEWORK_BUILD_TESTS OFF)
set(CF_FRAMEWORK_STATIC OFF)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui, ccmake
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

find_program(CCACHE ccache)
if(CCACHE)
  message(STATUS "Using ccache")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
  message(STATUS "Ccache not found")
endif()

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
        -Wpedantic
        -Wconversion
        -Wstrict-prototypes
        -Wmissing-prototypes
        -fPIC
        # $<$<CONFIG:DEBUG>:-fsanitize=undefined>
        $<$<CONFIG:DEBUG>:-g>)
endif()

# include(cmake/StaticAnalyzers.cmake)

add_library(
    gamelib SHARED
    src/game/game.c
    src/game/ecs.c
    src/game/factories.c
)
target_link_libraries(gamelib cute)
target_compile_definitions(
    gamelib PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    GAME_LIBRARY_NAME="$<TARGET_FILE_NAME:gamelib>"
)
target_include_directories(gamelib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_executable(
  ${PROJECT_NAME}
  src/main.c
  src/platform/platform_cute.c
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(${PROJECT_NAME} cute)
target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    GAME_LIBRARY_NAME="$<TARGET_FILE_NAME:gamelib>"
)
# if(UNIX)
#     target_link_options(gamelib PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=undefined>)
#     target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=undefined>)
# endif()

set(source "${CMAKE_SOURCE_DIR}/assets")
set(destination "${CMAKE_BINARY_DIR}/assets")
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
    COMMENT "Creating symbolic link for the assets folder in ${destination}"
)

# Copy the generated DLL for SDL to the built binary’s folder
if(WIN32)
    add_custom_command(
        TARGET game POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$" "$"
        VERBATIM
        COMMENT "Copying SDL3 DLL file to build folder"
    )
endif()
