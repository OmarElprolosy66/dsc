/**
 * List Tests
 * Tests all edge cases and functionality of the dsc_list API.
 */

#include "test_framework.h"

#define DSC_IMPLEMENTATION
#include "../dsc.h"

/* =========================================================
   Initialization Tests
   ========================================================= */

TEST(list_init_basic) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_NOT_NULL(list.items);
    ASSERT_EQ(sizeof(int), list.item_size);
    ASSERT_EQ(0, list.length);
    ASSERT_EQ(16, list.capacity);
    
    dsc_list_destroy(&list);
}

TEST(list_init_zero_capacity) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 0);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(256, list.capacity);  /* Default capacity */
    
    dsc_list_destroy(&list);
}

TEST(list_init_null_list) {
    dsc_list_init(NULL, sizeof(int), 16);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(list_init_zero_item_size) {
    dsc_list list;
    dsc_list_init(&list, 0, 16);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(list_init_large_item_size) {
    typedef struct { char data[1024]; } large_struct;
    dsc_list list;
    dsc_list_init(&list, sizeof(large_struct), 8);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(sizeof(large_struct), list.item_size);
    
    dsc_list_destroy(&list);
}

/* =========================================================
   Append Tests
   ========================================================= */

TEST(list_append_basic) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    int value = 42;
    dsc_list_append(&list, &value);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(1, list.length);
    
    dsc_list_destroy(&list);
}

TEST(list_append_multiple) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    for (int i = 0; i < 10; i++) {
        dsc_list_append(&list, &i);
    }
    
    ASSERT_EQ(10, list.length);
    
    /* Verify order */
    for (int i = 0; i < 10; i++) {
        int* val = (int*)dsc_list_get(&list, i);
        ASSERT_EQ(i, *val);
    }
    
    dsc_list_destroy(&list);
}

TEST(list_append_triggers_resize) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 4);
    size_t initial_capacity = list.capacity;
    
    for (int i = 0; i < 10; i++) {
        dsc_list_append(&list, &i);
    }
    
    ASSERT_TRUE(list.capacity > initial_capacity);
    ASSERT_EQ(10, list.length);
    
    /* All values should still be correct */
    for (int i = 0; i < 10; i++) {
        int* val = (int*)dsc_list_get(&list, i);
        ASSERT_EQ(i, *val);
    }
    
    dsc_list_destroy(&list);
}

TEST(list_append_null_list) {
    int value = 42;
    dsc_list_append(NULL, &value);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(list_append_null_item) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    dsc_list_append(&list, NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    ASSERT_EQ(0, list.length);
    
    dsc_list_destroy(&list);
}

TEST(list_append_struct) {
    typedef struct { int x; int y; } point;
    dsc_list list;
    dsc_list_init(&list, sizeof(point), 8);
    
    point p1 = {10, 20};
    point p2 = {30, 40};
    
    dsc_list_append(&list, &p1);
    dsc_list_append(&list, &p2);
    
    point* retrieved = (point*)dsc_list_get(&list, 0);
    ASSERT_EQ(10, retrieved->x);
    ASSERT_EQ(20, retrieved->y);
    
    retrieved = (point*)dsc_list_get(&list, 1);
    ASSERT_EQ(30, retrieved->x);
    ASSERT_EQ(40, retrieved->y);
    
    dsc_list_destroy(&list);
}

/* =========================================================
   Get Tests
   ========================================================= */

TEST(list_get_valid_index) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    int values[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        dsc_list_append(&list, &values[i]);
    }
    
    ASSERT_EQ(10, *(int*)dsc_list_get(&list, 0));
    ASSERT_EQ(20, *(int*)dsc_list_get(&list, 1));
    ASSERT_EQ(30, *(int*)dsc_list_get(&list, 2));
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    
    dsc_list_destroy(&list);
}

TEST(list_get_out_of_range) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    int value = 42;
    dsc_list_append(&list, &value);
    
    void* result = dsc_list_get(&list, 1);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_ERANGE, dsc_get_error());
    
    result = dsc_list_get(&list, 100);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_ERANGE, dsc_get_error());
    
    dsc_list_destroy(&list);
}

TEST(list_get_empty_list) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    void* result = dsc_list_get(&list, 0);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_ERANGE, dsc_get_error());
    
    dsc_list_destroy(&list);
}

TEST(list_get_null_list) {
    void* result = dsc_list_get(NULL, 0);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

/* =========================================================
   Pop Tests
   ========================================================= */

TEST(list_pop_basic) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    int values[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        dsc_list_append(&list, &values[i]);
    }
    
    dsc_list_pop(&list);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(2, list.length);
    
    /* 30 should be gone, 10 and 20 remain */
    ASSERT_EQ(10, *(int*)dsc_list_get(&list, 0));
    ASSERT_EQ(20, *(int*)dsc_list_get(&list, 1));
    
    dsc_list_destroy(&list);
}

TEST(list_pop_until_empty) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    int value = 42;
    dsc_list_append(&list, &value);
    
    dsc_list_pop(&list);
    ASSERT_EQ(0, list.length);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    
    dsc_list_destroy(&list);
}

TEST(list_pop_empty_list) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    dsc_list_pop(&list);
    ASSERT_EQ(DSC_EEMPTY, dsc_get_error());
    
    dsc_list_destroy(&list);
}

TEST(list_pop_null_list) {
    dsc_list_pop(NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

/* =========================================================
   Clear Tests
   ========================================================= */

TEST(list_clear_basic) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    for (int i = 0; i < 5; i++) {
        dsc_list_append(&list, &i);
    }
    
    dsc_list_clear(&list);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(0, list.length);
    ASSERT_TRUE(list.capacity > 0);  /* Capacity unchanged */
    
    dsc_list_destroy(&list);
}

TEST(list_clear_empty_list) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    dsc_list_clear(&list);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(0, list.length);
    
    dsc_list_destroy(&list);
}

TEST(list_clear_null_list) {
    dsc_list_clear(NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(list_clear_then_reuse) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    int v1 = 10;
    dsc_list_append(&list, &v1);
    dsc_list_clear(&list);
    
    int v2 = 20;
    dsc_list_append(&list, &v2);
    
    ASSERT_EQ(1, list.length);
    ASSERT_EQ(20, *(int*)dsc_list_get(&list, 0));
    
    dsc_list_destroy(&list);
}

/* =========================================================
   Resize Tests
   ========================================================= */

TEST(list_resize_grow) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 4);
    
    for (int i = 0; i < 3; i++) {
        dsc_list_append(&list, &i);
    }
    
    dsc_list_resize(&list, 10);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(10, list.length);
    ASSERT_TRUE(list.capacity >= 10);
    
    /* Original values should be preserved */
    ASSERT_EQ(0, *(int*)dsc_list_get(&list, 0));
    ASSERT_EQ(1, *(int*)dsc_list_get(&list, 1));
    ASSERT_EQ(2, *(int*)dsc_list_get(&list, 2));
    
    dsc_list_destroy(&list);
}

TEST(list_resize_shrink) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    for (int i = 0; i < 10; i++) {
        dsc_list_append(&list, &i);
    }
    
    dsc_list_resize(&list, 3);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(3, list.length);
    
    dsc_list_destroy(&list);
}

TEST(list_resize_null_list) {
    dsc_list_resize(NULL, 10);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

/* =========================================================
   Map/Foreach Tests
   ========================================================= */

static int map_sum = 0;
static void sum_callback(void* item) {
    map_sum += *(int*)item;
}

TEST(list_foreach_basic) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    for (int i = 1; i <= 5; i++) {
        dsc_list_append(&list, &i);
    }
    
    map_sum = 0;
    dsc_list_foreach(&list, sum_callback);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(15, map_sum);  /* 1+2+3+4+5 */
    
    dsc_list_destroy(&list);
}

TEST(list_foreach_empty) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    map_sum = 0;
    dsc_list_foreach(&list, sum_callback);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(0, map_sum);
    
    dsc_list_destroy(&list);
}

TEST(list_foreach_null_list) {
    dsc_list_foreach(NULL, sum_callback);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(list_foreach_null_callback) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    dsc_list_foreach(&list, NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_list_destroy(&list);
}

static void double_callback(void* item) {
    *(int*)item *= 2;
}

TEST(list_map_modifies_values) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    for (int i = 1; i <= 3; i++) {
        dsc_list_append(&list, &i);
    }
    
    dsc_list_map(&list, double_callback);
    
    ASSERT_EQ(2, *(int*)dsc_list_get(&list, 0));
    ASSERT_EQ(4, *(int*)dsc_list_get(&list, 1));
    ASSERT_EQ(6, *(int*)dsc_list_get(&list, 2));
    
    dsc_list_destroy(&list);
}

/* =========================================================
   Filter Tests
   ========================================================= */

static int is_even(void* item) {
    return (*(int*)item % 2) == 0;
}

TEST(list_filter_basic) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    for (int i = 1; i <= 10; i++) {
        dsc_list_append(&list, &i);
    }
    
    dsc_list result = dsc_list_filter(&list, is_even);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(5, result.length);  /* 2, 4, 6, 8, 10 */
    
    ASSERT_EQ(2, *(int*)dsc_list_get(&result, 0));
    ASSERT_EQ(4, *(int*)dsc_list_get(&result, 1));
    ASSERT_EQ(6, *(int*)dsc_list_get(&result, 2));
    ASSERT_EQ(8, *(int*)dsc_list_get(&result, 3));
    ASSERT_EQ(10, *(int*)dsc_list_get(&result, 4));
    
    dsc_list_destroy(&result);
    dsc_list_destroy(&list);
}

static int always_false(void* item) {
    (void)item;
    return 0;
}

TEST(list_filter_no_matches) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    for (int i = 1; i <= 5; i++) {
        dsc_list_append(&list, &i);
    }
    
    dsc_list result = dsc_list_filter(&list, always_false);
    
    ASSERT_EQ(0, result.length);
    
    dsc_list_destroy(&result);
    dsc_list_destroy(&list);
}

TEST(list_filter_null_list) {
    dsc_list result = dsc_list_filter(NULL, is_even);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    ASSERT_EQ(0, result.length);
}

TEST(list_filter_null_predicate) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    dsc_list result = dsc_list_filter(&list, NULL);
    (void)result;  /* Suppress unused variable warning */
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_list_destroy(&list);
}

/* =========================================================
   Destroy Tests
   ========================================================= */

TEST(list_destroy_basic) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);
    
    int value = 42;
    dsc_list_append(&list, &value);
    
    dsc_list_destroy(&list);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_NULL(list.items);
    ASSERT_EQ(0, list.length);
    ASSERT_EQ(0, list.capacity);
}

TEST(list_destroy_null_list) {
    dsc_list_destroy(NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

/* =========================================================
   Type-Safe Wrapper Tests (DSC_DEFINE_LIST)
   ========================================================= */

DSC_DEFINE_LIST(int, int)

TEST(typed_list_basic) {
    int_list list;
    int_list_init(&list, 16);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    
    int_list_append(&list, 10);
    int_list_append(&list, 20);
    int_list_append(&list, 30);
    
    ASSERT_EQ(10, int_list_get(&list, 0));
    ASSERT_EQ(20, int_list_get(&list, 1));
    ASSERT_EQ(30, int_list_get(&list, 2));
    
    int_list_destroy(&list);
}

TEST(typed_list_filter) {
    int_list list;
    int_list_init(&list, 16);
    
    for (int i = 1; i <= 6; i++) {
        int_list_append(&list, i);
    }
    
    int_list evens = int_list_filter(&list, is_even);
    
    ASSERT_EQ(3, evens.impl.length);
    ASSERT_EQ(2, int_list_get(&evens, 0));
    ASSERT_EQ(4, int_list_get(&evens, 1));
    ASSERT_EQ(6, int_list_get(&evens, 2));
    
    int_list_destroy(&evens);
    int_list_destroy(&list);
}

/* =========================================================
   Stress Tests
   ========================================================= */

TEST(list_stress_many_appends) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 8);
    
    for (int i = 0; i < 10000; i++) {
        dsc_list_append(&list, &i);
    }
    
    ASSERT_EQ(10000, list.length);
    
    /* Verify some values */
    ASSERT_EQ(0, *(int*)dsc_list_get(&list, 0));
    ASSERT_EQ(5000, *(int*)dsc_list_get(&list, 5000));
    ASSERT_EQ(9999, *(int*)dsc_list_get(&list, 9999));
    
    dsc_list_destroy(&list);
}

/* =========================================================
   Main
   ========================================================= */

int main(void) {
    TEST_INIT();  /* Enable ANSI colors on Windows */
    printf("=== List Tests ===\n");
    
    TEST_SECTION("Initialization");
    RUN_TEST(list_init_basic);
    RUN_TEST(list_init_zero_capacity);
    RUN_TEST(list_init_null_list);
    RUN_TEST(list_init_zero_item_size);
    RUN_TEST(list_init_large_item_size);
    
    TEST_SECTION("Append");
    RUN_TEST(list_append_basic);
    RUN_TEST(list_append_multiple);
    RUN_TEST(list_append_triggers_resize);
    RUN_TEST(list_append_null_list);
    RUN_TEST(list_append_null_item);
    RUN_TEST(list_append_struct);
    
    TEST_SECTION("Get");
    RUN_TEST(list_get_valid_index);
    RUN_TEST(list_get_out_of_range);
    RUN_TEST(list_get_empty_list);
    RUN_TEST(list_get_null_list);
    
    TEST_SECTION("Pop");
    RUN_TEST(list_pop_basic);
    RUN_TEST(list_pop_until_empty);
    RUN_TEST(list_pop_empty_list);
    RUN_TEST(list_pop_null_list);
    
    TEST_SECTION("Clear");
    RUN_TEST(list_clear_basic);
    RUN_TEST(list_clear_empty_list);
    RUN_TEST(list_clear_null_list);
    RUN_TEST(list_clear_then_reuse);
    
    TEST_SECTION("Resize");
    RUN_TEST(list_resize_grow);
    RUN_TEST(list_resize_shrink);
    RUN_TEST(list_resize_null_list);
    
    TEST_SECTION("Map/Foreach");
    RUN_TEST(list_foreach_basic);
    RUN_TEST(list_foreach_empty);
    RUN_TEST(list_foreach_null_list);
    RUN_TEST(list_foreach_null_callback);
    RUN_TEST(list_map_modifies_values);
    
    TEST_SECTION("Filter");
    RUN_TEST(list_filter_basic);
    RUN_TEST(list_filter_no_matches);
    RUN_TEST(list_filter_null_list);
    RUN_TEST(list_filter_null_predicate);
    
    TEST_SECTION("Destroy");
    RUN_TEST(list_destroy_basic);
    RUN_TEST(list_destroy_null_list);
    
    TEST_SECTION("Type-Safe Wrappers");
    RUN_TEST(typed_list_basic);
    RUN_TEST(typed_list_filter);
    
    TEST_SECTION("Stress Tests");
    RUN_TEST(list_stress_many_appends);
    
    TEST_SUMMARY();
    return TEST_EXIT_CODE();
}
