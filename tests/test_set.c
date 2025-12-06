/**
 * Set Tests
 * Tests all edge cases and functionality of the dsc_set API.
 */

#include "test_framework.h"

#define DSC_IMPLEMENTATION
#include "../dsc.h"
#include <string.h>

/* String comparison function for variable-length string keys */
static int str_cmp(const void* key1, size_t len1, const void* key2, size_t len2) {
    (void)len1;
    (void)len2;
    return strcmp((const char*)key1, (const char*)key2);
}

/* String hash function - djb2 algorithm */
static uint64_t str_hash(const void* key, size_t len) {
    const char* str = (const char*)key;
    uint64_t hash = 5381;
    (void)len;
    
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}

/* Integer comparison */
static int int_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return *(int*)k1 - *(int*)k2;
}

/* Integer hash (FNV-1a) */
static uint64_t int_hash(const void* key, size_t len) {
    (void)len;
    uint64_t hash = 14695981039346656037ULL;
    int val = *(int*)key;
    unsigned char* bytes = (unsigned char*)&val;
    for (size_t i = 0; i < sizeof(int); i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

#define STR_KEY_SIZE 0

/* =========================================================
   Initialization Tests
   ========================================================= */

TEST(set_init_basic) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_NOT_NULL(set.ht);
    ASSERT_EQ(16, set.ht->capacity);
    ASSERT_EQ(0, set.ht->size);
    dsc_set_destroy(&set);
}

TEST(set_init_zero_capacity) {
    dsc_set set;
    dsc_set_init(&set, 0, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_EQ(1, set.ht->capacity);
    dsc_set_destroy(&set);
}

TEST(set_init_null_set) {
    dsc_set_init(NULL, 16, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(set_init_null_hashfunc) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, NULL, str_cmp);
    ASSERT_EQ(DSC_EHASHFUNC, dsc_get_error());
}

TEST(set_init_null_cmpfunc) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, NULL);
    ASSERT_EQ(DSC_ECMPFUNC, dsc_get_error());
}

TEST(set_init_large_capacity) {
    dsc_set set;
    dsc_set_init(&set, 10000, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_EQ(10000, set.ht->capacity);
    dsc_set_destroy(&set);
}

/* =========================================================
   Add Tests
   ========================================================= */

TEST(set_add_basic) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    bool result = dsc_set_add(&set, "item1");
    ASSERT_TRUE(result);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(1, set.ht->size);
    
    dsc_set_destroy(&set);
}

TEST(set_add_multiple) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    ASSERT_TRUE(dsc_set_add(&set, "apple"));
    ASSERT_TRUE(dsc_set_add(&set, "banana"));
    ASSERT_TRUE(dsc_set_add(&set, "cherry"));
    ASSERT_EQ(3, set.ht->size);
    
    dsc_set_destroy(&set);
}

TEST(set_add_duplicate) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    ASSERT_TRUE(dsc_set_add(&set, "duplicate"));
    ASSERT_FALSE(dsc_set_add(&set, "duplicate"));
    ASSERT_EQ(DSC_EEXISTS, dsc_get_error());
    ASSERT_EQ(1, set.ht->size);
    
    dsc_set_destroy(&set);
}

TEST(set_add_null_set) {
    bool result = dsc_set_add(NULL, "item");
    ASSERT_FALSE(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(set_add_null_item) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    bool result = dsc_set_add(&set, NULL);
    ASSERT_FALSE(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_set_destroy(&set);
}

TEST(set_add_empty_string) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    bool result = dsc_set_add(&set, "");
    ASSERT_TRUE(result);
    
    void* found = dsc_set_get(&set, "");
    ASSERT_NOT_NULL(found);
    
    dsc_set_destroy(&set);
}

TEST(set_add_triggers_resize) {
    dsc_set set;
    dsc_set_init(&set, 4, STR_KEY_SIZE, str_hash, str_cmp);
    size_t initial_capacity = set.ht->capacity;
    
    char keys[10][16];
    for (int i = 0; i < 10; i++) {
        snprintf(keys[i], sizeof(keys[i]), "key%d", i);
        ASSERT_TRUE(dsc_set_add(&set, keys[i]));
    }
    
    ASSERT_TRUE(set.ht->capacity > initial_capacity);
    ASSERT_EQ(10, set.ht->size);
    
    // Verify all items still accessible
    for (int i = 0; i < 10; i++) {
        ASSERT_NOT_NULL(dsc_set_get(&set, keys[i]));
    }
    
    dsc_set_destroy(&set);
}

/* =========================================================
   Get Tests
   ========================================================= */

TEST(set_get_existing) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_add(&set, "exists");
    
    void* result = dsc_set_get(&set, "exists");
    ASSERT_NOT_NULL(result);
    ASSERT_STR_EQ("exists", (char*)result);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    
    dsc_set_destroy(&set);
}

TEST(set_get_nonexistent) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_set_get(&set, "nonexistent");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_ENOTFOUND, dsc_get_error());
    
    dsc_set_destroy(&set);
}

TEST(set_get_null_set) {
    void* result = dsc_set_get(NULL, "item");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(set_get_null_item) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_set_get(&set, NULL);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_set_destroy(&set);
}

TEST(set_get_after_multiple_adds) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_add(&set, "first");
    dsc_set_add(&set, "second");
    dsc_set_add(&set, "third");
    
    ASSERT_NOT_NULL(dsc_set_get(&set, "first"));
    ASSERT_NOT_NULL(dsc_set_get(&set, "second"));
    ASSERT_NOT_NULL(dsc_set_get(&set, "third"));
    ASSERT_NULL(dsc_set_get(&set, "fourth"));
    
    dsc_set_destroy(&set);
}

/* =========================================================
   Remove Tests
   ========================================================= */

TEST(set_remove_existing) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_add(&set, "remove_me");
    ASSERT_EQ(1, set.ht->size);
    
    dsc_set_remove(&set, "remove_me");
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(0, set.ht->size);
    
    ASSERT_NULL(dsc_set_get(&set, "remove_me"));
    
    dsc_set_destroy(&set);
}

TEST(set_remove_nonexistent) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_remove(&set, "nonexistent");
    ASSERT_EQ(DSC_ENOTFOUND, dsc_get_error());
    
    dsc_set_destroy(&set);
}

TEST(set_remove_null_set) {
    dsc_set_remove(NULL, "item");
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(set_remove_null_item) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_remove(&set, NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_set_destroy(&set);
}

TEST(set_remove_from_multiple) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_add(&set, "a");
    dsc_set_add(&set, "b");
    dsc_set_add(&set, "c");
    
    dsc_set_remove(&set, "b");
    
    ASSERT_NOT_NULL(dsc_set_get(&set, "a"));
    ASSERT_NULL(dsc_set_get(&set, "b"));
    ASSERT_NOT_NULL(dsc_set_get(&set, "c"));
    ASSERT_EQ(2, set.ht->size);
    
    dsc_set_destroy(&set);
}

/* =========================================================
   Clear Tests
   ========================================================= */

TEST(set_clear_empty) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_clear(&set);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(0, set.ht->size);
    
    dsc_set_destroy(&set);
}

TEST(set_clear_with_items) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_add(&set, "item1");
    dsc_set_add(&set, "item2");
    dsc_set_add(&set, "item3");
    
    dsc_set_clear(&set);
    ASSERT_EQ(0, set.ht->size);
    
    ASSERT_NULL(dsc_set_get(&set, "item1"));
    ASSERT_NULL(dsc_set_get(&set, "item2"));
    ASSERT_NULL(dsc_set_get(&set, "item3"));
    
    dsc_set_destroy(&set);
}

TEST(set_clear_null_set) {
    dsc_set_clear(NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(set_clear_then_reuse) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_add(&set, "before");
    dsc_set_clear(&set);
    
    dsc_set_add(&set, "after");
    ASSERT_EQ(1, set.ht->size);
    ASSERT_NOT_NULL(dsc_set_get(&set, "after"));
    ASSERT_NULL(dsc_set_get(&set, "before"));
    
    dsc_set_destroy(&set);
}

/* =========================================================
   Destroy Tests
   ========================================================= */

TEST(set_destroy_empty) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    dsc_set_destroy(&set);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
}

TEST(set_destroy_with_items) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    dsc_set_add(&set, "item1");
    dsc_set_add(&set, "item2");
    dsc_set_add(&set, "item3");
    
    dsc_set_destroy(&set);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
}

TEST(set_destroy_null_set) {
    dsc_set_destroy(NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

/* =========================================================
   Integer Key Tests
   ========================================================= */

TEST(set_int_keys_basic) {
    dsc_set set;
    dsc_set_init(&set, 16, sizeof(int), int_hash, int_cmp);
    
    int nums[] = {5, 10, 15, 20};
    for (int i = 0; i < 4; i++) {
        ASSERT_TRUE(dsc_set_add(&set, &nums[i]));
    }
    
    ASSERT_EQ(4, set.ht->size);
    
    for (int i = 0; i < 4; i++) {
        int* found = (int*)dsc_set_get(&set, &nums[i]);
        ASSERT_NOT_NULL(found);
        ASSERT_EQ(nums[i], *found);
    }
    
    dsc_set_destroy(&set);
}

TEST(set_int_keys_duplicates) {
    dsc_set set;
    dsc_set_init(&set, 16, sizeof(int), int_hash, int_cmp);
    
    int nums[] = {5, 10, 5, 15, 10, 20};
    int unique_count = 0;
    
    for (int i = 0; i < 6; i++) {
        if (dsc_set_add(&set, &nums[i])) {
            unique_count++;
        }
    }
    
    ASSERT_EQ(4, unique_count);
    ASSERT_EQ(4, set.ht->size);
    
    dsc_set_destroy(&set);
}

TEST(set_int_keys_remove) {
    dsc_set set;
    dsc_set_init(&set, 16, sizeof(int), int_hash, int_cmp);
    
    int nums[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        dsc_set_add(&set, &nums[i]);
    }
    
    int remove = 3;
    dsc_set_remove(&set, &remove);
    
    ASSERT_EQ(4, set.ht->size);
    ASSERT_NULL(dsc_set_get(&set, &remove));
    
    dsc_set_destroy(&set);
}

/* =========================================================
   Use Case Tests
   ========================================================= */

TEST(set_unique_word_count) {
    dsc_set words;
    dsc_set_init(&words, 32, STR_KEY_SIZE, str_hash, str_cmp);
    
    const char* sentence[] = {"the", "quick", "brown", "fox", "jumps", 
                              "over", "the", "lazy", "dog", "the", "fox"};
    
    for (int i = 0; i < 11; i++) {
        dsc_set_add(&words, sentence[i]);
    }
    
    ASSERT_EQ(8, words.ht->size);  // 8 unique words
    
    dsc_set_destroy(&words);
}

TEST(set_visited_tracking) {
    dsc_set visited;
    dsc_set_init(&visited, 16, sizeof(int), int_hash, int_cmp);
    
    int nodes[] = {1, 2, 3, 2, 4, 1, 5, 3};
    int visit_count = 0;
    
    for (int i = 0; i < 8; i++) {
        if (dsc_set_get(&visited, &nodes[i]) == NULL) {
            dsc_set_add(&visited, &nodes[i]);
            visit_count++;
        }
    }
    
    ASSERT_EQ(5, visit_count);  // Visited 1,2,3,4,5 (no revisits)
    
    dsc_set_destroy(&visited);
}

TEST(set_membership_testing) {
    dsc_set allowed;
    dsc_set_init(&allowed, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    // Add allowed items
    dsc_set_add(&allowed, "admin");
    dsc_set_add(&allowed, "moderator");
    dsc_set_add(&allowed, "editor");
    
    // Test membership
    ASSERT_NOT_NULL(dsc_set_get(&allowed, "admin"));
    ASSERT_NOT_NULL(dsc_set_get(&allowed, "moderator"));
    ASSERT_NULL(dsc_set_get(&allowed, "guest"));
    ASSERT_NULL(dsc_set_get(&allowed, "anonymous"));
    
    dsc_set_destroy(&allowed);
}

/* =========================================================
   Stress Tests
   ========================================================= */

TEST(set_stress_many_adds) {
    dsc_set set;
    dsc_set_init(&set, 16, sizeof(int), int_hash, int_cmp);
    
    int nums[1000];
    for (int i = 0; i < 1000; i++) {
        nums[i] = i;
        ASSERT_TRUE(dsc_set_add(&set, &nums[i]));
    }
    
    ASSERT_EQ(1000, set.ht->size);
    
    // Verify all can be retrieved
    for (int i = 0; i < 1000; i++) {
        int* found = (int*)dsc_set_get(&set, &nums[i]);
        ASSERT_NOT_NULL(found);
        ASSERT_EQ(i, *found);
    }
    
    dsc_set_destroy(&set);
}

TEST(set_stress_add_remove_cycle) {
    dsc_set set;
    dsc_set_init(&set, 32, sizeof(int), int_hash, int_cmp);
    
    for (int cycle = 0; cycle < 10; cycle++) {
        // Add 100 items
        for (int i = 0; i < 100; i++) {
            int val = cycle * 100 + i;
            dsc_set_add(&set, &val);
        }
        
        // Remove half
        for (int i = 0; i < 50; i++) {
            int val = cycle * 100 + i;
            dsc_set_remove(&set, &val);
        }
    }
    
    ASSERT_EQ(500, set.ht->size);  // 10 cycles * 50 remaining each
    
    dsc_set_destroy(&set);
}

TEST(set_stress_duplicates) {
    dsc_set set;
    dsc_set_init(&set, 64, STR_KEY_SIZE, str_hash, str_cmp);
    
    // Try to add same 10 items 100 times each
    char keys[10][16];
    for (int i = 0; i < 10; i++) {
        snprintf(keys[i], sizeof(keys[i]), "key%d", i);
    }
    
    int successful_adds = 0;
    for (int repeat = 0; repeat < 100; repeat++) {
        for (int i = 0; i < 10; i++) {
            if (dsc_set_add(&set, keys[i])) {
                successful_adds++;
            }
        }
    }
    
    ASSERT_EQ(10, successful_adds);  // Only first 10 succeed
    ASSERT_EQ(10, set.ht->size);
    
    dsc_set_destroy(&set);
}

/* =========================================================
   Edge Cases
   ========================================================= */

TEST(set_collision_handling) {
    dsc_set set;
    dsc_set_init(&set, 2, STR_KEY_SIZE, str_hash, str_cmp);  // Small to force collisions
    
    dsc_set_add(&set, "a");
    dsc_set_add(&set, "b");
    dsc_set_add(&set, "c");
    dsc_set_add(&set, "d");
    
    // All should be retrievable despite collisions
    ASSERT_NOT_NULL(dsc_set_get(&set, "a"));
    ASSERT_NOT_NULL(dsc_set_get(&set, "b"));
    ASSERT_NOT_NULL(dsc_set_get(&set, "c"));
    ASSERT_NOT_NULL(dsc_set_get(&set, "d"));
    
    dsc_set_destroy(&set);
}

TEST(set_add_remove_add_same) {
    dsc_set set;
    dsc_set_init(&set, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    ASSERT_TRUE(dsc_set_add(&set, "item"));
    ASSERT_EQ(1, set.ht->size);
    
    dsc_set_remove(&set, "item");
    ASSERT_EQ(0, set.ht->size);
    
    ASSERT_TRUE(dsc_set_add(&set, "item"));
    ASSERT_EQ(1, set.ht->size);
    
    dsc_set_destroy(&set);
}

TEST(set_zero_then_grow) {
    dsc_set set;
    dsc_set_init(&set, 0, sizeof(int), int_hash, int_cmp);
    
    ASSERT_EQ(1, set.ht->capacity);
    
    for (int i = 0; i < 10; i++) {
        dsc_set_add(&set, &i);
    }
    
    ASSERT_TRUE(set.ht->capacity > 1);
    ASSERT_EQ(10, set.ht->size);
    
    dsc_set_destroy(&set);
}

/* =========================================================
   Main
   ========================================================= */

int main(void) {
    TEST_INIT();  /* Enable ANSI colors on Windows */
    TEST_HEADER("Set Tests");
    
    TEST_SECTION("Initialization");
    RUN_TEST(set_init_basic);
    RUN_TEST(set_init_zero_capacity);
    RUN_TEST(set_init_null_set);
    RUN_TEST(set_init_null_hashfunc);
    RUN_TEST(set_init_null_cmpfunc);
    RUN_TEST(set_init_large_capacity);
    
    TEST_SECTION("Add");
    RUN_TEST(set_add_basic);
    RUN_TEST(set_add_multiple);
    RUN_TEST(set_add_duplicate);
    RUN_TEST(set_add_null_set);
    RUN_TEST(set_add_null_item);
    RUN_TEST(set_add_empty_string);
    RUN_TEST(set_add_triggers_resize);
    
    TEST_SECTION("Get");
    RUN_TEST(set_get_existing);
    RUN_TEST(set_get_nonexistent);
    RUN_TEST(set_get_null_set);
    RUN_TEST(set_get_null_item);
    RUN_TEST(set_get_after_multiple_adds);
    
    TEST_SECTION("Remove");
    RUN_TEST(set_remove_existing);
    RUN_TEST(set_remove_nonexistent);
    RUN_TEST(set_remove_null_set);
    RUN_TEST(set_remove_null_item);
    RUN_TEST(set_remove_from_multiple);
    
    TEST_SECTION("Clear");
    RUN_TEST(set_clear_empty);
    RUN_TEST(set_clear_with_items);
    RUN_TEST(set_clear_null_set);
    RUN_TEST(set_clear_then_reuse);
    
    TEST_SECTION("Destroy");
    RUN_TEST(set_destroy_empty);
    RUN_TEST(set_destroy_with_items);
    RUN_TEST(set_destroy_null_set);
    
    TEST_SECTION("Integer Keys");
    RUN_TEST(set_int_keys_basic);
    RUN_TEST(set_int_keys_duplicates);
    RUN_TEST(set_int_keys_remove);
    
    TEST_SECTION("Use Cases");
    RUN_TEST(set_unique_word_count);
    RUN_TEST(set_visited_tracking);
    RUN_TEST(set_membership_testing);
    
    TEST_SECTION("Stress Tests");
    RUN_TEST(set_stress_many_adds);
    RUN_TEST(set_stress_add_remove_cycle);
    RUN_TEST(set_stress_duplicates);
    
    TEST_SECTION("Edge Cases");
    RUN_TEST(set_collision_handling);
    RUN_TEST(set_add_remove_add_same);
    RUN_TEST(set_zero_then_grow);
    
    TEST_SUMMARY();
    return TEST_EXIT_CODE();
}
