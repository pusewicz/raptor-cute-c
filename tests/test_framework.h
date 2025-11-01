#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Test framework macros and utilities
typedef struct TestStats {
	int total;
	int passed;
	int failed;
} TestStats;

static TestStats g_test_stats = {0, 0, 0};

#define TEST_SUITE(name) \
	static void name##_suite(void); \
	void name##_suite(void)

#define TEST_CASE(name) \
	static bool name(void); \
	printf("  Running: %s\n", #name); \
	g_test_stats.total++; \
	if (name()) { \
		g_test_stats.passed++; \
		printf("    ✓ PASS\n"); \
	} else { \
		g_test_stats.failed++; \
		printf("    ✗ FAIL\n"); \
	} \
	static bool name(void)

#define TEST_ASSERT(condition) \
	do { \
		if (!(condition)) { \
			printf("      Assertion failed: %s (line %d)\n", #condition, __LINE__); \
			return false; \
		} \
	} while (0)

#define TEST_ASSERT_EQ(a, b) \
	do { \
		if ((a) != (b)) { \
			printf("      Assertion failed: %s == %s (line %d)\n", #a, #b, __LINE__); \
			printf("      Expected: %lld, Got: %lld\n", (long long)(b), (long long)(a)); \
			return false; \
		} \
	} while (0)

#define TEST_ASSERT_NEQ(a, b) \
	do { \
		if ((a) == (b)) { \
			printf("      Assertion failed: %s != %s (line %d)\n", #a, #b, __LINE__); \
			return false; \
		} \
	} while (0)

#define TEST_ASSERT_NULL(ptr) \
	do { \
		if ((ptr) != NULL) { \
			printf("      Assertion failed: %s is NULL (line %d)\n", #ptr, __LINE__); \
			return false; \
		} \
	} while (0)

#define TEST_ASSERT_NOT_NULL(ptr) \
	do { \
		if ((ptr) == NULL) { \
			printf("      Assertion failed: %s is not NULL (line %d)\n", #ptr, __LINE__); \
			return false; \
		} \
	} while (0)

#define TEST_ASSERT_STR_EQ(a, b) \
	do { \
		if (strcmp((a), (b)) != 0) { \
			printf("      Assertion failed: %s == %s (line %d)\n", #a, #b, __LINE__); \
			printf("      Expected: \"%s\", Got: \"%s\"\n", (b), (a)); \
			return false; \
		} \
	} while (0)

#define RUN_TEST_SUITE(name) \
	do { \
		printf("\n=== Running Test Suite: %s ===\n", #name); \
		name##_suite(); \
	} while (0)

#define TEST_SUMMARY() \
	do { \
		printf("\n=== Test Summary ===\n"); \
		printf("Total:  %d\n", g_test_stats.total); \
		printf("Passed: %d\n", g_test_stats.passed); \
		printf("Failed: %d\n", g_test_stats.failed); \
		printf("Success Rate: %.1f%%\n", \
		       g_test_stats.total > 0 ? (100.0 * g_test_stats.passed / g_test_stats.total) : 0.0); \
		if (g_test_stats.failed > 0) { \
			printf("\n❌ TESTS FAILED\n"); \
		} else { \
			printf("\n✅ ALL TESTS PASSED\n"); \
		} \
	} while (0)

#define TEST_RETURN_CODE() (g_test_stats.failed > 0 ? 1 : 0)

#endif // TEST_FRAMEWORK_H
