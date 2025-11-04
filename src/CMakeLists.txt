set(NAME "Raptor")

add_subdirectory(engine)
add_subdirectory(game)

include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_executable(
  ${NAME}
  # WIN32 MACOSX_BUNDLE
  main.c
  platform/platform_cute.c
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set_target_properties(${NAME} PROPERTIES OUTPUT_NAME "${NAME}" SUFFIX ".html")
    target_compile_options(${NAME} PRIVATE -O1 -fno-rtti -fno-exceptions -gsplit-dwarf)
    target_link_options(${NAME} PRIVATE -o ${NAME}.html -sASYNCIFY=1 -O1 -gseparate-dwarf)
    target_link_options(${NAME} PRIVATE --preload-file "${PROJECT_SOURCE_DIR}/assets@/assets")
endif()

target_include_directories(${NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
target_compile_features(${NAME} PRIVATE c_std_23)
target_link_libraries(${NAME}
  PRIVATE project_warnings
  PUBLIC cute)
if(NOT ${RELOADABLE})
    target_link_libraries(${NAME} PRIVATE game)
endif()
target_compile_definitions(
    ${NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    GAME_LIBRARY_NAME="$<TARGET_FILE_NAME:game>"
    ENGINE_ENABLE_HOT_RELOAD=$<IF:$<BOOL:${RELOADABLE}>,1,0>
)

include(CheckPIESupported)
check_pie_supported(OUTPUT_VARIABLE output LANGUAGES C)
set_property(TARGET ${NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
if(NOT CMAKE_C_LINK_PIE_SUPPORTED)
  message(WARNING "PIE is not supported at link time:\n${output}"
                  "PIE link options will not be passed to linker.")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(source "${CMAKE_SOURCE_DIR}/assets")
    set(destination "${CMAKE_BINARY_DIR}/assets")
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        COMMENT "Creating symbolic link for the assets folder in ${destination}"
    )
endif()

# Copy the generated DLL for SDL to the built binary’s folder
if(WIN32)
  add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$" "$"
        VERBATIM
        COMMENT "Copying SDL3 DLL file to build folder"
    )
endif()
