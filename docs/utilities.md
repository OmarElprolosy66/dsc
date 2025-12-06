# Utility Functions

**Interoperability and conversion functions for seamless integration between data structures**

## Overview

DSC provides utility functions for converting between C arrays, lists, sets, and extracting data from hash tables. These utilities enable common patterns like duplicate detection, data transformation, and collection conversions.

## Quick Reference

### Array Conversions
```c
void dsc_list_from_array(dsc_list* list, const void* array, size_t count, size_t item_size);
void dsc_set_from_array(dsc_set* set, const void* array, size_t count, size_t item_size, ...);
```

### Collection Conversions
```c
dsc_set  dsc_list_to_set(dsc_list* list, dsc_hashfunc* hf, dsc_cmpfunc* cf);
dsc_list dsc_set_to_list(dsc_set* set);
```

### Duplicate Detection
```c
bool dsc_list_has_duplicates(dsc_list* list, dsc_hashfunc* hf, dsc_cmpfunc* cf);
```

### Hash Table Utilities
```c
dsc_list dsc_hash_table_keys(dsc_hash_table* ht);
dsc_list dsc_hash_table_values(dsc_hash_table* ht);
```

---

## C Array to List

Convert a C array to a dynamic list.

### Example - Integer Array

```c
#include "dsc.h"

int main(void) {
    int numbers[] = {10, 20, 30, 40, 50};
    
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 5);
    dsc_list_from_array(&list, numbers, 5, sizeof(int));
    
    printf("List length: %zu\n", list.length);  // 5
    
    int* first = (int*)dsc_list_get(&list, 0);
    printf("First: %d\n", *first);  // 10
    
    dsc_list_destroy(&list);
}
```

### Example - String Array (Pointers)

```c
const char* names[] = {"Alice", "Bob", "Charlie"};

dsc_list list;
dsc_list_init(&list, sizeof(char*), 3);
dsc_list_from_array(&list, names, 3, sizeof(char*));

char** first_name = (char**)dsc_list_get(&list, 0);
printf("%s\n", *first_name);  // Alice

dsc_list_destroy(&list);
```

### Parameters

- `list` - Pointer to initialized list
- `array` - Pointer to C array
- `count` - Number of elements in array
- `item_size` - Size of each element in bytes

**Note:** The list must be initialized before calling this function.

---

## C Array to Set

Convert a C array to a set (automatically removes duplicates).

### Example - Integer Array with Duplicates

```c
#include "dsc.h"

// Integer hash function (FNV-1a)
uint64_t int_hash(const void* key, size_t len) {
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

int int_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return *(int*)k1 - *(int*)k2;
}

int main(void) {
    int numbers[] = {1, 2, 3, 2, 4, 1, 5};  // Has duplicates
    
    dsc_set set;
    dsc_set_init(&set, 16, sizeof(int), int_hash, int_cmp);
    dsc_set_from_array(&set, numbers, 7, sizeof(int));
    
    printf("Unique values: %zu\n", set.ht->size);  // 5 (not 7)
    
    dsc_set_destroy(&set);
}
```

### Example - String Array

```c
const char* tags[] = {"python", "c", "rust", "python", "java", "c"};

dsc_set set;
dsc_set_init(&set, 16, 0, str_hash, str_cmp);  // key_size=0 for strings
dsc_set_from_array(&set, tags, 6, 0, str_hash, str_cmp);

printf("Unique tags: %zu\n", set.ht->size);  // 4

dsc_set_destroy(&set);
```

### Parameters

- `set` - Pointer to initialized set
- `array` - Pointer to C array
- `count` - Number of elements in array
- `item_size` - Size of each element (0 for variable-length pointers)
- `...` - For variable-length keys (item_size=0): hash and compare functions

**Note:** Duplicates are automatically removed during conversion.

---

## List to Set Conversion

Convert a list to a set, removing duplicates.

### Example - Duplicate Detection Pattern

```c
#include "dsc.h"

int main(void) {
    // Create list with duplicates
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 10);
    
    int values[] = {1, 2, 3, 2, 4, 1};
    for (int i = 0; i < 6; i++) {
        dsc_list_append(&list, &values[i]);
    }
    
    // Convert to set (removes duplicates)
    dsc_set set = dsc_list_to_set(&list, int_hash, int_cmp);
    
    printf("Original list: %zu items\n", list.length);      // 6
    printf("Unique values: %zu items\n", set.ht->size);     // 4
    
    dsc_list_destroy(&list);
    dsc_set_destroy(&set);
}
```

### Use Case - Remove Duplicates from List

```c
// Remove duplicates from a list by converting to set and back
dsc_list original;
dsc_list_init(&original, sizeof(int), 10);
// ... populate list ...

dsc_set unique_set = dsc_list_to_set(&original, int_hash, int_cmp);
dsc_list deduplicated = dsc_set_to_list(&unique_set);

printf("Before: %zu, After: %zu\n", original.length, deduplicated.length);

dsc_list_destroy(&original);
dsc_set_destroy(&unique_set);
dsc_list_destroy(&deduplicated);
```

### Returns

A new `dsc_set` containing unique elements from the list. The set must be destroyed separately.

---

## Set to List Conversion

Convert a set to a list.

### Example

```c
#include "dsc.h"

int main(void) {
    dsc_set set;
    dsc_set_init(&set, 16, sizeof(int), int_hash, int_cmp);
    
    int values[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        dsc_set_add(&set, &values[i]);
    }
    
    // Convert to list
    dsc_list list = dsc_set_to_list(&set);
    
    printf("List has %zu items\n", list.length);  // 3
    
    // Iterate over list
    for (size_t i = 0; i < list.length; i++) {
        int* val = (int*)dsc_list_get(&list, i);
        printf("%d ", *val);
    }
    
    dsc_set_destroy(&set);
    dsc_list_destroy(&list);
}
```

### Returns

A new `dsc_list` containing all elements from the set. The list must be destroyed separately.

**Note:** Order of elements is not guaranteed (depends on hash table iteration order).

---

## Duplicate Detection

Check if a list contains duplicate elements (LeetCode pattern).

### Example - LeetCode "Contains Duplicate"

```c
#include "dsc.h"

bool containsDuplicate(int* nums, int numsSize) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), numsSize);
    dsc_list_from_array(&list, nums, numsSize, sizeof(int));
    
    bool has_dups = dsc_list_has_duplicates(&list, int_hash, int_cmp);
    
    dsc_list_destroy(&list);
    return has_dups;
}

int main(void) {
    int arr1[] = {1, 2, 3, 1};
    int arr2[] = {1, 2, 3, 4};
    
    printf("arr1 has duplicates: %s\n", 
           containsDuplicate(arr1, 4) ? "true" : "false");  // true
    
    printf("arr2 has duplicates: %s\n", 
           containsDuplicate(arr2, 4) ? "true" : "false");  // false
}
```

### Example - String Duplicate Detection

```c
const char* words[] = {"hello", "world", "hello", "foo"};

dsc_list list;
dsc_list_init(&list, sizeof(char*), 4);
dsc_list_from_array(&list, words, 4, sizeof(char*));

if (dsc_list_has_duplicates(&list, str_hash, str_cmp)) {
    printf("Found duplicate words!\n");
}

dsc_list_destroy(&list);
```

### How It Works

1. Creates a temporary set from the list
2. Compares set size with list length
3. If sizes differ, duplicates exist
4. Returns `true` if duplicates found, `false` otherwise

**Time Complexity:** O(n) average case  
**Space Complexity:** O(n) temporary set

---

## Hash Table Keys

Extract all keys from a hash table as a list.

### Example

```c
#include "dsc.h"

int main(void) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, 0, str_hash, str_cmp);
    
    int v1 = 10, v2 = 20, v3 = 30;
    dsc_hash_table_insert(&ht, "alice", &v1);
    dsc_hash_table_insert(&ht, "bob", &v2);
    dsc_hash_table_insert(&ht, "charlie", &v3);
    
    // Get all keys
    dsc_list keys = dsc_hash_table_keys(&ht);
    
    printf("Keys (%zu):\n", keys.length);
    for (size_t i = 0; i < keys.length; i++) {
        char** key = (char**)dsc_list_get(&keys, i);
        printf("  - %s\n", *key);
    }
    
    dsc_hash_table_destroy(&ht, NULL);
    dsc_list_destroy(&keys);
}
```

### Output
```
Keys (3):
  - alice
  - bob
  - charlie
```

### Use Case - Process All Keys

```c
dsc_list keys = dsc_hash_table_keys(&config_table);

for (size_t i = 0; i < keys.length; i++) {
    char** key = (char**)dsc_list_get(&keys, i);
    printf("Config key: %s\n", *key);
}

dsc_list_destroy(&keys);
```

### Returns

A new `dsc_list` containing pointers to all keys. The list must be destroyed separately.

**Note:** Keys point to internal hash table storage - do not modify or free them.

---

## Hash Table Values

Extract all values from a hash table as a list.

### Example

```c
#include "dsc.h"

int main(void) {
    dsc_hash_table scores;
    dsc_hash_table_init(&scores, 16, 0, str_hash, str_cmp);
    
    int s1 = 95, s2 = 87, s3 = 92;
    dsc_hash_table_insert(&scores, "alice", &s1);
    dsc_hash_table_insert(&scores, "bob", &s2);
    dsc_hash_table_insert(&scores, "charlie", &s3);
    
    // Get all values
    dsc_list values = dsc_hash_table_values(&scores);
    
    printf("Scores (%zu):\n", values.length);
    for (size_t i = 0; i < values.length; i++) {
        int** score = (int**)dsc_list_get(&values, i);
        printf("  - %d\n", **score);
    }
    
    dsc_hash_table_destroy(&scores, NULL);
    dsc_list_destroy(&values);
}
```

### Use Case - Calculate Statistics

```c
dsc_list values = dsc_hash_table_values(&scores);

int total = 0;
for (size_t i = 0; i < values.length; i++) {
    int** val = (int**)dsc_list_get(&values, i);
    total += **val;
}

double average = (double)total / values.length;
printf("Average score: %.2f\n", average);

dsc_list_destroy(&values);
```

### Returns

A new `dsc_list` containing pointers to all values. The list must be destroyed separately.

**Note:** Values point to internal hash table storage - modifications affect the original table.

---

## Complete Example - LeetCode Pattern

Solving LeetCode's "Contains Duplicate" problem using DSC utilities:

```c
#include "dsc.h"

// Hash function for integers
uint64_t int_hash(const void* key, size_t len) {
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

int int_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return *(int*)k1 - *(int*)k2;
}

bool containsDuplicate(int* nums, int numsSize) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), numsSize);
    dsc_list_from_array(&list, nums, numsSize, sizeof(int));
    
    bool result = dsc_list_has_duplicates(&list, int_hash, int_cmp);
    
    dsc_list_destroy(&list);
    return result;
}

int main(void) {
    // Test case 1: [1,2,3,1] -> true
    int test1[] = {1, 2, 3, 1};
    printf("Test 1: %s\n", containsDuplicate(test1, 4) ? "true" : "false");
    
    // Test case 2: [1,2,3,4] -> false
    int test2[] = {1, 2, 3, 4};
    printf("Test 2: %s\n", containsDuplicate(test2, 4) ? "true" : "false");
    
    // Test case 3: [1,1,1,3,3,4,3,2,4,2] -> true
    int test3[] = {1, 1, 1, 3, 3, 4, 3, 2, 4, 2};
    printf("Test 3: %s\n", containsDuplicate(test3, 10) ? "true" : "false");
    
    return 0;
}
```

**Output:**
```
Test 1: true
Test 2: false
Test 3: true
```

---

## Error Handling

All utility functions follow DSC's error handling conventions:

```c
// Check for NULL returns
dsc_list keys = dsc_hash_table_keys(&ht);
if (keys.items == NULL) {
    dsc_error_t err = dsc_get_error();
    fprintf(stderr, "Error: %s\n", dsc_strerror(err));
}

// Check boolean returns
bool has_dups = dsc_list_has_duplicates(&list, hash_fn, cmp_fn);
if (dsc_get_error() != DSC_EOK) {
    fprintf(stderr, "Error checking duplicates\n");
}
```

---

## Performance Notes

### Time Complexity

| Function | Time | Notes |
|----------|------|-------|
| `list_from_array` | O(n) | Linear copy |
| `set_from_array` | O(n) | Average case with hash |
| `list_to_set` | O(n) | Average case |
| `set_to_list` | O(n) | Iteration over set |
| `list_has_duplicates` | O(n) | Average case (creates temp set) |
| `hash_table_keys` | O(n) | Iteration over table |
| `hash_table_values` | O(n) | Iteration over table |

### Memory Usage

- Conversion functions create new data structures
- Remember to destroy both original and converted collections
- `list_has_duplicates` creates a temporary set (freed internally)

---

## Best Practices

### 1. Always Initialize Before Conversion

```c
// CORRECT
dsc_list list;
dsc_list_init(&list, sizeof(int), 10);
dsc_list_from_array(&list, array, count, sizeof(int));

// WRONG - list not initialized
dsc_list list;
dsc_list_from_array(&list, array, count, sizeof(int));  // Undefined behavior
```

### 2. Destroy All Collections

```c
dsc_list original;
dsc_list_init(&original, sizeof(int), 5);

dsc_set converted = dsc_list_to_set(&original, int_hash, int_cmp);

// Destroy BOTH
dsc_list_destroy(&original);
dsc_set_destroy(&converted);  // Don't forget this!
```

### 3. Check for Errors

```c
dsc_list keys = dsc_hash_table_keys(&ht);
if (keys.items == NULL) {
    // Handle error
    return;
}
dsc_list_destroy(&keys);
```

### 4. Use Appropriate Hash Functions

```c
// For integers - FNV-1a
uint64_t int_hash(...);

// For strings - djb2
uint64_t str_hash(...);

// For custom types - implement your own
uint64_t custom_hash(...);
```

---

## See Also

- [Hash Table Documentation](hash_table.md)
- [List Documentation](list.md)
- [Set Documentation](set.md)
- [Main Documentation](README.md)
