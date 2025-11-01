set(TEST_NAME "hot_reload_tests")

# Test executable
add_executable(
  ${TEST_NAME}
  test_main.c
  test_platform.c
  test_hot_reload.c
  test_state_preservation.c
  # Include platform source to test it
  ${PROJECT_SOURCE_DIR}/src/platform/platform_cute.c
)

# Include directories
target_include_directories(${TEST_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compile features
target_compile_features(${TEST_NAME} PRIVATE c_std_23)

# Link libraries
target_link_libraries(${TEST_NAME}
  PRIVATE project_warnings
  PUBLIC cute engine
)

# Compile definitions
target_compile_definitions(
  ${TEST_NAME} PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
  $<$<CONFIG:Release>:RELEASE>
  GAME_LIBRARY_NAME="$<TARGET_FILE_NAME:gamelib>"
  ENGINE_ENABLE_HOT_RELOAD=$<IF:$<BOOL:${RELOADABLE}>,1,0>
)

# Enable Position Independent Code
set_property(TARGET ${TEST_NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)

# Add test to CTest
enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

# Set test properties
set_tests_properties(${TEST_NAME} PROPERTIES
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy assets for testing (if needed)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(source "${CMAKE_SOURCE_DIR}/assets")
  set(destination "${CMAKE_BINARY_DIR}/assets")
  add_custom_command(
    TARGET ${TEST_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
    COMMENT "Creating symbolic link for the assets folder in ${destination}"
  )
endif()
