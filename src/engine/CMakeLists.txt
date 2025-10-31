set(NAME "engine")

add_library(${NAME} STATIC
    game_state.c
)

target_link_libraries(${NAME}
    PRIVATE project_warnings
    PUBLIC cute
)

target_compile_features(${NAME} PRIVATE c_std_23)

target_compile_definitions(${NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)

target_include_directories(${NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
