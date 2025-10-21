set(NAME "gamelib")

add_library(
    ${NAME} SHARED
    ../engine/game_state.c
    asset/audio.c
    asset/font.c
    asset/sprite.c
    coroutine.c
    input.c
    movement.c
    player.c
    enemy.c
    explosion.c
    collision.c
    floating_score.c
    hit_particle.c
    star_particle.c
    sprite.c
    background_scroll.c
    scene.c
    scene/intro.c
    scene/gameplay.c
    game.c
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

