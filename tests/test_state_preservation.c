#include "test_framework.h"
#include "../src/platform/platform_cute.h"
#include "../src/engine/game_state.h"
#include <cute.h>
#include <stdlib.h>
#include <string.h>

// Helper function to create test platform
static Platform create_test_platform(void) {
	return (Platform){
	    .allocate_memory = platform_allocate_memory,
	    .free_memory     = platform_free_memory,
	};
}

// Test that GameState structure is preserved across reload
TEST_CASE(test_game_state_structure_preserved) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game
	library.init(&platform);
	GameState* state = (GameState*)library.state();
	TEST_ASSERT_NOT_NULL(state);

	// Store some values in the state
	int original_score       = state->score;
	int original_lives       = state->lives;
	bool original_game_over  = state->is_game_over;
	void* original_platform  = state->platform;

	// Perform hot reload
	GameLibrary new_library = platform_load_game_library();
	new_library.hot_reload(state);

	// Get state after reload
	GameState* reloaded_state = (GameState*)new_library.state();
	TEST_ASSERT_NOT_NULL(reloaded_state);

	// Verify state pointer is the same
	TEST_ASSERT_EQ(state, reloaded_state);

	// Verify values are preserved
	TEST_ASSERT_EQ(reloaded_state->score, original_score);
	TEST_ASSERT_EQ(reloaded_state->lives, original_lives);
	TEST_ASSERT_EQ(reloaded_state->is_game_over, original_game_over);
	TEST_ASSERT_EQ(reloaded_state->platform, original_platform);

	// Cleanup
	new_library.shutdown();
	platform_unload_game_library(&library);
	platform_unload_game_library(&new_library);

	return true;
}

// Test that modified game variables survive reload
TEST_CASE(test_game_variables_survive_reload) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game
	library.init(&platform);
	GameState* state = (GameState*)library.state();
	TEST_ASSERT_NOT_NULL(state);

	// Modify game state
	state->score       = 12345;
	state->lives       = 7;
	state->is_game_over = false;

	// Perform hot reload
	GameLibrary new_library = platform_load_game_library();
	new_library.hot_reload(state);

	// Get state after reload
	GameState* reloaded_state = (GameState*)new_library.state();
	TEST_ASSERT_NOT_NULL(reloaded_state);

	// Verify modified values are preserved
	TEST_ASSERT_EQ(reloaded_state->score, 12345);
	TEST_ASSERT_EQ(reloaded_state->lives, 7);
	TEST_ASSERT_EQ(reloaded_state->is_game_over, false);

	// Cleanup
	new_library.shutdown();
	platform_unload_game_library(&library);
	platform_unload_game_library(&new_library);

	return true;
}

// Test that canvas size is preserved
TEST_CASE(test_canvas_size_preserved) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game
	library.init(&platform);
	GameState* state = (GameState*)library.state();
	TEST_ASSERT_NOT_NULL(state);

	// Store original canvas size
	float original_width  = state->canvas_size.x;
	float original_height = state->canvas_size.y;
	float original_scale  = state->scale;

	// Perform hot reload
	GameLibrary new_library = platform_load_game_library();
	new_library.hot_reload(state);

	// Get state after reload
	GameState* reloaded_state = (GameState*)new_library.state();
	TEST_ASSERT_NOT_NULL(reloaded_state);

	// Verify canvas settings are preserved
	TEST_ASSERT_EQ(reloaded_state->canvas_size.x, original_width);
	TEST_ASSERT_EQ(reloaded_state->canvas_size.y, original_height);
	TEST_ASSERT_EQ(reloaded_state->scale, original_scale);

	// Cleanup
	new_library.shutdown();
	platform_unload_game_library(&library);
	platform_unload_game_library(&new_library);

	return true;
}

// Test that platform pointer is preserved
TEST_CASE(test_platform_pointer_preserved) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game
	library.init(&platform);
	GameState* state = (GameState*)library.state();
	TEST_ASSERT_NOT_NULL(state);

	// Store original platform pointer
	Platform* original_platform = state->platform;
	TEST_ASSERT_NOT_NULL(original_platform);

	// Perform hot reload
	GameLibrary new_library = platform_load_game_library();
	new_library.hot_reload(state);

	// Get state after reload
	GameState* reloaded_state = (GameState*)new_library.state();
	TEST_ASSERT_NOT_NULL(reloaded_state);

	// Verify platform pointer is preserved
	TEST_ASSERT_EQ(reloaded_state->platform, original_platform);

	// Verify platform functions are still accessible
	TEST_ASSERT_NOT_NULL(reloaded_state->platform->allocate_memory);
	TEST_ASSERT_NOT_NULL(reloaded_state->platform->free_memory);

	// Cleanup
	new_library.shutdown();
	platform_unload_game_library(&library);
	platform_unload_game_library(&new_library);

	return true;
}

// Test memory arena pointers survive reload
TEST_CASE(test_memory_arenas_preserved) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game
	library.init(&platform);
	GameState* state = (GameState*)library.state();
	TEST_ASSERT_NOT_NULL(state);

	// Store arena memory pointers
	void* permanent_mem = state->permanent_arena.mem;
	void* stage_mem     = state->stage_arena.mem;
	void* scratch_mem   = state->scratch_arena.mem;

	size_t permanent_size = state->permanent_arena.capacity;
	size_t stage_size     = state->stage_arena.capacity;
	size_t scratch_size   = state->scratch_arena.capacity;

	// Perform hot reload
	GameLibrary new_library = platform_load_game_library();
	new_library.hot_reload(state);

	// Get state after reload
	GameState* reloaded_state = (GameState*)new_library.state();
	TEST_ASSERT_NOT_NULL(reloaded_state);

	// Verify arena pointers are preserved
	TEST_ASSERT_EQ(reloaded_state->permanent_arena.mem, permanent_mem);
	TEST_ASSERT_EQ(reloaded_state->stage_arena.mem, stage_mem);
	TEST_ASSERT_EQ(reloaded_state->scratch_arena.mem, scratch_mem);

	// Verify arena sizes are preserved
	TEST_ASSERT_EQ(reloaded_state->permanent_arena.capacity, permanent_size);
	TEST_ASSERT_EQ(reloaded_state->stage_arena.capacity, stage_size);
	TEST_ASSERT_EQ(reloaded_state->scratch_arena.capacity, scratch_size);

	// Cleanup
	new_library.shutdown();
	platform_unload_game_library(&library);
	platform_unload_game_library(&new_library);

	return true;
}

// Test that state validity check passes after reload
TEST_CASE(test_state_validity_after_multiple_reloads) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game
	library.init(&platform);
	GameState* state = (GameState*)library.state();
	TEST_ASSERT_NOT_NULL(state);

	// Perform multiple reloads
	for (int i = 0; i < 5; i++) {
		// Modify state
		state->score += 100 * i;
		state->lives = 3 + i;

		// Verify state is valid before reload
		TEST_ASSERT(game_state_is_valid(state));

		// Perform reload
		GameLibrary new_library = platform_load_game_library();
		TEST_ASSERT(new_library.ok);

		new_library.hot_reload(state);

		// Get state after reload
		GameState* reloaded_state = (GameState*)new_library.state();
		TEST_ASSERT_NOT_NULL(reloaded_state);
		TEST_ASSERT_EQ(state, reloaded_state);

		// Verify state is still valid after reload
		TEST_ASSERT(game_state_is_valid(reloaded_state));

		// Verify values persisted
		TEST_ASSERT_EQ(reloaded_state->score, 100 * i * (i + 1) / 2);
		TEST_ASSERT_EQ(reloaded_state->lives, 3 + i);

		platform_unload_game_library(&library);
		library = new_library;
	}

	// Cleanup
	library.shutdown();
	platform_unload_game_library(&library);

	return true;
}

// Test that player data is preserved
TEST_CASE(test_player_data_preserved) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game
	library.init(&platform);
	GameState* state = (GameState*)library.state();
	TEST_ASSERT_NOT_NULL(state);

	// Store original player position
	float original_x = state->player.position.x;
	float original_y = state->player.position.y;

	// Perform hot reload
	GameLibrary new_library = platform_load_game_library();
	new_library.hot_reload(state);

	// Get state after reload
	GameState* reloaded_state = (GameState*)new_library.state();
	TEST_ASSERT_NOT_NULL(reloaded_state);

	// Verify player position is preserved
	TEST_ASSERT_EQ(reloaded_state->player.position.x, original_x);
	TEST_ASSERT_EQ(reloaded_state->player.position.y, original_y);

	// Cleanup
	new_library.shutdown();
	platform_unload_game_library(&library);
	platform_unload_game_library(&new_library);

	return true;
}

TEST_SUITE(test_state_preservation) {
	TEST_CASE(test_game_state_structure_preserved);
	TEST_CASE(test_game_variables_survive_reload);
	TEST_CASE(test_canvas_size_preserved);
	TEST_CASE(test_platform_pointer_preserved);
	TEST_CASE(test_memory_arenas_preserved);
	TEST_CASE(test_state_validity_after_multiple_reloads);
	TEST_CASE(test_player_data_preserved);
}
