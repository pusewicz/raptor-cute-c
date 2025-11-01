#include "test_framework.h"
#include "../src/platform/platform_cute.h"
#include <cute.h>
#include <stdio.h>

// External test suite declarations
extern void test_platform_suite(void);
extern void test_hot_reload_suite(void);
extern void test_state_preservation_suite(void);

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	printf("========================================\n");
	printf("  Hot Reload Test Suite\n");
	printf("========================================\n");

	// Initialize platform (required for Cute Framework)
	int result = platform_init();
	if (result != 0) {
		printf("ERROR: Failed to initialize platform\n");
		return 1;
	}

	// Run all test suites
	RUN_TEST_SUITE(test_platform);
	RUN_TEST_SUITE(test_hot_reload);
	RUN_TEST_SUITE(test_state_preservation);

	// Print summary
	TEST_SUMMARY();

	// Cleanup
	platform_shutdown();

	return TEST_RETURN_CODE();
}
