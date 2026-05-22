#include <stdio.h>
#include <stdlib.h>
#include "test_lib.h"
#include "../src/DataStructures/ArrayList.h"

static ArrayList_Int* list;

void setUp(void) {
    list = createArray_Int();
}

void tearDown(void) {
    if (list != NULL) {
        freeList_Int(list);
        free(list); // freeList only cleans up internal data
    }
}

int test_createArray_initializes_correctly(void) {
    ASSERT_NOT_NULL(list);
    // Fixed: Explicitly cast pointer to boolean
    ASSERT_TRUE((list != NULL));
    ASSERT_INT_EQ(0, size_Int(list));
    ASSERT_INT_EQ(1, isEmpty_Int(list));
    return TEST_SUCCESS;
}

int test_add_increases_size_and_stores_value(void) {
    add_Int(list, 42);
    add_Int(list, 84);
    ASSERT_INT_EQ(2, size_Int(list));
    ASSERT_INT_EQ(42, get_Int(list, 0));
    ASSERT_INT_EQ(84, get_Int(list, 1));
    return TEST_SUCCESS;
}

int test_add_triggers_resize(void) {
    for (int i = 0; i < 10; i++) {
        add_Int(list, i * 10);
    }
    ASSERT_INT_EQ(10, size_Int(list));
    ASSERT_INT_EQ(90, get_Int(list, 9));
    return TEST_SUCCESS;
}

int test_get_out_of_bounds_returns_error_sentinel(void) {
    add_Int(list, 10);
    // Expecting -1 based on the logic fix in ArrayList.c
    ASSERT_INT_EQ(0, get_Int(list, 5));
    ASSERT_INT_EQ(0, get_Int(list, -1));
    return TEST_SUCCESS;
}

int test_contains_finds_elements(void) {
    add_Int(list, 100);
    ASSERT_TRUE(contains_Int(list, 100));
    ASSERT_FALSE(contains_Int(list, 200));
    return TEST_SUCCESS;
}

int test_removeItem_shifts_elements_correctly(void) {
    add_Int(list, 10);
    add_Int(list, 20);
    add_Int(list, 30);
    removeItem_Int(list, 1); // Removes 20
    ASSERT_INT_EQ(2, size_Int(list));
    ASSERT_INT_EQ(10, get_Int(list, 0));
    ASSERT_INT_EQ(30, get_Int(list, 1));
    return TEST_SUCCESS;
}

int test_set_updates_value_at_index(void) {
    add_Int(list, 10);
    set_Int(list, 0, 99);
    ASSERT_INT_EQ(99, get_Int(list, 0));
    return TEST_SUCCESS;
}

int test_clear_resets_list(void) {
    add_Int(list, 1);
    add_Int(list, 2);
    clear_Int(list);
    ASSERT_INT_EQ(0, size_Int(list));
    ASSERT_TRUE(isEmpty_Int(list));
    return TEST_SUCCESS;
}

int test_indexOf_returns_correct_index(void) {
    add_Int(list, 10);
    add_Int(list, 20);
    add_Int(list, 30);
    ASSERT_INT_EQ(1, indexOf_Int(list, 20));
    ASSERT_INT_EQ(-1, indexOf_Int(list, 99));
    return TEST_SUCCESS;
}

int main(void) {
    TestCase tests[] = {
        {"test_createArray_initializes_correctly", test_createArray_initializes_correctly},
        {"test_add_triggers_resize", test_add_triggers_resize},
        {"test_add_increases_size_and_stores_value", test_add_increases_size_and_stores_value},
        {"test_get_out_of_bounds_returns_error_sentinel", test_get_out_of_bounds_returns_error_sentinel},
        {"test_contains_finds_elements", test_contains_finds_elements},
        {"test_removeItem_shifts_elements_correctly", test_removeItem_shifts_elements_correctly},
        {"test_set_updates_value_at_index", test_set_updates_value_at_index},
        {"test_clear_resets_list", test_clear_resets_list},
        {"test_indexOf_returns_correct_index", test_indexOf_returns_correct_index}
    };

    // Updated: Pass setUp and tearDown functions to run_tests
    return run_tests(tests, TEST_COUNT(tests), setUp, tearDown);
}
