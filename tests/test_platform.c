#include "test_framework.h"
#include "../src/platform/platform_cute.h"
#include <stdlib.h>

// Test platform memory allocation functions
TEST_CASE(test_platform_allocate_memory) {
	size_t size = 1024;
	void*  ptr  = platform_allocate_memory(size);

	TEST_ASSERT_NOT_NULL(ptr);

	// Write to memory to ensure it's valid
	memset(ptr, 0xAA, size);

	platform_free_memory(ptr);
	return true;
}

// Test platform memory deallocation (shouldn't crash)
TEST_CASE(test_platform_free_memory) {
	void* ptr = platform_allocate_memory(512);
	TEST_ASSERT_NOT_NULL(ptr);

	// Free should not crash
	platform_free_memory(ptr);

	// Double free should be handled gracefully
	platform_free_memory(NULL);

	return true;
}

// Test large memory allocation
TEST_CASE(test_platform_large_allocation) {
	size_t large_size = 64 * 1024 * 1024; // 64MB
	void*  ptr        = platform_allocate_memory(large_size);

	TEST_ASSERT_NOT_NULL(ptr);

	// Write to first and last bytes
	((char*)ptr)[0]              = 1;
	((char*)ptr)[large_size - 1] = 1;

	platform_free_memory(ptr);
	return true;
}

// Test Platform struct creation
TEST_CASE(test_platform_struct_creation) {
	Platform platform = {
	    .allocate_memory = platform_allocate_memory,
	    .free_memory     = platform_free_memory,
	};

	TEST_ASSERT_NOT_NULL(platform.allocate_memory);
	TEST_ASSERT_NOT_NULL(platform.free_memory);

	// Test using the platform functions
	void* ptr = platform.allocate_memory(256);
	TEST_ASSERT_NOT_NULL(ptr);
	platform.free_memory(ptr);

	return true;
}

// Test game library loading
TEST_CASE(test_load_game_library) {
	GameLibrary game_library = platform_load_game_library();

	TEST_ASSERT(game_library.ok);
	TEST_ASSERT_NOT_NULL(game_library.init);
	TEST_ASSERT_NOT_NULL(game_library.update);
	TEST_ASSERT_NOT_NULL(game_library.render);
	TEST_ASSERT_NOT_NULL(game_library.shutdown);
	TEST_ASSERT_NOT_NULL(game_library.state);
	TEST_ASSERT_NOT_NULL(game_library.hot_reload);

	platform_unload_game_library(&game_library);
	return true;
}

// Test game library unloading
TEST_CASE(test_unload_game_library) {
	GameLibrary game_library = platform_load_game_library();
	TEST_ASSERT(game_library.ok);

	platform_unload_game_library(&game_library);

	// After unloading, library handle should be NULL
	TEST_ASSERT_NULL(game_library.library);

	return true;
}

// Test multiple load/unload cycles
TEST_CASE(test_multiple_load_unload_cycles) {
	for (int i = 0; i < 5; i++) {
		GameLibrary game_library = platform_load_game_library();
		TEST_ASSERT(game_library.ok);
		TEST_ASSERT_NOT_NULL(game_library.init);

		platform_unload_game_library(&game_library);
		TEST_ASSERT_NULL(game_library.library);
	}

	return true;
}

TEST_SUITE(test_platform) {
	TEST_CASE(test_platform_allocate_memory);
	TEST_CASE(test_platform_free_memory);
	TEST_CASE(test_platform_large_allocation);
	TEST_CASE(test_platform_struct_creation);
	TEST_CASE(test_load_game_library);
	TEST_CASE(test_unload_game_library);
	TEST_CASE(test_multiple_load_unload_cycles);
}
