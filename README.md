# DSC — Data Structure Collection

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C Standard](https://img.shields.io/badge/C-C99%2B-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()
[![Status](https://img.shields.io/badge/Status-Early%20Development-red.svg)]()

> ⚠️ **IMPORTANT:** This library is currently in early development and is **NOT READY FOR PRODUCTION OR COMMERCIAL USE**. The API is unstable and may change significantly between versions. Use at your own risk.

> 💡 **Type-Safe Usage:** DSC provides **type-safe generic macros** (eg. `DSC_DEFINE_LIST`, `DSC_DEFINE_HASH_TABLE`) that generate wrapper functions for your specific types. We recommend using these macros for the best experience and compile-time type safety. While raw void\* functions are available, the type-safe wrappers eliminate casting and make your code cleaner and safer. Check out the [Type-Safe Wrappers](#type-safe-wrappers) section to see how easy it is!

A lightweight, single-header C library providing efficient implementations of common data structures. Designed in the style of [STB libraries](https://github.com/nothings/stb) for maximum portability and ease of integration.

## Features

- **Hash Table** — O(1) average insert/lookup/delete with automatic resizing with Generic keys (int, string, struct, pointer)
- **Dynamic List** — Growable array with map, filter, and foreach operations
- **Type-Safe Generics** — Compile-time type safety via macros
- **Error Handling** — Thread-local errno-style error system
- **Zero Dependencies** — Only requires standard C library
- **Cross-Platform** — Windows, Linux, macOS, BSD
- **Flexible Linking** — Header-only, static library, or shared library

## Table of Contents

- [DSC — Data Structure Collection](#dsc--data-structure-collection)
  - [Features](#features)
  - [Table of Contents](#table-of-contents)
  - [Quick Start](#quick-start)
  - [Installation](#installation)
    - [Option 1: Single Header (Recommended)](#option-1-single-header-recommended)
    - [Option 2: Git Clone](#option-2-git-clone)
    - [Option 3: Git Submodule](#option-3-git-submodule)
  - [Usage Examples](#usage-examples)
    - [Hash Table](#hash-table)
    - [Dynamic List](#dynamic-list)
    - [Type-Safe Wrappers](#type-safe-wrappers)
    - [Error Handling](#error-handling)
    - [Raw API (void\* Functions)](#raw-api-void-functions)
  - [Building](#building)
    - [Header-Only (Recommended)](#header-only-recommended)
    - [Static Library](#static-library)
    - [Shared/Dynamic Library](#shareddynamic-library)
    - [Build Flags Reference](#build-flags-reference)
  - [API Reference](#api-reference)
    - [Error Functions](#error-functions)
    - [Hash Table](#hash-table-1)
    - [Dynamic List](#dynamic-list-1)
    - [Type-Safe Macros](#type-safe-macros)
  - [Running Tests](#running-tests)
  - [Contributing](#contributing)
    - [Code Style](#code-style)
    - [Example Style](#example-style)
    - [Commit Messages](#commit-messages)
    - [Pull Request Process](#pull-request-process)
    - [Adding New Data Structures](#adding-new-data-structures)
    - [Reporting Issues](#reporting-issues)
  - [License](#license)
  - [Acknowledgments](#acknowledgments)

## Quick Start

**Step 1:** Copy `dsc.h` into your project.

**Step 2:** In **one** source file, define `DSC_IMPLEMENTATION` before including:

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
```

**Step 3:** In all other files, just include normally:

```c
#include "dsc.h"
```

That's it! No build system, no dependencies.

## Installation

### Option 1: Single Header (Recommended)

Simply download [`dsc.h`](dsc.h) and add it to your project.

### Option 2: Git Clone

```bash
git clone https://github.com/OmarElprolosy66/dsc.git
cp dsc/dsc.h /path/to/your/project/
```

### Option 3: Git Submodule

```bash
git submodule add https://github.com/OmarElprolosy66/dsc.git libs/dsc
```

Then include with:
```c
#include "libs/dsc/dsc.h"
```

## Usage Examples

### Hash Table

The hash table supports **any key type** (strings, integers, structs, pointers). You provide custom hash and comparison functions.

**Example: String Keys**

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>
#include <string.h>

// String comparison function
int str_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return strcmp((const char*)k1, (const char*)k2);
}

// String hash function (djb2)
uint64_t str_hash(const void* key, size_t len) {
    const char* str = (const char*)key;
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}

int main(void) {
    // Create hash table: capacity=64, key_size=0 (variable-length strings)
    dsc_hash_table* ht = dsc_hash_table_init(64, 0, str_hash, str_cmp);
    if (ht == NULL) {
        fprintf(stderr, "Error: %s\n", dsc_strerror(dsc_get_error()));
        return 1;
    }

    // Insert key-value pairs
    int age = 25;
    dsc_hash_table_insert(ht, "alice", &age);

    int score = 100;
    dsc_hash_table_insert(ht, "bob", &score);

    // Retrieve values
    int* alice_age = (int*)dsc_hash_table_get(ht, "alice");
    if (alice_age != NULL) {
        printf("Alice's age: %d\n", *alice_age);
    }

    // Delete a key
    int* deleted = (int*)dsc_hash_table_delete(ht, "bob");
    if (deleted != NULL) {
        printf("Deleted bob's score: %d\n", *deleted);
    }

    // Cleanup
    dsc_hash_table_destroy(ht, NULL);
    return 0;
}
```

**Example: Integer Keys**

```c
// Integer comparison function
int int_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return *(int*)k1 - *(int*)k2;
}

// Integer hash function (FNV-1a)
uint64_t int_hash(const void* key, size_t len) {
    (void)len;
    uint64_t hash = 14695981039346656037ULL;
    const uint8_t* bytes = (const uint8_t*)key;
    for (size_t i = 0; i < sizeof(int); i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

int main(void) {
    // Create hash table with fixed-size integer keys
    dsc_hash_table* ht = dsc_hash_table_init(64, sizeof(int), int_hash, int_cmp);
    
    int key = 42;
    char* value = "The Answer";
    dsc_hash_table_insert(ht, &key, value);
    
    char* result = (char*)dsc_hash_table_get(ht, &key);
    printf("%d -> %s\n", key, result);  // "42 -> The Answer"
    
    dsc_hash_table_destroy(ht, NULL);
    return 0;
}
```

### Dynamic List

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>

void print_int(void* item) {
    printf("%d ", *(int*)item);
}

int is_even(void* item) {
    return (*(int*)item % 2) == 0;
}

int main(void) {
    dsc_list numbers;
    dsc_list_init(&numbers, sizeof(int), 16);

    // Append items
    for (int i = 1; i <= 10; i++) {
        dsc_list_append(&numbers, &i);
    }

    // Iterate with foreach
    printf("All numbers: ");
    dsc_list_foreach(&numbers, print_int);
    printf("\n");

    // Filter even numbers
    dsc_list evens = dsc_list_filter(&numbers, is_even);
    printf("Even numbers: ");
    dsc_list_foreach(&evens, print_int);
    printf("\n");

    // Access by index
    int* third = (int*)dsc_list_get(&numbers, 2);
    if (third != NULL) {
        printf("Third element: %d\n", *third);
    }

    // Pop last element
    dsc_list_pop(&numbers);
    printf("After pop, length: %zu\n", numbers.length);

    // Cleanup
    dsc_list_destroy(&evens);
    dsc_list_destroy(&numbers);
    return 0;
}
```

### Type-Safe Wrappers

DSC provides macros to generate type-safe wrapper functions:

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>

// Generate type-safe list for integers
DSC_DEFINE_LIST(int, int)

// Generate type-safe list for a custom struct
typedef struct {
    char name[32];
    int age;
} Person;

DSC_DEFINE_LIST(Person, person)

int main(void) {
    // Type-safe integer list
    int_list numbers = int_list_init(16);
    
    int_list_append(&numbers, 10);
    int_list_append(&numbers, 20);
    int_list_append(&numbers, 30);
    
    int val = int_list_get(&numbers, 1);  // Returns int, not void*
    printf("Value at index 1: %d\n", val);
    
    int_list_destroy(&numbers);

    // Type-safe struct list
    person_list people = person_list_init(8);
    
    Person alice = {"Alice", 25};
    Person bob = {"Bob", 30};
    
    person_list_append(&people, alice);
    person_list_append(&people, bob);
    
    Person p = person_list_get(&people, 0);
    printf("First person: %s, age %d\n", p.name, p.age);
    
    person_list_destroy(&people);
    return 0;
}
```

### Error Handling

DSC uses a thread-local error system similar to `errno`:

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>

int main(void) {
    dsc_list list;
    dsc_list_init(&list, sizeof(int), 16);

    // Try to access invalid index
    void* item = dsc_list_get(&list, 100);
    
    if (item == NULL) {
        dsc_error_t err = dsc_get_error();
        
        switch (err) {
            case DSC_ERANGE:
                fprintf(stderr, "Index out of range!\n");
                break;
            case DSC_EINVAL:
                fprintf(stderr, "Invalid argument!\n");
                break;
            default:
                fprintf(stderr, "Error: %s\n", dsc_strerror(err));
        }
    }

    // Clear error state
    dsc_clear_error();

    dsc_list_destroy(&list);
    return 0;
}
```

**Available Error Codes:**

| Error Code | Description |
|------------|-------------|
| `DSC_EOK` | Success (no error) |
| `DSC_ENOMEM` | Memory allocation failed |
| `DSC_EINVAL` | Invalid argument |
| `DSC_ENOTFOUND` | Key or element not found |
| `DSC_EEXISTS` | Key already exists |
| `DSC_ERANGE` | Index out of range |
| `DSC_EEMPTY` | Container is empty |
| `DSC_EHASHFUNC` | Hash function is NULL or invalid |

### Raw API (void\* Functions)

For advanced use cases where you need more flexibility, DSC exposes raw void\* functions. This is useful when working with mixed types or when type-safe wrappers would be too restrictive:

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>

typedef struct {
    char name[32];
    int age;
    float height;
} Person;

int main(void) {
    // Create a generic list that can hold any data
    dsc_list mixed_list;
    dsc_list_init(&mixed_list, sizeof(void*), 16);

    // Store different types by their pointers
    int numbers[] = {10, 20, 30};
    Person people[] = {
        {"Alice", 25, 5.6f},
        {"Bob", 30, 5.9f}
    };

    // Append pointers to different types
    for (int i = 0; i < 3; i++) {
        dsc_list_append(&mixed_list, &numbers[i]);
    }

    for (int i = 0; i < 2; i++) {
        dsc_list_append(&mixed_list, &people[i]);
    }

    // Retrieve and work with raw pointers
    int* first_num = *(int**)dsc_list_get(&mixed_list, 0);
    if (first_num != NULL) {
        printf("First number: %d\n", *first_num);
    }

    Person* first_person = *(Person**)dsc_list_get(&mixed_list, 3);
    if (first_person != NULL) {
        printf("First person: %s, age %d\n", first_person->name, first_person->age);
    }

    dsc_list_destroy(&mixed_list);
    return 0;
}
```

**Raw API Functions:**

- `dsc_hash_table_get()`, `dsc_hash_table_insert()`, `dsc_hash_table_delete()` — Work with void\* pointers
- `dsc_list_get()`, `dsc_list_append()` — Return/accept void\* pointers
- `dsc_list_map()`, `dsc_list_foreach()` — Execute callbacks on void\* items
- `dsc_list_filter()` — Filter using predicates on void\* items

> **Note:** When using raw void\* APIs, you're responsible for:
> - Correct pointer casting and dereferencing
> - Type consistency (don't mix types in the same container unless intended)
> - Memory management of the actual data (DSC manages containers only)

## Building

### Header-Only (Recommended)

No build step required! Just include the header:

```c
// In ONE .c file:
#define DSC_IMPLEMENTATION
#include "dsc.h"

// In all other files:
#include "dsc.h"
```

Compile your project normally:

```bash
gcc -o myapp main.c
```

### Static Library

**Linux/macOS:**

```bash
# Create implementation file
echo '#define DSC_IMPLEMENTATION' > dsc.c
echo '#include "dsc.h"' >> dsc.c

# Compile to object file
gcc -c -O2 -o dsc.o dsc.c

# Create static library
ar rcs libdsc.a dsc.o

# Link with your project
gcc -o myapp main.c -L. -ldsc
```

**Windows (MSVC):**

```cmd
REM Create implementation file
echo #define DSC_IMPLEMENTATION > dsc.c
echo #include "dsc.h" >> dsc.c

REM Compile to object file
cl /c /O2 dsc.c

REM Create static library
lib /OUT:dsc.lib dsc.obj

REM Link with your project
cl /O2 main.c dsc.lib
```

### Shared/Dynamic Library

**Linux:**

```bash
# Create implementation file
echo '#define DSC_IMPLEMENTATION' > dsc.c
echo '#include "dsc.h"' >> dsc.c

# Build shared library
gcc -DDSC_SHARED -DDSC_BUILD -fPIC -shared -O2 -o libdsc.so dsc.c

# Compile your project
gcc -DDSC_SHARED -o myapp main.c -L. -ldsc -Wl,-rpath,'$ORIGIN'

# Or set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
./myapp
```

**macOS:**

```bash
# Build shared library
gcc -DDSC_SHARED -DDSC_BUILD -fPIC -shared -O2 -o libdsc.dylib dsc.c

# Compile your project
gcc -DDSC_SHARED -o myapp main.c -L. -ldsc
```

**Windows (GCC/MinGW):**

```bash
# Build DLL
gcc -DDSC_SHARED -DDSC_BUILD -shared -O2 -o dsc.dll dsc.c

# Compile your project
gcc -DDSC_SHARED -o myapp.exe main.c -L. -ldsc
```

**Windows (MSVC):**

```cmd
REM Build DLL
cl /DDSC_SHARED /DDSC_BUILD /LD /O2 dsc.c

REM Compile your project
cl /DDSC_SHARED /O2 main.c dsc.lib
```

### Build Flags Reference

| Flag | Description |
|------|-------------|
| `DSC_IMPLEMENTATION` | Include the implementation (define in ONE .c file) |
| `DSC_SHARED` | Build/use as shared library |
| `DSC_BUILD` | Define when building the shared library |
| `DSC_PREFIX` | Custom function prefix (default: `dsc_`) |

## API Reference

### Error Functions

```c
dsc_error_t  dsc_get_error(void);           // Get last error code
const char*  dsc_strerror(dsc_error_t err); // Get error message
void         dsc_clear_error(void);          // Clear error state
```

### Hash Table

**Type Definitions:**
```c
typedef uint64_t dsc_hashfunc(const void* key, size_t key_size);
typedef int      dsc_cmpfunc(const void* key1, size_t len1, const void* key2, size_t len2);
typedef void     dsc_cleanupfunc(void* obj);
```

**Functions:**
```c
// Initialize hash table
// capacity: initial size, key_size: size of keys (0 for variable-length strings)
// hf: hash function, cf: comparison function
dsc_hash_table* dsc_hash_table_init(size_t capacity, size_t key_size, 
                                     dsc_hashfunc *hf, dsc_cmpfunc *cf);

// Insert key-value pair (returns false if key exists or on error)
bool            dsc_hash_table_insert(dsc_hash_table *ht, const void *key, void *obj);

// Get value by key (returns NULL if not found)
void*           dsc_hash_table_get(dsc_hash_table *ht, const void *key);

// Delete key and return its value (returns NULL if not found)
void*           dsc_hash_table_delete(dsc_hash_table *ht, const void *key);

// Destroy hash table (cf is optional cleanup function for values)
void            dsc_hash_table_destroy(dsc_hash_table *ht, dsc_cleanupfunc *cf);
```

**Supported Key Types:**
- **Strings** — Use `key_size = 0` for variable-length null-terminated strings
- **Integers** — Use `key_size = sizeof(int)` (or other numeric type)
- **Structs** — Use `key_size = sizeof(YourStruct)`
- **Pointers** — Use `key_size = sizeof(void*)`
- **Custom** — Any fixed-size data with appropriate hash and comparison functions

### Dynamic List

```c
void     dsc_list_init(dsc_list* list, size_t item_size, size_t initial_capacity);
void     dsc_list_destroy(dsc_list* list);
void     dsc_list_append(dsc_list* list, void* item);
void*    dsc_list_get(dsc_list* list, size_t index);
void     dsc_list_pop(dsc_list* list);
void     dsc_list_clear(dsc_list* list);
void     dsc_list_resize(dsc_list* list, size_t new_size);
void     dsc_list_map(dsc_list* list, dsc_callback cf);
void     dsc_list_foreach(dsc_list* list, dsc_callback cf);
dsc_list dsc_list_filter(dsc_list* list, dsc_predicate cf);
```

### Type-Safe Macros

```c
// Generate type-safe list wrapper
// Type: element type, name: prefix for generated functions
DSC_DEFINE_LIST(Type, name)
// Generates: name_list type and name_list_init(), name_list_append(), etc.

// Generate type-safe hash table wrapper  
// KeyType: key type, ValueType: value type, name: prefix for generated functions
DSC_DEFINE_HASH_TABLE(KeyType, ValueType, name)
// Generates: name_table type and name_table_init(), name_table_insert(), etc.
```

**Example:**
```c
// Integer list
DSC_DEFINE_LIST(int, int)
int_list nums = int_list_init(16);

// Hash table with int keys and string values
DSC_DEFINE_HASH_TABLE(int, char*, int_str)
int_str_table table = int_str_table_init(64, int_hash, int_cmp);
```

## Running Tests

DSC includes a comprehensive test suite:

```bash
cd tests

# Windows
run_tests.bat

# Linux/macOS
chmod +x run_tests.sh
./run_tests.sh
```

To use a specific compiler:

```bash
# Windows
run_tests.bat clang
run_tests.bat cl

# Linux/macOS
./run_tests.sh clang
```

## Contributing

Contributions are welcome! Please follow these guidelines:

### Code Style

- **Indentation:** 4 spaces (no tabs)
- **Braces:** K&R style (opening brace on same line)
- **Naming:** 
  - Functions: `snake_case` with `dsc_` prefix
  - Types: `snake_case` with `dsc_` prefix
  - Macros: `UPPER_SNAKE_CASE` with `DSC_` prefix
  - Local variables: `snake_case`
- **Comments:** Use `/* */` for multi-line, `//` for single-line
- **Line length:** Max 100 characters

### Example Style

```c
void dsc_example_function(dsc_list* list, size_t index) {
    if (list == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);

    // Single-line comment
    for (size_t i = 0; i < list->length; i++) {
        void* item = dsc_list_get(list, i);
        /* Multi-line comment
         * explaining complex logic */
        if (item != NULL) {
            process_item(item);
        }
    }
}
```

### Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

**Examples:**
```
feat(list): add list_insert_at function
fix(hash_table): fix memory leak in delete
docs(readme): add shared library build instructions
test(list): add edge case tests for empty list
```

### Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feat/my-feature`
3. **Make** your changes
4. **Add tests** for new functionality
5. **Run tests** to ensure everything passes
6. **Commit** with a descriptive message
7. **Push** to your fork
8. **Open** a Pull Request

### Adding New Data Structures

When adding a new data structure:

1. Add API declarations in the header section
2. Add implementation in the `#ifdef DSC_IMPLEMENTATION` section
3. Add comprehensive tests in `tests/test_<structure>.c`
4. Update this README with usage examples
5. Follow existing patterns for error handling

### Reporting Issues

When reporting bugs, please include:

- Operating system and version
- Compiler and version
- Minimal code example reproducing the issue
- Expected vs actual behavior
- Error messages (if any)

## License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.

```
Copyright (C) 2025 OmarElprolosy66

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
```

## Acknowledgments

- Inspired by [STB libraries](https://github.com/nothings/stb) by Sean Barrett

---

**Made by [OmarElprolosy66](https://github.com/OmarElprolosy66)**
