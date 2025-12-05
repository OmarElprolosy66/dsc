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
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_NOT_NULL(ht);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(16, ht->capacity);
    ASSERT_EQ(0, ht->size);
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_init_zero_capacity) {
    /* Should default to capacity of 1 */
    dsc_hash_table* ht = dsc_hash_table_init(0, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_NOT_NULL(ht);
    ASSERT_EQ(1, ht->capacity);
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_init_null_hashfunc) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, NULL, str_cmp);
    ASSERT_NULL(ht);
    ASSERT_EQ(DSC_EHASHFUNC, dsc_get_error());
}

TEST(hash_table_init_null_cmpfunc) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, NULL);
    ASSERT_NULL(ht);
    ASSERT_EQ(DSC_ECMPFUNC, dsc_get_error());
}

TEST(hash_table_init_large_capacity) {
    dsc_hash_table* ht = dsc_hash_table_init(10000, STR_KEY_SIZE, str_hash, str_cmp);
    ASSERT_NOT_NULL(ht);
    ASSERT_EQ(10000, ht->capacity);
    dsc_hash_table_destroy(ht, NULL);
}

/* =========================================================
   Insert Tests
   ========================================================= */

TEST(hash_table_insert_basic) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    bool result = dsc_hash_table_insert(ht, "key1", &value);
    ASSERT_TRUE(result);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    ASSERT_EQ(1, ht->size);
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_insert_multiple) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2, v3 = 3;
    ASSERT_TRUE(dsc_hash_table_insert(ht, "key1", &v1));
    ASSERT_TRUE(dsc_hash_table_insert(ht, "key2", &v2));
    ASSERT_TRUE(dsc_hash_table_insert(ht, "key3", &v3));
    ASSERT_EQ(3, ht->size);
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_insert_duplicate_key) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2;
    ASSERT_TRUE(dsc_hash_table_insert(ht, "key1", &v1));
    ASSERT_FALSE(dsc_hash_table_insert(ht, "key1", &v2));
    ASSERT_EQ(DSC_EEXISTS, dsc_get_error());
    ASSERT_EQ(1, ht->size);
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_insert_null_table) {
    int value = 42;
    bool result = dsc_hash_table_insert(NULL, "key1", &value);
    ASSERT_FALSE(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(hash_table_insert_null_key) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    bool result = dsc_hash_table_insert(ht, NULL, &value);
    ASSERT_FALSE(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_insert_null_value) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    bool result = dsc_hash_table_insert(ht, "key1", NULL);
    ASSERT_FALSE(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_insert_triggers_resize) {
    /* Start with small capacity to trigger resize quickly */
    dsc_hash_table* ht = dsc_hash_table_init(4, STR_KEY_SIZE, str_hash, str_cmp);
    size_t initial_capacity = ht->capacity;
    
    int values[10];
    char keys[10][16];
    
    for (int i = 0; i < 10; i++) {
        values[i] = i;
        snprintf(keys[i], sizeof(keys[i]), "key%d", i);
        ASSERT_TRUE(dsc_hash_table_insert(ht, keys[i], &values[i]));
    }
    
    /* Capacity should have grown */
    ASSERT_TRUE(ht->capacity > initial_capacity);
    ASSERT_EQ(10, ht->size);
    
    /* All values should still be retrievable */
    for (int i = 0; i < 10; i++) {
        int* val = (int*)dsc_hash_table_get(ht, keys[i]);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(i, *val);
    }
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_insert_empty_key) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    bool result = dsc_hash_table_insert(ht, "", &value);
    ASSERT_TRUE(result);
    
    int* retrieved = (int*)dsc_hash_table_get(ht, "");
    ASSERT_NOT_NULL(retrieved);
    ASSERT_EQ(42, *retrieved);
    
    dsc_hash_table_destroy(ht, NULL);
}

/* =========================================================
   Get Tests
   ========================================================= */

TEST(hash_table_get_existing) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    dsc_hash_table_insert(ht, "key1", &value);
    
    int* result = (int*)dsc_hash_table_get(ht, "key1");
    ASSERT_NOT_NULL(result);
    ASSERT_EQ(42, *result);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_get_nonexistent) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_hash_table_get(ht, "nonexistent");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_ENOTFOUND, dsc_get_error());
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_get_null_table) {
    void* result = dsc_hash_table_get(NULL, "key1");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(hash_table_get_null_key) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_hash_table_get(ht, NULL);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_get_after_collision) {
    /* Use small table to increase collision chance */
    dsc_hash_table* ht = dsc_hash_table_init(2, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2, v3 = 3;
    dsc_hash_table_insert(ht, "a", &v1);
    dsc_hash_table_insert(ht, "b", &v2);
    dsc_hash_table_insert(ht, "c", &v3);
    
    ASSERT_EQ(1, *(int*)dsc_hash_table_get(ht, "a"));
    ASSERT_EQ(2, *(int*)dsc_hash_table_get(ht, "b"));
    ASSERT_EQ(3, *(int*)dsc_hash_table_get(ht, "c"));
    
    dsc_hash_table_destroy(ht, NULL);
}

/* =========================================================
   Delete Tests
   ========================================================= */

TEST(hash_table_delete_existing) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int value = 42;
    dsc_hash_table_insert(ht, "key1", &value);
    
    void* result = dsc_hash_table_delete(ht, "key1");
    ASSERT_NOT_NULL(result);
    ASSERT_EQ(42, *(int*)result);
    ASSERT_EQ(0, ht->size);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
    
    /* Should not be found anymore */
    ASSERT_NULL(dsc_hash_table_get(ht, "key1"));
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_delete_nonexistent) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_hash_table_delete(ht, "nonexistent");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_ENOTFOUND, dsc_get_error());
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_delete_null_table) {
    void* result = dsc_hash_table_delete(NULL, "key1");
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(hash_table_delete_null_key) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    void* result = dsc_hash_table_delete(ht, NULL);
    ASSERT_NULL(result);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_delete_from_chain) {
    /* Small table to force collisions */
    dsc_hash_table* ht = dsc_hash_table_init(2, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2, v3 = 3;
    dsc_hash_table_insert(ht, "a", &v1);
    dsc_hash_table_insert(ht, "b", &v2);
    dsc_hash_table_insert(ht, "c", &v3);
    
    /* Delete middle element */
    dsc_hash_table_delete(ht, "b");
    
    /* Others should still exist */
    ASSERT_EQ(1, *(int*)dsc_hash_table_get(ht, "a"));
    ASSERT_NULL(dsc_hash_table_get(ht, "b"));
    ASSERT_EQ(3, *(int*)dsc_hash_table_get(ht, "c"));
    
    dsc_hash_table_destroy(ht, NULL);
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
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int v1 = 1, v2 = 2, v3 = 3;
    dsc_hash_table_insert(ht, "key1", &v1);
    dsc_hash_table_insert(ht, "key2", &v2);
    dsc_hash_table_insert(ht, "key3", &v3);
    
    cleanup_call_count = 0;
    dsc_hash_table_destroy(ht, test_cleanup);
    
    ASSERT_EQ(3, cleanup_call_count);
}

TEST(hash_table_destroy_null_table) {
    dsc_hash_table_destroy(NULL, NULL);
    ASSERT_EQ(DSC_EINVAL, dsc_get_error());
}

TEST(hash_table_destroy_empty) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    dsc_hash_table_destroy(ht, NULL);
    ASSERT_EQ(DSC_EOK, dsc_get_error());
}

/* =========================================================
   Stress Tests
   ========================================================= */

TEST(hash_table_stress_many_inserts) {
    dsc_hash_table* ht = dsc_hash_table_init(16, STR_KEY_SIZE, str_hash, str_cmp);
    
    int values[1000];
    char keys[1000][32];
    
    for (int i = 0; i < 1000; i++) {
        values[i] = i;
        snprintf(keys[i], sizeof(keys[i]), "stress_key_%d", i);
        ASSERT_TRUE(dsc_hash_table_insert(ht, keys[i], &values[i]));
    }
    
    ASSERT_EQ(1000, ht->size);
    
    /* Verify all can be retrieved */
    for (int i = 0; i < 1000; i++) {
        int* val = (int*)dsc_hash_table_get(ht, keys[i]);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(i, *val);
    }
    
    dsc_hash_table_destroy(ht, NULL);
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
   Integer Key Tests
   ========================================================= */

/* Integer comparison function */
static int int_cmp(const void* key1, size_t len1, const void* key2, size_t len2) {
    (void)len1;
    (void)len2;
    int k1 = *(const int*)key1;
    int k2 = *(const int*)key2;
    return k1 - k2;
}

/* Integer hash function - FNV-1a variant */
static uint64_t int_hash(const void* key, size_t len) {
    (void)len;
    uint64_t hash = 14695981039346656037ULL;
    const uint8_t* bytes = (const uint8_t*)key;
    for (size_t i = 0; i < sizeof(int); i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

TEST(hash_table_int_keys_basic) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(int), int_hash, int_cmp);
    ASSERT_NOT_NULL(ht);
    
    int key1 = 10, key2 = 20, key3 = 30;
    int val1 = 100, val2 = 200, val3 = 300;
    
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key1, &val1));
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key2, &val2));
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key3, &val3));
    
    ASSERT_EQ(100, *(int*)dsc_hash_table_get(ht, &key1));
    ASSERT_EQ(200, *(int*)dsc_hash_table_get(ht, &key2));
    ASSERT_EQ(300, *(int*)dsc_hash_table_get(ht, &key3));
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_int_keys_delete) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(int), int_hash, int_cmp);
    
    int key = 42;
    int val = 999;
    dsc_hash_table_insert(ht, &key, &val);
    
    int* deleted = (int*)dsc_hash_table_delete(ht, &key);
    ASSERT_NOT_NULL(deleted);
    ASSERT_EQ(999, *deleted);
    ASSERT_NULL(dsc_hash_table_get(ht, &key));
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_int_keys_negative) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(int), int_hash, int_cmp);
    
    int key = -123;
    int val = 456;
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key, &val));
    ASSERT_EQ(456, *(int*)dsc_hash_table_get(ht, &key));
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_int_keys_zero) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(int), int_hash, int_cmp);
    
    int key = 0;
    int val = 777;
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key, &val));
    ASSERT_EQ(777, *(int*)dsc_hash_table_get(ht, &key));
    
    dsc_hash_table_destroy(ht, NULL);
}

/* =========================================================
   Struct Key Tests
   ========================================================= */

typedef struct {
    int id;
    char name[32];
} user_key_t;

/* Struct comparison function */
static int user_cmp(const void* key1, size_t len1, const void* key2, size_t len2) {
    (void)len1;
    (void)len2;
    const user_key_t* u1 = (const user_key_t*)key1;
    const user_key_t* u2 = (const user_key_t*)key2;
    
    if (u1->id != u2->id) return u1->id - u2->id;
    return strcmp(u1->name, u2->name);
}

/* Struct hash function */
static uint64_t user_hash(const void* key, size_t len) {
    const user_key_t* u = (const user_key_t*)key;
    (void)len;
    
    uint64_t hash = 5381;
    /* Hash the ID */
    const uint8_t* bytes = (const uint8_t*)&u->id;
    for (size_t i = 0; i < sizeof(u->id); i++) {
        hash = ((hash << 5) + hash) + bytes[i];
    }
    /* Hash the name */
    for (const char* p = u->name; *p; p++) {
        hash = ((hash << 5) + hash) + (uint8_t)*p;
    }
    return hash;
}

TEST(hash_table_struct_keys_basic) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(user_key_t), user_hash, user_cmp);
    ASSERT_NOT_NULL(ht);
    
    user_key_t key1 = {1, "Alice"};
    user_key_t key2 = {2, "Bob"};
    int val1 = 100, val2 = 200;
    
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key1, &val1));
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key2, &val2));
    
    ASSERT_EQ(100, *(int*)dsc_hash_table_get(ht, &key1));
    ASSERT_EQ(200, *(int*)dsc_hash_table_get(ht, &key2));
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_struct_keys_same_id_diff_name) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(user_key_t), user_hash, user_cmp);
    
    user_key_t key1 = {1, "Alice"};
    user_key_t key2 = {1, "Bob"};  /* Same ID, different name */
    int val1 = 100, val2 = 200;
    
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key1, &val1));
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key2, &val2));
    
    /* Both should be stored as different keys */
    ASSERT_EQ(100, *(int*)dsc_hash_table_get(ht, &key1));
    ASSERT_EQ(200, *(int*)dsc_hash_table_get(ht, &key2));
    
    dsc_hash_table_destroy(ht, NULL);
}

/* =========================================================
   Pointer Key Tests
   ========================================================= */

/* Pointer comparison function */
static int ptr_cmp(const void* key1, size_t len1, const void* key2, size_t len2) {
    (void)len1;
    (void)len2;
    uintptr_t p1 = (uintptr_t)(*(const void**)key1);
    uintptr_t p2 = (uintptr_t)(*(const void**)key2);
    return (p1 > p2) - (p1 < p2);
}

/* Pointer hash function */
static uint64_t ptr_hash(const void* key, size_t len) {
    (void)len;
    uintptr_t ptr = (uintptr_t)(*(const void**)key);
    /* Knuth multiplicative hash */
    return ptr * 2654435761ULL;
}

TEST(hash_table_ptr_keys_basic) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(void*), ptr_hash, ptr_cmp);
    ASSERT_NOT_NULL(ht);
    
    int obj1 = 1, obj2 = 2, obj3 = 3;
    void* ptr1 = &obj1;
    void* ptr2 = &obj2;
    void* ptr3 = &obj3;
    
    int val1 = 10, val2 = 20, val3 = 30;
    
    ASSERT_TRUE(dsc_hash_table_insert(ht, &ptr1, &val1));
    ASSERT_TRUE(dsc_hash_table_insert(ht, &ptr2, &val2));
    ASSERT_TRUE(dsc_hash_table_insert(ht, &ptr3, &val3));
    
    ASSERT_EQ(10, *(int*)dsc_hash_table_get(ht, &ptr1));
    ASSERT_EQ(20, *(int*)dsc_hash_table_get(ht, &ptr2));
    ASSERT_EQ(30, *(int*)dsc_hash_table_get(ht, &ptr3));
    
    dsc_hash_table_destroy(ht, NULL);
}

/* =========================================================
   Edge Case Tests
   ========================================================= */

/* Helper functions for large key test */
typedef struct { char data[1024]; } large_key_t;

static int large_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return memcmp(k1, k2, sizeof(large_key_t));
}

static uint64_t large_hash(const void* k, size_t l) {
    (void)l;
    const uint8_t* bytes = (const uint8_t*)k;
    uint64_t hash = 5381;
    for (size_t i = 0; i < sizeof(large_key_t); i++) {
        hash = ((hash << 5) + hash) + bytes[i];
    }
    return hash;
}

TEST(hash_table_large_key_size) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(large_key_t), large_hash, large_cmp);
    ASSERT_NOT_NULL(ht);
    
    large_key_t key1 = {{0}};
    memset(&key1, 'A', sizeof(key1));
    int val = 42;
    
    ASSERT_TRUE(dsc_hash_table_insert(ht, &key1, &val));
    ASSERT_EQ(42, *(int*)dsc_hash_table_get(ht, &key1));
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_many_int_keys) {
    dsc_hash_table* ht = dsc_hash_table_init(16, sizeof(int), int_hash, int_cmp);
    
    int values[100];
    for (int i = 0; i < 100; i++) {
        int key = i * 7;  /* Use non-sequential keys */
        values[i] = i * 100;
        ASSERT_TRUE(dsc_hash_table_insert(ht, &key, &values[i]));
    }
    
    /* Verify all can be retrieved */
    for (int i = 0; i < 100; i++) {
        int key = i * 7;
        int* val = (int*)dsc_hash_table_get(ht, &key);
        ASSERT_NOT_NULL(val);
        ASSERT_EQ(i * 100, *val);
    }
    
    dsc_hash_table_destroy(ht, NULL);
}

TEST(hash_table_mixed_operations_int_keys) {
    dsc_hash_table* ht = dsc_hash_table_init(8, sizeof(int), int_hash, int_cmp);
    
    /* Insert */
    int k1 = 10, k2 = 20, k3 = 30;
    int v1 = 1, v2 = 2, v3 = 3;
    dsc_hash_table_insert(ht, &k1, &v1);
    dsc_hash_table_insert(ht, &k2, &v2);
    dsc_hash_table_insert(ht, &k3, &v3);
    
    /* Delete middle */
    dsc_hash_table_delete(ht, &k2);
    
    /* Verify others still exist */
    ASSERT_EQ(1, *(int*)dsc_hash_table_get(ht, &k1));
    ASSERT_NULL(dsc_hash_table_get(ht, &k2));
    ASSERT_EQ(3, *(int*)dsc_hash_table_get(ht, &k3));
    
    /* Re-insert */
    int v4 = 4;
    dsc_hash_table_insert(ht, &k2, &v4);
    ASSERT_EQ(4, *(int*)dsc_hash_table_get(ht, &k2));
    
    dsc_hash_table_destroy(ht, NULL);
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
    
    TEST_SECTION("Integer Keys");
    RUN_TEST(hash_table_int_keys_basic);
    RUN_TEST(hash_table_int_keys_delete);
    RUN_TEST(hash_table_int_keys_negative);
    RUN_TEST(hash_table_int_keys_zero);
    
    TEST_SECTION("Struct Keys");
    RUN_TEST(hash_table_struct_keys_basic);
    RUN_TEST(hash_table_struct_keys_same_id_diff_name);
    
    TEST_SECTION("Pointer Keys");
    RUN_TEST(hash_table_ptr_keys_basic);
    
    TEST_SECTION("Edge Cases");
    RUN_TEST(hash_table_large_key_size);
    RUN_TEST(hash_table_many_int_keys);
    RUN_TEST(hash_table_mixed_operations_int_keys);
    
    TEST_SUMMARY();
    return TEST_EXIT_CODE();
}
