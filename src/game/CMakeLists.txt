set(NAME "gamelib")

add_library(
    ${NAME} SHARED
    ../engine/game_state.c
    asset/font.c
    asset/sprite.c
    coroutine.c
    game.c
    ecs.c
    event.c
    factory.c
)
target_link_libraries(${NAME}
  PRIVATE project_warnings
  PUBLIC cute)
target_compile_features(${NAME} PRIVATE c_std_23)
target_compile_definitions(
    ${NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    GAME_LIBRARY_NAME="$<TARGET_FILE_NAME:${NAME}>"
)
target_include_directories(${NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

