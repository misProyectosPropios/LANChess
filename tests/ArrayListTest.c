#include <stdio.h>
#include <stdlib.h>
#include "test_lib.h"
#include "../src/DataStructures/ArrayList.h"

static ArrayList* list;

void setUp(void) {
    list = createArray();
}

void tearDown(void) {
    if (list != NULL) {
        freeList(list);
        free(list); // freeList only cleans up internal data
    }
}

int test_createArray_initializes_correctly(void) {
    ASSERT_TRUE(list != NULL); // Check if list is not NULL
    // Fixed: Explicitly cast pointer to boolean
    ASSERT_TRUE((list != NULL));
    ASSERT_INT_EQ(0, size(list));
    ASSERT_INT_EQ(1, isEmpty(list));
    return TEST_SUCCESS;
}

int test_add_increases_size_and_stores_value(void) {
    add(list, 42);
    add(list, 84);
    ASSERT_INT_EQ(2, size(list));
    ASSERT_INT_EQ(42, get(list, 0));
    ASSERT_INT_EQ(84, get(list, 1));
    return TEST_SUCCESS;
}

int test_add_triggers_resize(void) {
    for (int i = 0; i < 10; i++) {
        add(list, i * 10);
    }
    ASSERT_INT_EQ(10, size(list));
    ASSERT_INT_EQ(90, get(list, 9));
    return TEST_SUCCESS;
}

int test_get_out_of_bounds_returns_error_sentinel(void) {
    add(list, 10);
    // Expecting -1 based on the logic fix in ArrayList.c
    ASSERT_INT_EQ(-1, get(list, 5));
    ASSERT_INT_EQ(-1, get(list, -1));
    return TEST_SUCCESS;
}

int test_contains_finds_elements(void) {
    add(list, 100);
    ASSERT_TRUE(contains(list, 100));
    ASSERT_FALSE(contains(list, 200));
    return TEST_SUCCESS;
}

int test_removeItem_shifts_elements_correctly(void) {
    add(list, 10);
    add(list, 20);
    add(list, 30);
    removeItem(list, 1); // Removes 20
    ASSERT_INT_EQ(2, size(list));
    ASSERT_INT_EQ(10, get(list, 0));
    ASSERT_INT_EQ(30, get(list, 1));
    return TEST_SUCCESS;
}

int test_set_updates_value_at_index(void) {
    add(list, 10);
    set(list, 0, 99);
    ASSERT_INT_EQ(99, get(list, 0));
    return TEST_SUCCESS;
}

int test_clear_resets_list(void) {
    add(list, 1);
    add(list, 2);
    clear(list);
    ASSERT_INT_EQ(0, size(list));
    ASSERT_TRUE(isEmpty(list));
    return TEST_SUCCESS;
}

int test_indexOf_returns_correct_index(void) {
    add(list, 10);
    add(list, 20);
    add(list, 30);
    ASSERT_INT_EQ(1, indexOf(list, 20));
    ASSERT_INT_EQ(-1, indexOf(list, 99));
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
