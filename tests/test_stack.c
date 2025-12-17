/**
 * Stack Tests - Comprehensive test suite for dsc_stack
 * 
 * Copyright (C) 2025 OmarElprolosy66
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 */

#define DSC_IMPLEMENTATION
#include "../dsc.h"
#include "test_framework.h"
#include <string.h>

/* ================================================================
 * Helper Types and Functions
 * ================================================================ */

typedef struct {
    int x;
    int y;
    char name[32];
} Point;

DSC_DEFINE_STACK(int, int)
DSC_DEFINE_STACK(Point, point)

/* ================================================================
 * Basic Initialization Tests
 * ================================================================ */

TEST(test_stack_init_basic) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 16);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(0, stack.list.length);
    ASSERT_EQ(16, stack.list.capacity);
    ASSERT_EQ(sizeof(int), stack.list.item_size);
    ASSERT_NOT_NULL(stack.list.items);
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_init_zero_capacity) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 0);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(0, stack.list.length);
    ASSERT_EQ(256, stack.list.capacity);  /* Default capacity */
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_init_null_stack) {
    dsc_stack_init(NULL, sizeof(int), 16);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(test_stack_init_zero_item_size) {
    dsc_stack stack;
    dsc_stack_init(&stack, 0, 16);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

/* ================================================================
 * Push Operation Tests
 * ================================================================ */

TEST(test_stack_push_basic) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    int val = 42;
    bool success = dsc_stack_push(&stack, &val);
    
    ASSERT_TRUE(success);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(1, dsc_stack_size(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_push_multiple) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    for (int i = 1; i <= 5; i++) {
        ASSERT_TRUE(dsc_stack_push(&stack, &i));
    }
    
    ASSERT_EQ(5, dsc_stack_size(&stack));
    ASSERT_FALSE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_push_null_stack) {
    int val = 42;
    bool success = dsc_stack_push(NULL, &val);
    
    ASSERT_FALSE(success);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(test_stack_push_null_item) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    bool success = dsc_stack_push(&stack, NULL);
    
    ASSERT_FALSE(success);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_push_auto_resize) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 2);
    
    /* Push beyond initial capacity */
    for (int i = 1; i <= 10; i++) {
        ASSERT_TRUE(dsc_stack_push(&stack, &i));
    }
    
    ASSERT_EQ(10, dsc_stack_size(&stack));
    ASSERT(stack.list.capacity > 2);
    
    dsc_stack_destroy(&stack);
}

/* ================================================================
 * Pop Operation Tests
 * ================================================================ */

TEST(test_stack_pop_basic) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    int push_val = 42;
    dsc_stack_push(&stack, &push_val);
    
    int pop_val = 0;
    void* result = dsc_stack_pop(&stack, &pop_val);
    
    ASSERT_NOT_NULL(result);
    ASSERT_EQ(42, pop_val);
    ASSERT_EQ(0, dsc_stack_size(&stack));
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_pop_lifo_order) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 8);
    
    /* Push 1, 2, 3, 4 */
    for (int i = 1; i <= 4; i++) {
        dsc_stack_push(&stack, &i);
    }
    
    /* Pop should return 4, 3, 2, 1 (LIFO) */
    int expected[] = {4, 3, 2, 1};
    for (int i = 0; i < 4; i++) {
        int val = 0;
        dsc_stack_pop(&stack, &val);
        ASSERT_EQ(expected[i], val);
    }
    
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_pop_empty) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    int val = 0;
    void* result = dsc_stack_pop(&stack, &val);
    
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EEMPTY, dsc_get_error());
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_pop_null_stack) {
    int val = 0;
    void* result = dsc_stack_pop(NULL, &val);
    
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(test_stack_pop_all_elements) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 8);
    
    /* Push 10 elements */
    for (int i = 0; i < 10; i++) {
        dsc_stack_push(&stack, &i);
    }
    
    /* Pop all elements */
    for (int i = 9; i >= 0; i--) {
        int val = -1;
        ASSERT_NOT_NULL(dsc_stack_pop(&stack, &val));
        ASSERT_EQ(i, val);
    }
    
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    /* Try popping from empty stack */
    int val = 0;
    ASSERT_NULL(dsc_stack_pop(&stack, &val));
    ASSERT_EQ(DSC_EEMPTY, dsc_get_error());
    
    dsc_stack_destroy(&stack);
}

/* ================================================================
 * Peek Operation Tests
 * ================================================================ */

TEST(test_stack_peek_basic) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    int val = 99;
    dsc_stack_push(&stack, &val);
    
    int* peek_val = (int*)dsc_stack_peek(&stack);
    
    ASSERT_NOT_NULL(peek_val);
    ASSERT_EQ(99, *peek_val);
    ASSERT_EQ(1, dsc_stack_size(&stack));  /* Size unchanged */
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_peek_empty) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    void* result = dsc_stack_peek(&stack);
    
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EEMPTY, dsc_get_error());
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_peek_null_stack) {
    void* result = dsc_stack_peek(NULL);
    
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(test_stack_peek_after_multiple_push) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    for (int i = 1; i <= 5; i++) {
        dsc_stack_push(&stack, &i);
        int* peek_val = (int*)dsc_stack_peek(&stack);
        ASSERT_NOT_NULL(peek_val);
        ASSERT_EQ(i, *peek_val);
    }
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_peek_non_destructive) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    int val = 42;
    dsc_stack_push(&stack, &val);
    
    /* Peek multiple times */
    for (int i = 0; i < 5; i++) {
        int* peek_val = (int*)dsc_stack_peek(&stack);
        ASSERT_NOT_NULL(peek_val);
        ASSERT_EQ(42, *peek_val);
        ASSERT_EQ(1, dsc_stack_size(&stack));
    }
    
    dsc_stack_destroy(&stack);
}

/* ================================================================
 * Size and IsEmpty Tests
 * ================================================================ */

TEST(test_stack_size_empty) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    ASSERT_EQ(0, dsc_stack_size(&stack));
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_size_after_operations) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    ASSERT_EQ(0, dsc_stack_size(&stack));
    
    int val = 1;
    dsc_stack_push(&stack, &val);
    ASSERT_EQ(1, dsc_stack_size(&stack));
    
    dsc_stack_push(&stack, &val);
    ASSERT_EQ(2, dsc_stack_size(&stack));
    
    dsc_stack_pop(&stack, &val);
    ASSERT_EQ(1, dsc_stack_size(&stack));
    
    dsc_stack_pop(&stack, &val);
    ASSERT_EQ(0, dsc_stack_size(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_size_null_stack) {
    size_t size = dsc_stack_size(NULL);
    ASSERT_EQ(0, size);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(test_stack_is_empty_transitions) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    int val = 10;
    dsc_stack_push(&stack, &val);
    ASSERT_FALSE(dsc_stack_is_empty(&stack));
    
    dsc_stack_pop(&stack, &val);
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_is_empty_null_stack) {
    bool result = dsc_stack_is_empty(NULL);
    ASSERT_TRUE(result);  /* Returns true for safety */
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

/* ================================================================
 * Clear and Destroy Tests
 * ================================================================ */

TEST(test_stack_clear_basic) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    for (int i = 1; i <= 5; i++) {
        dsc_stack_push(&stack, &i);
    }
    
    ASSERT_EQ(5, dsc_stack_size(&stack));
    
    dsc_stack_clear(&stack);
    
    ASSERT_EQ(0, dsc_stack_size(&stack));
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    ASSERT_NOT_NULL(stack.list.items);  /* Memory still allocated */
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_clear_empty) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    dsc_stack_clear(&stack);
    
    ASSERT_EQ(0, dsc_stack_size(&stack));
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_clear_null_stack) {
    dsc_stack_clear(NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(test_stack_clear_reuse) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    /* First usage */
    for (int i = 1; i <= 3; i++) {
        dsc_stack_push(&stack, &i);
    }
    dsc_stack_clear(&stack);
    
    /* Reuse after clear */
    for (int i = 10; i <= 12; i++) {
        dsc_stack_push(&stack, &i);
    }
    
    ASSERT_EQ(3, dsc_stack_size(&stack));
    
    int val = 0;
    dsc_stack_pop(&stack, &val);
    ASSERT_EQ(12, val);
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_destroy_basic) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    for (int i = 1; i <= 3; i++) {
        dsc_stack_push(&stack, &i);
    }
    
    dsc_stack_destroy(&stack);
    
    ASSERT_NULL(stack.list.items);
    ASSERT_EQ(0, stack.list.length);
    ASSERT_EQ(0, stack.list.capacity);
}

TEST(test_stack_destroy_empty) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    dsc_stack_destroy(&stack);
    
    ASSERT_NULL(stack.list.items);
    ASSERT_EQ(0, stack.list.capacity);
}

TEST(test_stack_destroy_null_stack) {
    dsc_stack_destroy(NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

/* ================================================================
 * Type-Safe Wrapper Tests (int_stack)
 * ================================================================ */

TEST(test_int_stack_basic) {
    int_stack stack;
    int_stack_init(&stack, 4);
    
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(0, int_stack_size(&stack));
    ASSERT_TRUE(int_stack_is_empty(&stack));
    
    int_stack_destroy(&stack);
}

TEST(test_int_stack_push_pop) {
    int_stack stack;
    int_stack_init(&stack, 4);
    
    ASSERT_TRUE(int_stack_push(&stack, 10));
    ASSERT_TRUE(int_stack_push(&stack, 20));
    ASSERT_TRUE(int_stack_push(&stack, 30));
    
    ASSERT_EQ(3, int_stack_size(&stack));
    
    int val = 0;
    ASSERT_TRUE(int_stack_pop(&stack, &val));
    ASSERT_EQ(30, val);
    
    ASSERT_TRUE(int_stack_pop(&stack, &val));
    ASSERT_EQ(20, val);
    
    ASSERT_TRUE(int_stack_pop(&stack, &val));
    ASSERT_EQ(10, val);
    
    ASSERT_TRUE(int_stack_is_empty(&stack));
    
    int_stack_destroy(&stack);
}

TEST(test_int_stack_peek) {
    int_stack stack;
    int_stack_init(&stack, 4);
    
    int_stack_push(&stack, 99);
    
    int* peek_val = int_stack_peek(&stack);
    ASSERT_NOT_NULL(peek_val);
    ASSERT_EQ(99, *peek_val);
    ASSERT_EQ(1, int_stack_size(&stack));
    
    int_stack_destroy(&stack);
}

TEST(test_int_stack_clear) {
    int_stack stack;
    int_stack_init(&stack, 4);
    
    for (int i = 1; i <= 5; i++) {
        int_stack_push(&stack, i * 10);
    }
    
    int_stack_clear(&stack);
    
    ASSERT_EQ(0, int_stack_size(&stack));
    ASSERT_TRUE(int_stack_is_empty(&stack));
    
    int_stack_destroy(&stack);
}

/* ================================================================
 * Complex Type Tests (Point struct)
 * ================================================================ */

TEST(test_stack_struct_basic) {
    point_stack stack;
    point_stack_init(&stack, 4);
    
    Point p1 = {10, 20, "origin"};
    Point p2 = {30, 40, "target"};
    
    ASSERT_TRUE(point_stack_push(&stack, p1));
    ASSERT_TRUE(point_stack_push(&stack, p2));
    
    ASSERT_EQ(2, point_stack_size(&stack));
    
    Point popped;
    ASSERT_TRUE(point_stack_pop(&stack, &popped));
    ASSERT_EQ(30, popped.x);
    ASSERT_EQ(40, popped.y);
    ASSERT_STR_EQ("target", popped.name);
    
    ASSERT_TRUE(point_stack_pop(&stack, &popped));
    ASSERT_EQ(10, popped.x);
    ASSERT_EQ(20, popped.y);
    ASSERT_STR_EQ("origin", popped.name);
    
    point_stack_destroy(&stack);
}

TEST(test_stack_struct_peek) {
    point_stack stack;
    point_stack_init(&stack, 2);
    
    Point p = {100, 200, "test"};
    point_stack_push(&stack, p);
    
    Point* peek_p = point_stack_peek(&stack);
    ASSERT_NOT_NULL(peek_p);
    ASSERT_EQ(100, peek_p->x);
    ASSERT_EQ(200, peek_p->y);
    ASSERT_STR_EQ("test", peek_p->name);
    
    point_stack_destroy(&stack);
}

/* ================================================================
 * Stress Tests
 * ================================================================ */

TEST(test_stack_stress_large_capacity) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 1000);
    
    /* Push 1000 elements */
    for (int i = 0; i < 1000; i++) {
        ASSERT_TRUE(dsc_stack_push(&stack, &i));
    }
    
    ASSERT_EQ(1000, dsc_stack_size(&stack));
    
    /* Pop all elements and verify LIFO order */
    for (int i = 999; i >= 0; i--) {
        int val = -1;
        ASSERT_NOT_NULL(dsc_stack_pop(&stack, &val));
        ASSERT_EQ(i, val);
    }
    
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_stress_many_operations) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 8);
    
    /* Alternate push and pop operations */
    for (int i = 0; i < 100; i++) {
        dsc_stack_push(&stack, &i);
        
        if (i % 2 == 1) {
            int val = 0;
            dsc_stack_pop(&stack, &val);
        }
    }
    
    ASSERT_EQ(50, dsc_stack_size(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_stress_repeated_clear) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    for (int round = 0; round < 10; round++) {
        /* Fill stack */
        for (int i = 0; i < 20; i++) {
            dsc_stack_push(&stack, &i);
        }
        ASSERT_EQ(20, dsc_stack_size(&stack));
        
        /* Clear it */
        dsc_stack_clear(&stack);
        ASSERT_EQ(0, dsc_stack_size(&stack));
        ASSERT_TRUE(dsc_stack_is_empty(&stack));
    }
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_stress_auto_growth) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 1);  /* Minimal capacity */
    
    /* Push many elements to trigger multiple resizes */
    for (int i = 0; i < 500; i++) {
        ASSERT_TRUE(dsc_stack_push(&stack, &i));
    }
    
    ASSERT_EQ(500, dsc_stack_size(&stack));
    ASSERT(stack.list.capacity >= 500);
    
    /* Verify LIFO order */
    for (int i = 499; i >= 400; i--) {
        int val = -1;
        dsc_stack_pop(&stack, &val);
        ASSERT_EQ(i, val);
    }
    
    dsc_stack_destroy(&stack);
}

/* ================================================================
 * Edge Case Tests
 * ================================================================ */

TEST(test_stack_edge_single_element) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 1);
    
    int val = 42;
    dsc_stack_push(&stack, &val);
    
    int* peek = (int*)dsc_stack_peek(&stack);
    ASSERT_EQ(42, *peek);
    
    int popped = 0;
    dsc_stack_pop(&stack, &popped);
    ASSERT_EQ(42, popped);
    
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_edge_large_struct) {
    typedef struct {
        char data[1024];
        int id;
    } LargeStruct;
    
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(LargeStruct), 2);
    
    LargeStruct large1 = {{0}, 1};
    LargeStruct large2 = {{0}, 2};
    strcpy(large1.data, "test1");
    strcpy(large2.data, "test2");
    
    ASSERT_TRUE(dsc_stack_push(&stack, &large1));
    ASSERT_TRUE(dsc_stack_push(&stack, &large2));
    
    LargeStruct popped;
    dsc_stack_pop(&stack, &popped);
    ASSERT_EQ(2, popped.id);
    ASSERT_STR_EQ("test2", popped.data);
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_edge_push_pop_cycle) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    /* Cycle: push, pop, push, pop... */
    for (int i = 0; i < 50; i++) {
        int val = i;
        dsc_stack_push(&stack, &val);
        
        int popped = 0;
        dsc_stack_pop(&stack, &popped);
        ASSERT_EQ(i, popped);
    }
    
    ASSERT_TRUE(dsc_stack_is_empty(&stack));
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_edge_peek_after_pop) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    int vals[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        dsc_stack_push(&stack, &vals[i]);
    }
    
    int popped = 0;
    dsc_stack_pop(&stack, &popped);  /* Remove 3 */
    
    int* peek = (int*)dsc_stack_peek(&stack);
    ASSERT_NOT_NULL(peek);
    ASSERT_EQ(2, *peek);  /* Should see 2 now */
    
    dsc_stack_destroy(&stack);
}

TEST(test_stack_edge_multiple_stacks) {
    int_stack stack1, stack2, stack3;
    int_stack_init(&stack1, 4);
    int_stack_init(&stack2, 4);
    int_stack_init(&stack3, 4);
    
    int_stack_push(&stack1, 100);
    int_stack_push(&stack2, 200);
    int_stack_push(&stack3, 300);
    
    int val1, val2, val3;
    int_stack_pop(&stack1, &val1);
    int_stack_pop(&stack2, &val2);
    int_stack_pop(&stack3, &val3);
    
    ASSERT_EQ(100, val1);
    ASSERT_EQ(200, val2);
    ASSERT_EQ(300, val3);
    
    int_stack_destroy(&stack1);
    int_stack_destroy(&stack2);
    int_stack_destroy(&stack3);
}

/* ================================================================
 * Main Test Runner
 * ================================================================ */

int main(void) {
    TEST_INIT();
    TEST_HEADER("DSC Stack Tests");
    
    TEST_SECTION("Initialization Tests");
    RUN_TEST(test_stack_init_basic);
    RUN_TEST(test_stack_init_zero_capacity);
    RUN_TEST(test_stack_init_null_stack);
    RUN_TEST(test_stack_init_zero_item_size);
    
    TEST_SECTION("Push Operation Tests");
    RUN_TEST(test_stack_push_basic);
    RUN_TEST(test_stack_push_multiple);
    RUN_TEST(test_stack_push_null_stack);
    RUN_TEST(test_stack_push_null_item);
    RUN_TEST(test_stack_push_auto_resize);
    
    TEST_SECTION("Pop Operation Tests");
    RUN_TEST(test_stack_pop_basic);
    RUN_TEST(test_stack_pop_lifo_order);
    RUN_TEST(test_stack_pop_empty);
    RUN_TEST(test_stack_pop_null_stack);
    RUN_TEST(test_stack_pop_all_elements);
    
    TEST_SECTION("Peek Operation Tests");
    RUN_TEST(test_stack_peek_basic);
    RUN_TEST(test_stack_peek_empty);
    RUN_TEST(test_stack_peek_null_stack);
    RUN_TEST(test_stack_peek_after_multiple_push);
    RUN_TEST(test_stack_peek_non_destructive);
    
    TEST_SECTION("Size and IsEmpty Tests");
    RUN_TEST(test_stack_size_empty);
    RUN_TEST(test_stack_size_after_operations);
    RUN_TEST(test_stack_size_null_stack);
    RUN_TEST(test_stack_is_empty_transitions);
    RUN_TEST(test_stack_is_empty_null_stack);
    
    TEST_SECTION("Clear and Destroy Tests");
    RUN_TEST(test_stack_clear_basic);
    RUN_TEST(test_stack_clear_empty);
    RUN_TEST(test_stack_clear_null_stack);
    RUN_TEST(test_stack_clear_reuse);
    RUN_TEST(test_stack_destroy_basic);
    RUN_TEST(test_stack_destroy_empty);
    RUN_TEST(test_stack_destroy_null_stack);
    
    TEST_SECTION("Type-Safe Wrapper Tests");
    RUN_TEST(test_int_stack_basic);
    RUN_TEST(test_int_stack_push_pop);
    RUN_TEST(test_int_stack_peek);
    RUN_TEST(test_int_stack_clear);
    
    TEST_SECTION("Complex Type Tests");
    RUN_TEST(test_stack_struct_basic);
    RUN_TEST(test_stack_struct_peek);
    
    TEST_SECTION("Stress Tests");
    RUN_TEST(test_stack_stress_large_capacity);
    RUN_TEST(test_stack_stress_many_operations);
    RUN_TEST(test_stack_stress_repeated_clear);
    RUN_TEST(test_stack_stress_auto_growth);
    
    TEST_SECTION("Edge Case Tests");
    RUN_TEST(test_stack_edge_single_element);
    RUN_TEST(test_stack_edge_large_struct);
    RUN_TEST(test_stack_edge_push_pop_cycle);
    RUN_TEST(test_stack_edge_peek_after_pop);
    RUN_TEST(test_stack_edge_multiple_stacks);
    
    TEST_SUMMARY();
    return TEST_EXIT_CODE();
}
