set(NAME "gamelib")

if (${RELOADABLE})
    set(LIBRARY_TYPE SHARED)
else()
    set(LIBRARY_TYPE STATIC)
endif()

add_library(
    ${NAME} ${LIBRARY_TYPE}
    asset/audio.c
    asset/font.c
    asset/sprite.c
    background_scroll.c
    collision.c
    coroutine.c
    enemy.c
    explosion.c
    explosion_particle.c
    floating_score.c
    formation.c
    game.c
    hit_particle.c
    input.c
    player.c
    player_bullet.c
    star_particle.c
)
target_link_libraries(${NAME}
  PRIVATE project_warnings
  PUBLIC cute engine)
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

