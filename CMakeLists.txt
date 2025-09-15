cmake_minimum_required(VERSION 4.0)

project(Raptor
  VERSION 0.1
  LANGUAGES C CXX
)

set(PROJECT_COMPANY_NAME "layer|twenty|two|games")
set(PROJECT_COMPANY_NAMESPACE "com.layer22")

include(cmake/StandardProjectSettings.cmake)
include(GNUInstallDirs)
# Link this "library" to use the warnings specified in CompilerWarnings.cmake.
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

add_subdirectory(vendor)

add_library(
    gamelib SHARED
    src/game/state.c
    src/game/game.c
    src/game/ecs.c
    src/game/factories.c
)
target_link_libraries(gamelib
  PRIVATE project_warnings
  PUBLIC cute)
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
target_link_libraries(${PROJECT_NAME}
  PRIVATE project_warnings
  PUBLIC cute)
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
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$" "$"
        VERBATIM
        COMMENT "Copying SDL3 DLL file to build folder"
    )
endif()
