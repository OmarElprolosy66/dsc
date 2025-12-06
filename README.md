# DSC — Data Structure Collection

[![License: LGPL v3](https://img.shields.io/badge/License-LGPLv3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![C Standard](https://img.shields.io/badge/C-C99%2B-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()
[![Status](https://img.shields.io/badge/Status-Early%20Development-red.svg)]()

> ⚠️ **WARNING:** This library is in **EARLY DEVELOPMENT** and is **NOT READY FOR PRODUCTION USE**. The API is unstable and may change without notice. Use at your own risk. The author assumes NO RESPONSIBILITY for any damages, data loss, or issues arising from the use of this library in any environment.

A lightweight, single-header C library for common data structures. STB-style, zero dependencies, cross-platform.

## Features

- **Hash Table** — O(1) insert/lookup/delete, generic keys (string/int/struct/pointer)
- **Dynamic List** — Growable array with map/filter/foreach
- **Set** — Hash-based set with duplicate prevention
- **Type-Safe** — Generic macros for compile-time safety
- **Error System** — Thread-local errno-style handling
- **Zero Deps** — Only standard C library
- **Cross-Platform** — Windows, Linux, macOS, BSD

## Quick Start

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"

int main(void) {
    // Hash table with string keys
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, 0, str_hash, str_cmp);
    
    int value = 42;
    dsc_hash_table_insert(&ht, "answer", &value);
    
    int* result = (int*)dsc_hash_table_get(&ht, "answer");
    printf("%d\n", *result);  // 42
    
    dsc_hash_table_destroy(&ht, NULL);
}
```

**[Full Documentation & Examples](docs/README.md)**

## Installation

**Download** [`dsc.h`](dsc.h) and add to your project.

```c
#define DSC_IMPLEMENTATION  // In ONE .c file only
#include "dsc.h"
```

**Or clone:**

```bash
git clone https://github.com/OmarElprolosy66/dsc.git
cp dsc/dsc.h /path/to/your/project/
```

## Examples

### Hash Table - String Keys

```c
// Hash function (djb2)
uint64_t str_hash(const void* key, size_t len) {
    const char* str = (const char*)key;
    uint64_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + (uint64_t)c;
    return hash;
}

// Comparison function
int str_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    return strcmp((const char*)k1, (const char*)k2);
}

int main(void) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, 0, str_hash, str_cmp);
    
    int age = 25;
    dsc_hash_table_insert(&ht, "alice", &age);
    
    int* result = (int*)dsc_hash_table_get(&ht, "alice");
    printf("Age: %d\n", *result);
    
    dsc_hash_table_destroy(&ht, NULL);
}
```

**More examples:** [Integer keys](docs/hash_table.md#integer-keys), [Struct keys](docs/hash_table.md#struct-keys), [Caching](docs/hash_table.md#use-case-caching)

### Dynamic List

```c
dsc_list numbers;
dsc_list_init(&numbers, sizeof(int), 10);

// Append
int values[] = {10, 20, 30};
for (int i = 0; i < 3; i++) {
    dsc_list_append(&numbers, &values[i]);
}

// Get
int* first = (int*)dsc_list_get(&numbers, 0);
printf("%d\n", *first);  // 10

// Map operation
void double_value(void* item) {
    *(int*)item *= 2;
}
dsc_list_map(&numbers, double_value);

dsc_list_destroy(&numbers);
```

**More examples:** [Filter](docs/list.md#filter-operation), [Type-safe wrappers](docs/list.md#type-safe-wrapper), [Use cases](docs/list.md#use-case-command-queue)

### Set

```c
dsc_set tags;
dsc_set_init(&tags, 16, 0, str_hash, str_cmp);

dsc_set_add(&tags, "python");
dsc_set_add(&tags, "c");
dsc_set_add(&tags, "python");  // Duplicate - ignored

printf("Unique tags: %zu\n", tags.ht->size);  // 2

if (dsc_set_get(&tags, "c")) {
    printf("Found 'c'\n");
}

dsc_set_destroy(&tags);
```

**More examples:** [Deduplication](docs/set.md#use-case-email-deduplication), [Graph traversal](docs/set.md#use-case-visited-nodes-graph-traversal)

### Type-Safe Wrappers

```c
// Define once
DSC_DEFINE_LIST(int, int)

int main(void) {
    // No more void* casting!
    int_list nums;
    int_list_init(&nums, 10);
    
    int_list_append(&nums, 42);  // Pass by value
    int first = int_list_get(&nums, 0);  // Returns by value
    
    int_list_destroy(&nums);
}
```

## Error Handling

```c
if (!dsc_hash_table_insert(&ht, key, value)) {
    dsc_error_t err = dsc_get_error();
    fprintf(stderr, "Error: %s\n", dsc_strerror(err));
}
```

**Error codes:** `DSC_EOK`, `DSC_ENOMEM`, `DSC_EINVAL`, `DSC_ENOTFOUND`, `DSC_EEXISTS`, `DSC_ERANGE`, `DSC_EEMPTY`

## Documentation

- **[Overview & Quick Start](docs/README.md)**
- **[Hash Table Guide](docs/hash_table.md)** — All key types, use cases, examples
- **[List Guide](docs/list.md)** — Map/filter/foreach, use cases, examples
- **[Set Guide](docs/set.md)** — Deduplication, membership testing, examples

## API Reference

### Hash Table

```c
void   dsc_hash_table_init(dsc_hash_table *ht, size_t capacity, size_t key_size, dsc_hashfunc *hf, dsc_cmpfunc *cf);
bool   dsc_hash_table_insert(dsc_hash_table *ht, const void *key, void *value);
void*  dsc_hash_table_get(dsc_hash_table *ht, const void *key);
void*  dsc_hash_table_delete(dsc_hash_table *ht, const void *key);
void   dsc_hash_table_destroy(dsc_hash_table *ht, dsc_cleanupfunc *cf);
void   dsc_hash_table_clear(dsc_hash_table *ht, dsc_cleanupfunc *cf);
```

### List

```c
void     dsc_list_init(dsc_list* list, size_t item_size, size_t initial_capacity);
void     dsc_list_append(dsc_list* list, void* item);
void*    dsc_list_get(dsc_list* list, size_t index);
void     dsc_list_pop(dsc_list* list);
void     dsc_list_map(dsc_list* list, dsc_callback cf);
void     dsc_list_foreach(dsc_list* list, dsc_callback cf);
dsc_list dsc_list_filter(dsc_list* list, dsc_predicate cf);
void     dsc_list_destroy(dsc_list* list);
```

### Set

```c
void  dsc_set_init(dsc_set* set, size_t initial_capacity, size_t key_size, dsc_hashfunc* hf, dsc_cmpfunc* cf);
bool  dsc_set_add(dsc_set* set, const void* item);
void* dsc_set_get(dsc_set* set, const void* item);
void  dsc_set_remove(dsc_set* set, const void* item);
void  dsc_set_destroy(dsc_set* set);
```

## Building & Testing

```bash
cd tests
./run_tests.bat    # Windows
./run_tests.sh     # Linux/macOS
```

All 69 tests must pass.

## Contributing

1. **Fork & clone** the repository
2. **Create a branch** for your feature
3. **Add tests** for new functionality
4. **Run all tests** and ensure they pass
5. **Follow code style** (see existing code)
6. **Submit pull request**

### Code Style

- C99 standard
- 4 spaces indentation
- Clear variable names
- Comment complex logic
- Add examples to docs

### Commit Messages

```
feat(hash_table): add clear function
fix(list): correct bounds checking in get
docs(set): add deduplication example
test(hash_table): add stress tests
```

## License

**GNU Lesser General Public License v3.0**

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

See [LICENSE](LICENSE) file for full text.

## Acknowledgments

- Inspired by [STB libraries](https://github.com/nothings/stb)
- djb2 hash algorithm by Daniel J. Bernstein
- FNV-1a hash algorithm by Glenn Fowler, Landon Curt Noll, and Kiem-Phong Vo

## Links

- **Repository:** https://github.com/OmarElprolosy66/dsc
- **Documentation:** [docs/](docs/)
- **Issues:** https://github.com/OmarElprolosy66/dsc/issues
- **License:** [LGPL v3.0](LICENSE)
