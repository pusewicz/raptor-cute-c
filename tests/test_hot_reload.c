#include "test_framework.h"
#include "../src/platform/platform_cute.h"
#include "../src/engine/game_state.h"
#include <cute.h>
#include <stdlib.h>
#include <string.h>

// Mock platform for testing
static Platform create_test_platform(void) {
	return (Platform){
	    .allocate_memory = platform_allocate_memory,
	    .free_memory     = platform_free_memory,
	};
}

// Test basic hot reload workflow
TEST_CASE(test_basic_hot_reload) {
	Platform    platform     = create_test_platform();
	GameLibrary game_library = platform_load_game_library();

	TEST_ASSERT(game_library.ok);
	TEST_ASSERT_NOT_NULL(game_library.init);

	// Initialize the game
	game_library.init(&platform);

	// Get the initial game state
	void* state_before = game_library.state();
	TEST_ASSERT_NOT_NULL(state_before);

	// Simulate hot reload
	GameLibrary new_library = platform_load_game_library();
	TEST_ASSERT(new_library.ok);

	// Hot reload with the saved state
	new_library.hot_reload(state_before);

	// Verify state pointer is the same
	void* state_after = new_library.state();
	TEST_ASSERT_EQ(state_before, state_after);

	// Cleanup
	new_library.shutdown();
	platform_unload_game_library(&game_library);
	platform_unload_game_library(&new_library);

	return true;
}

// Test state pointer preservation across reload
TEST_CASE(test_state_pointer_preservation) {
	Platform    platform = create_test_platform();
	GameLibrary library1 = platform_load_game_library();

	TEST_ASSERT(library1.ok);

	// Initialize game
	library1.init(&platform);
	void* original_state = library1.state();
	TEST_ASSERT_NOT_NULL(original_state);

	// Perform reload
	GameLibrary library2 = platform_load_game_library();
	library2.hot_reload(original_state);

	void* reloaded_state = library2.state();

	// The state pointer should be exactly the same
	TEST_ASSERT_EQ(original_state, reloaded_state);

	// Cleanup
	library2.shutdown();
	platform_unload_game_library(&library1);
	platform_unload_game_library(&library2);

	return true;
}

// Test function pointer updates after reload
TEST_CASE(test_function_pointer_updates) {
	GameLibrary library1 = platform_load_game_library();
	TEST_ASSERT(library1.ok);

	void* init_ptr1   = (void*)library1.init;
	void* update_ptr1 = (void*)library1.update;
	void* render_ptr1 = (void*)library1.render;

	GameLibrary library2 = platform_load_game_library();
	TEST_ASSERT(library2.ok);

	void* init_ptr2   = (void*)library2.init;
	void* update_ptr2 = (void*)library2.update;
	void* render_ptr2 = (void*)library2.render;

	// Function pointers should be valid (not NULL)
	TEST_ASSERT_NOT_NULL(init_ptr2);
	TEST_ASSERT_NOT_NULL(update_ptr2);
	TEST_ASSERT_NOT_NULL(render_ptr2);

	// Note: In hot reload scenario, pointers might be the same or different
	// depending on where the library is loaded. We just ensure they're valid.

	platform_unload_game_library(&library1);
	platform_unload_game_library(&library2);

	return true;
}

// Test multiple consecutive reloads
TEST_CASE(test_multiple_consecutive_reloads) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game once
	library.init(&platform);
	void* state = library.state();
	TEST_ASSERT_NOT_NULL(state);

	// Perform multiple reloads
	for (int i = 0; i < 3; i++) {
		GameLibrary new_library = platform_load_game_library();
		TEST_ASSERT(new_library.ok);

		new_library.hot_reload(state);
		void* new_state = new_library.state();

		// State pointer should remain the same
		TEST_ASSERT_EQ(state, new_state);

		platform_unload_game_library(&library);
		library = new_library;
	}

	// Cleanup
	library.shutdown();
	platform_unload_game_library(&library);

	return true;
}

// Test reload without initialization (should handle gracefully)
TEST_CASE(test_reload_without_init) {
	GameLibrary library = platform_load_game_library();
	TEST_ASSERT(library.ok);

	// Try to hot reload with NULL state
	// This simulates an error case - the library should handle it
	library.hot_reload(NULL);

	// Get state after reload with NULL
	void* state = library.state();

	// State might be NULL or a default initialized state
	// The important part is that it doesn't crash

	platform_unload_game_library(&library);

	return true;
}

// Test that all library functions are loaded correctly
TEST_CASE(test_all_library_functions_loaded) {
	GameLibrary library = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Verify all function pointers are loaded
	TEST_ASSERT_NOT_NULL(library.init);
	TEST_ASSERT_NOT_NULL(library.update);
	TEST_ASSERT_NOT_NULL(library.render);
	TEST_ASSERT_NOT_NULL(library.shutdown);
	TEST_ASSERT_NOT_NULL(library.state);
	TEST_ASSERT_NOT_NULL(library.hot_reload);

#if ENGINE_ENABLE_HOT_RELOAD
	TEST_ASSERT_NOT_NULL(library.library); // Shared library handle
	TEST_ASSERT_NOT_NULL(library.path);    // Library path
#endif

	platform_unload_game_library(&library);

	return true;
}

// Test reload preserves game state validity
TEST_CASE(test_reload_preserves_state_validity) {
	Platform    platform = create_test_platform();
	GameLibrary library  = platform_load_game_library();

	TEST_ASSERT(library.ok);

	// Initialize game
	library.init(&platform);

	// Get state and verify it's valid
	GameState* state = (GameState*)library.state();
	TEST_ASSERT_NOT_NULL(state);

	bool is_valid_before = game_state_is_valid(state);
	TEST_ASSERT(is_valid_before);

	// Perform reload
	GameLibrary new_library = platform_load_game_library();
	new_library.hot_reload(state);

	// Get state after reload
	GameState* state_after = (GameState*)new_library.state();
	TEST_ASSERT_NOT_NULL(state_after);

	// State should still be valid after reload
	bool is_valid_after = game_state_is_valid(state_after);
	TEST_ASSERT(is_valid_after);

	// Cleanup
	new_library.shutdown();
	platform_unload_game_library(&library);
	platform_unload_game_library(&new_library);

	return true;
}

TEST_SUITE(test_hot_reload) {
	TEST_CASE(test_basic_hot_reload);
	TEST_CASE(test_state_pointer_preservation);
	TEST_CASE(test_function_pointer_updates);
	TEST_CASE(test_multiple_consecutive_reloads);
	TEST_CASE(test_reload_without_init);
	TEST_CASE(test_all_library_functions_loaded);
	TEST_CASE(test_reload_preserves_state_validity);
}
