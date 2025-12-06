# DSC Documentation

Example-focused documentation for the Data Structure Collection library.

## Data Structures

- **[Hash Table](hash_table.md)** - O(1) average insert/lookup/delete with generic keys
- **[Dynamic List](list.md)** - Growable array with map/filter/foreach
- **[Set](set.md)** - Hash-based set with automatic duplicate prevention

---

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/OmarElprolosy66/dsc.git
cd dsc
```

### Usage

**Single header library - just include it:**

```c
#define DSC_IMPLEMENTATION  // In ONE .c file only
#include "dsc.h"
```

In all other files:

```c
#include "dsc.h"  // No DSC_IMPLEMENTATION
```

---

## 30-Second Examples

### Hash Table

```c
dsc_hash_table ht;
dsc_hash_table_init(&ht, 16, 0, str_hash, str_cmp);

int value = 42;
dsc_hash_table_insert(&ht, "answer", &value);

int* result = (int*)dsc_hash_table_get(&ht, "answer");
printf("%d\n", *result);  // 42

dsc_hash_table_destroy(&ht, NULL);
```

### List

```c
dsc_list nums;
dsc_list_init(&nums, sizeof(int), 10);

int x = 10;
dsc_list_append(&nums, &x);

int* first = (int*)dsc_list_get(&nums, 0);
printf("%d\n", *first);  // 10

dsc_list_destroy(&nums);
```

### Set

```c
dsc_set tags;
dsc_set_init(&tags, 16, 0, str_hash, str_cmp);

dsc_set_add(&tags, "python");
dsc_set_add(&tags, "c");
dsc_set_add(&tags, "python");  // Ignored - duplicate

printf("Size: %zu\n", tags.ht->size);  // 2

dsc_set_destroy(&tags);
```

---

## Common Patterns

### Error Handling

```c
if (!dsc_hash_table_insert(&ht, key, value)) {
    fprintf(stderr, "Error: %s\n", dsc_strerror(dsc_get_error()));
}
```

### Hash Functions

```c
// Strings (djb2)
uint64_t str_hash(const void* key, size_t len) {
    const char* str = (const char*)key;
    uint64_t hash = 5381;
    (void)len;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + (uint64_t)c;
    return hash;
}

// Integers (FNV-1a)
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

// Pointers (Knuth multiplicative)
uint64_t ptr_hash(const void* key, size_t len) {
    (void)len;
    uintptr_t ptr = (uintptr_t)(*(void**)key);
    return ptr * 2654435761ULL;
}
```

### Comparison Functions

```c
// Strings
int str_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return strcmp((const char*)k1, (const char*)k2);
}

// Integers
int int_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return *(int*)k1 - *(int*)k2;
}

// Structs
int struct_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return memcmp(k1, k2, l1);
}
```

---

## Type-Safe Wrappers

```c
// Define once
DSC_DEFINE_HASH_TABLE(int, char*, int)  // Key=int, Value=char*, Name=int
DSC_DEFINE_LIST(int, int)                // Type=int, Name=int
DSC_DEFINE_SET(int, int)                 // Type=int, Name=int

// Use with type safety
int_table map;
int_table_init(&map, 16, int_hash, int_cmp);

int key = 42;
char* value = "answer";
int_table_insert(&map, &key, value);

char* result = int_table_get(&map, &key);

int_table_destroy(&map, NULL);
```

---

## Grep-Friendly Tags

Search documentation with these tags:

```bash
# Find all examples
grep -r "int main" docs/

# Find error handling examples
grep -r "dsc_get_error" docs/

# Find specific data structure usage
grep -r "dsc_hash_table_init" docs/
grep -r "dsc_list_append" docs/
grep -r "dsc_set_add" docs/

# Find use cases
grep -r "Use Case:" docs/

# Find specific key types
grep -r "Integer Keys" docs/
grep -r "String Keys" docs/
grep -r "Struct Keys" docs/
```

---

## Building Tests

```bash
cd tests
./run_tests.bat    # Windows
./run_tests.sh     # Linux/macOS
```

---

## License

LGPL v3.0 - See LICENSE file for details.

---

## Contributing

See main README for contribution guidelines.
