#ifndef LANCHESS_TEST_LIB_H
#define LANCHESS_TEST_LIB_H

#include <stdio.h>
#include <string.h>

#define TEST_SUCCESS 0
#define TEST_FAILURE 1

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            fprintf(stderr, "FAIL: %s:%d: Pointer is NULL\n", \
                    __FILE__, __LINE__); \
            return TEST_FAILURE; \
        } \
    } while (0)

/* Library assertion macros */
#define ASSERT_INT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "FAIL: %s:%d: Expected %d, but got %d\n", __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_TRUE(expected) \
    do { \
        if (!expected) { \
            fprintf(stderr, "FAIL: %s:%d: Expected: TRUE, but got %d\n", __FILE__, __LINE__, (int)(expected)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_FALSE(expected) \
    do { \
        if (expected) { \
            fprintf(stderr, "FAIL: %s:%d: Expected: FALSE, but got %d\n", __FILE__, __LINE__, (int)(expected)); \
            return TEST_FAILURE; \
        } \
    } while (0)


#define ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            fprintf(stderr, "FAIL: %s:%d: Expected '%s', but got '%s'\n", __FILE__, __LINE__, (expected), (actual)); \
            return TEST_FAILURE; \
        } \
    } while (0)

#define ASSERT_TEST_SUCCESS(func_call) \
    do { \
        if ((func_call) != TEST_SUCCESS) { \
            return TEST_FAILURE; \
        } \
    } while (0)

typedef int (*TestFunction)(void);
typedef void (*SetupFunction)(void);
typedef void (*TearDownFunction)(void);

typedef struct {
    const char *name;
    TestFunction run;
} TestCase;

static int run_tests(const TestCase *tests, int test_count, SetupFunction setUp, TearDownFunction tearDown)
{
    int failures = 0;

    for (int i = 0; i < test_count; i++) {
        if (setUp) {
            setUp();
        }

        int result = tests[i].run();

        if (tearDown) {
            tearDown();
        }

        if (result == TEST_SUCCESS) {
            printf("PASS %s\n", tests[i].name);
        } else {
            printf("FAIL %s\n", tests[i].name);
            failures++;
        }
    }

    printf("%d test(s), %d failure(s)\n", test_count, failures);
    return failures == 0 ? 0 : 1;
}

#define TEST_COUNT(tests) ((int)(sizeof(tests) / sizeof((tests)[0])))

#endif
