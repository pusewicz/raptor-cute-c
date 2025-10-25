# Assets for all platforms
set(SHARED_STATIC_ASSETS ${PROJECT_SOURCE_DIR}/assets)

# Platform specific static assets
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_sources(${NAME} PUBLIC
    ${SHARED_STATIC_ASSETS}
    ${PROJECT_SOURCE_DIR}/assets/icons/icon.ico
    ${PROJECT_SOURCE_DIR}/src/app/Manifests/app.rc
    ${PROJECT_SOURCE_DIR}/src/app/Manifests/App.manifest)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(MACOSX_STATIC_ASSETS
    ${SHARED_STATIC_ASSETS}
    ${PROJECT_SOURCE_DIR}/assets/icons/icon.icns)
  set_source_files_properties(${MACOSX_STATIC_ASSETS}
    PROPERTIES MACOSX_PACKAGE_LOCATION ${CMAKE_INSTALL_DATADIR})
  target_sources(${NAME} PUBLIC ${MACOSX_STATIC_ASSETS})
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_sources(${NAME} PRIVATE ${SHARED_STATIC_ASSETS})
endif ()

# TODO: This should only be for development
set(source "${CMAKE_SOURCE_DIR}/assets")
set(destination "${CMAKE_BINARY_DIR}/assets")
add_custom_command(
    TARGET ${NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
    COMMENT "Creating symbolic link for the assets folder in ${destination}"
)
