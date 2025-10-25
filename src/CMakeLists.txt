add_subdirectory(app)
add_subdirectory(game)

# macOS bundle configuration
# if(APPLE)
#   set_target_properties(${NAME} PROPERTIES
#     MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
#     MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_COMPANY_NAMESPACE}.${PROJECT_NAME}"
#     MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
#     MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
#     MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} ${PROJECT_VERSION}"
#     MACOSX_BUNDLE_COPYRIGHT "Copyright © ${PROJECT_COMPANY_NAME}"
#   )

#   # Copy assets into the macOS bundle
#   add_custom_command(
#     TARGET ${NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#       "${CMAKE_SOURCE_DIR}/assets"
#       "$<TARGET_BUNDLE_CONTENT_DIR:${NAME}>/Resources/assets"
#     COMMENT "Copying assets to macOS bundle"
#   )
# endif()

# set(source "${CMAKE_SOURCE_DIR}/assets")
# set(destination "${CMAKE_BINARY_DIR}/assets")
# add_custom_command(
#     TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
#     COMMENT "Creating symbolic link for the assets folder in ${destination}"
# )

# Copy the generated DLL for SDL to the built binary’s folder
# if(WIN32)
#   add_custom_command(
#         TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$" "$"
#         VERBATIM
#         COMMENT "Copying SDL3 DLL file to build folder"
#     )
# endif()
