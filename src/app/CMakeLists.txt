set(NAME "Raptor")

include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_executable(${NAME}
    main.c
    ../platform/platform_cute.c
)

include(${PROJECT_SOURCE_DIR}/src/app/cmake/AppAssets.cmake)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${NAME}
        PROPERTIES
        MACOSX_BUNDLE TRUE
        WIN32 TRUE
    )

    include(${PROJECT_SOURCE_DIR}/src/app/cmake/Packaging.cmake)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${NAME}")

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)
target_compile_features(${NAME} PRIVATE c_std_23)

# In Release mode, link gamelib statically; in Debug mode, load it dynamically
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_libraries(${NAME}
      PRIVATE project_warnings gamelib
      PUBLIC cute)
else()
    target_link_libraries(${NAME}
      PRIVATE project_warnings
      PUBLIC cute)
endif()

target_compile_definitions(
    ${NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:HOT_RELOAD_ENABLED>
    $<$<CONFIG:Release>:RELEASE>
    $<$<CONFIG:Debug>:GAME_LIBRARY_NAME="$<TARGET_FILE_NAME:gamelib>">
)

include(CheckPIESupported)
check_pie_supported(OUTPUT_VARIABLE output LANGUAGES C)
set_property(TARGET ${NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
if(NOT CMAKE_C_LINK_PIE_SUPPORTED)
  message(WARNING "PIE is not supported at link time:\n${output}"
                  "PIE link options will not be passed to linker.")
endif()
