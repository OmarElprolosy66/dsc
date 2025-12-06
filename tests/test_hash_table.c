/**
 * Hash Table Tests
 * Tests all edge cases and functionality of the dsc_hash_table API.
 */

#include "test_framework.h"

#define DSC_IMPLEMENTATION
#include "../dsc.h"
#include <string.h>

/* String comparison function for variable-length string keys */
static int str_cmp(const void* key1, size_t len1, const void* key2, size_t len2) {
    /* For strings, we ignore len params and use strcmp since they're null-terminated */
    (void)len1;
    (void)len2;
    return strcmp((const char*)key1, (const char*)key2);
}

/* String hash function - djb2 algorithm */
static uint64_t str_hash(const void* key, size_t len) {
    const char* str = (const char*)key;
    uint64_t hash = 5381;
    (void)len; /* Ignore len for null-terminated strings */
    
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}

/* For string keys, we use a sentinel value for key_size to indicate variable length */
#define STR_KEY_SIZE 0

/* =========================================================
   Initialization Tests
   ========================================================= */

TEST(hash_table_init_basic) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(16, ht.capacity);
    ASSERT_EQ(0, ht.size);
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_init_zero_capacity) {
    /* Should default to capacity of 1 */
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 0, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_EQ(1, ht.capacity);
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_init_null_hashfunc) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, NULL, str_cmp);
    ASSERT_EQ(DSC_EHASHFUNC, dsc_get_error());
}

TEST(hash_table_init_null_cmpfunc) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, NULL);
    ASSERT_EQ(DSC_ECMPFUNC, dsc_get_error());
}

TEST(hash_table_init_large_capacity) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 10000, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_EQ(10000, ht.capacity);
    dsc_hash_table_destroy(&ht, NULL);
}

/* =========================================================
   Insert Tests
   ========================================================= */

TEST(hash_table_insert_basic) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    bool result = dsc_hash_table_insert(&ht, "key1", &value);
    ASSERT_TRUE(result);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(1, ht.size);
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_insert_multiple) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2, v3 = 3;
    ASSERT_TRUE(dsc_hash_table_insert(&ht, "key1", &v1));
    ASSERT_TRUE(dsc_hash_table_insert(&ht, "key2", &v2));
    ASSERT_TRUE(dsc_hash_table_insert(&ht, "key3", &v3));
    ASSERT_EQ(3, ht.size);
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_insert_duplicate_key) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2;
    ASSERT_TRUE(dsc_hash_table_insert(&ht, "key1", &v1));
    ASSERT_FALSE(dsc_hash_table_insert(&ht, "key1", &v2));
    ASSERT_EQ(DSC_EEXISTS, dsc_get_error());
    ASSERT_EQ(1, ht.size);
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_insert_null_table) {
    int value = 42;
    bool result = dsc_hash_table_insert(NULL, "key1", &value);
    ASSERT_FALSE(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(hash_table_insert_null_key) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    bool result = dsc_hash_table_insert(&ht, NULL, &value);
    ASSERT_FALSE(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_insert_null_value) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    bool result = dsc_hash_table_insert(&ht, "key1", NULL);
    ASSERT_FALSE(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_insert_triggers_resize) {
    /* Start with small capacity to trigger resize quickly */
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 4, STR_KEY_SIZE, str_hash, str_cmp);
    size_t initial_capacity = ht.capacity;
    
    int values[10];
    char keys[10][16];
    
    for (int i = 0; i < 10; i++) {
        values[i] = i;
        snprintf(keys[i], sizeof(keys[i]), "key%d", i);
        ASSERT_TRUE(dsc_hash_table_insert(&ht, keys[i], &values[i]));
    }
    
    /* Capacity should have grown */
    ASSERT_TRUE(ht.capacity > initial_capacity);
    ASSERT_EQ(10, ht.size);
    
    /* All values should still be retrievable */
    for (int i = 0; i < 10; i++) {
        int* val = (int*)dsc_hash_table_get(&ht, keys[i]);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(i, *val);
    }
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_insert_empty_key) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    bool result = dsc_hash_table_insert(&ht, "", &value);
    ASSERT_TRUE(result);
    
    int* retrieved = (int*)dsc_hash_table_get(&ht, "");
    ASSERT_NOT_NULL(retrieved);
    ASSERT_EQ(42, *retrieved);
    
    dsc_hash_table_destroy(&ht, NULL);
}

/* =========================================================
   Get Tests
   ========================================================= */

TEST(hash_table_get_existing) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    dsc_hash_table_insert(&ht, "key1", &value);
    
    int* result = (int*)dsc_hash_table_get(&ht, "key1");
    ASSERT_NOT_NULL(result);
    ASSERT_EQ(42, *result);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_get_nonexistent) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_hash_table_get(&ht, "nonexistent");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_ENOTFOUND, dsc_get_error());
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_get_null_table) {
    void* result = dsc_hash_table_get(NULL, "key1");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(hash_table_get_null_key) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_hash_table_get(&ht, NULL);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_get_after_collision) {
    /* Use small table to increase collision chance */
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 2, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2, v3 = 3;
    dsc_hash_table_insert(&ht, "a", &v1);
    dsc_hash_table_insert(&ht, "b", &v2);
    dsc_hash_table_insert(&ht, "c", &v3);
    
    ASSERT_EQ(1, *(int*)dsc_hash_table_get(&ht, "a"));
    ASSERT_EQ(2, *(int*)dsc_hash_table_get(&ht, "b"));
    ASSERT_EQ(3, *(int*)dsc_hash_table_get(&ht, "c"));
    
    dsc_hash_table_destroy(&ht, NULL);
}

/* =========================================================
   Delete Tests
   ========================================================= */

TEST(hash_table_delete_existing) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    dsc_hash_table_insert(&ht, "key1", &value);
    
    void* result = dsc_hash_table_delete(&ht, "key1");
    ASSERT_NOT_NULL(result);
    ASSERT_EQ(42, *(int*)result);
    ASSERT_EQ(0, ht.size);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    
    /* Should not be found anymore */
    ASSERT_NULL(dsc_hash_table_get(&ht, "key1"));
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_delete_nonexistent) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_hash_table_delete(&ht, "nonexistent");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_ENOTFOUND, dsc_get_error());
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_delete_null_table) {
    void* result = dsc_hash_table_delete(NULL, "key1");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(hash_table_delete_null_key) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_hash_table_delete(&ht, NULL);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_hash_table_destroy(&ht, NULL);
}

TEST(hash_table_delete_from_chain) {
    /* Small table to force collisions */
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 2, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2, v3 = 3;
    dsc_hash_table_insert(&ht, "a", &v1);
    dsc_hash_table_insert(&ht, "b", &v2);
    dsc_hash_table_insert(&ht, "c", &v3);
    
    /* Delete middle element */
    dsc_hash_table_delete(&ht, "b");
    
    /* Others should still exist */
    ASSERT_EQ(1, *(int*)dsc_hash_table_get(&ht, "a"));
    ASSERT_NULL(dsc_hash_table_get(&ht, "b"));
    ASSERT_EQ(3, *(int*)dsc_hash_table_get(&ht, "c"));
    
    dsc_hash_table_destroy(&ht, NULL);
}

/* =========================================================
   Destroy Tests
   ========================================================= */

static int cleanup_call_count = 0;
static void test_cleanup(void* obj) {
    cleanup_call_count++;
    (void)obj;
}

TEST(hash_table_destroy_with_cleanup) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2, v3 = 3;
    dsc_hash_table_insert(&ht, "key1", &v1);
    dsc_hash_table_insert(&ht, "key2", &v2);
    dsc_hash_table_insert(&ht, "key3", &v3);
    
    cleanup_call_count = 0;
    dsc_hash_table_destroy(&ht, test_cleanup);
    
    ASSERT_EQ(3, cleanup_call_count);
}

TEST(hash_table_destroy_null_table) {
    dsc_hash_table_destroy(NULL, NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(hash_table_destroy_empty) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    dsc_hash_table_destroy(&ht, NULL);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
}

/* =========================================================
   Stress Tests
   ========================================================= */

TEST(hash_table_stress_many_inserts) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int values[1000];
    char keys[1000][32];
    
    for (int i = 0; i < 1000; i++) {
        values[i] = i;
        snprintf(keys[i], sizeof(keys[i]), "stress_key_%d", i);
        ASSERT_TRUE(dsc_hash_table_insert(&ht, keys[i], &values[i]));
    }
    
    ASSERT_EQ(1000, ht.size);
    
    /* Verify all can be retrieved */
    for (int i = 0; i < 1000; i++) {
        int* val = (int*)dsc_hash_table_get(&ht, keys[i]);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(i, *val);
    }
    
    dsc_hash_table_destroy(&ht, NULL);
}

/* =========================================================
   Error Handling Tests
   ========================================================= */

TEST(hash_table_error_clear) {
    /* Generate an error */
    dsc_hash_table_get(NULL, "key");
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    /* Clear it */
    dsc_clear_error();
    ASSERT_EQ(DSC_EOK, dsc_get_error());
}

TEST(hash_table_strerror) {
    ASSERT_STR_EQ("Success", dsc_strerror(DSC_EOK));
    ASSERT_STR_EQ("Memory allocation failed", dsc_strerror(DSC_ENOMEM));
    ASSERT_STR_EQ("Invalid argument", dsc_strerror(DSC_EINVAL));
    ASSERT_STR_EQ("Key or element not found", dsc_strerror(DSC_ENOTFOUND));
    ASSERT_STR_EQ("Key already exists", dsc_strerror(DSC_EEXISTS));
    ASSERT_STR_EQ("Unknown error", dsc_strerror((dsc_error_t)999));
}

/* =========================================================
   Main
   ========================================================= */

int main(void) {
    TEST_INIT();  /* Enable ANSI colors on Windows */
    printf("=== Hash Table Tests ===\n");
    
    TEST_SECTION("Initialization");
    RUN_TEST(hash_table_init_basic);
    RUN_TEST(hash_table_init_zero_capacity);
    RUN_TEST(hash_table_init_null_hashfunc);
    RUN_TEST(hash_table_init_null_cmpfunc);
    RUN_TEST(hash_table_init_large_capacity);
    
    TEST_SECTION("Insert");
    RUN_TEST(hash_table_insert_basic);
    RUN_TEST(hash_table_insert_multiple);
    RUN_TEST(hash_table_insert_duplicate_key);
    RUN_TEST(hash_table_insert_null_table);
    RUN_TEST(hash_table_insert_null_key);
    RUN_TEST(hash_table_insert_null_value);
    RUN_TEST(hash_table_insert_triggers_resize);
    RUN_TEST(hash_table_insert_empty_key);
    
    TEST_SECTION("Get");
    RUN_TEST(hash_table_get_existing);
    RUN_TEST(hash_table_get_nonexistent);
    RUN_TEST(hash_table_get_null_table);
    RUN_TEST(hash_table_get_null_key);
    RUN_TEST(hash_table_get_after_collision);
    
    TEST_SECTION("Delete");
    RUN_TEST(hash_table_delete_existing);
    RUN_TEST(hash_table_delete_nonexistent);
    RUN_TEST(hash_table_delete_null_table);
    RUN_TEST(hash_table_delete_null_key);
    RUN_TEST(hash_table_delete_from_chain);
    
    TEST_SECTION("Destroy");
    RUN_TEST(hash_table_destroy_with_cleanup);
    RUN_TEST(hash_table_destroy_null_table);
    RUN_TEST(hash_table_destroy_empty);
    
    TEST_SECTION("Stress Tests");
    RUN_TEST(hash_table_stress_many_inserts);
    
    TEST_SECTION("Error Handling");
    RUN_TEST(hash_table_error_clear);
    RUN_TEST(hash_table_strerror);
    
    TEST_SUMMARY();
    return TEST_EXIT_CODE();
}
