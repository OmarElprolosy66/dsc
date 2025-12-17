# Hash Table

**O(1) average insert/lookup/delete with automatic resizing and generic key support**

## Quick Reference

```c
void   dsc_hash_table_init(dsc_hash_table *ht, size_t capacity, size_t key_size, dsc_hashfunc *hf, dsc_cmpfunc *cf);
bool   dsc_hash_table_insert(dsc_hash_table *ht, const void *key, void *value);
void*  dsc_hash_table_get(dsc_hash_table *ht, const void *key);
void*  dsc_hash_table_delete(dsc_hash_table *ht, const void *key);
void   dsc_hash_table_destroy(dsc_hash_table *ht, dsc_cleanupfunc *cf);
void   dsc_hash_table_clear(dsc_hash_table *ht, dsc_cleanupfunc *cf);
```

---

## Basic Example - String Keys

```c
#include "dsc.h"

// Comparison function for strings
int str_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return strcmp((const char*)k1, (const char*)k2);
}

// Hash function for strings (djb2)
uint64_t str_hash(const void* key, size_t len) {
    const char* str = (const char*)key;
    uint64_t hash = 5381;
    (void)len;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}

int main(void) {
    // Initialize hash table with string keys (key_size=0 for variable-length)
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, 0, str_hash, str_cmp);
    
    // Insert
    int age = 25;
    dsc_hash_table_insert(&ht, "alice", &age);
    
    // Get
    int* result = (int*)dsc_hash_table_get(&ht, "alice");
    printf("Alice's age: %d\n", *result);  // 25
    
    // Delete
    int* deleted = (int*)dsc_hash_table_delete(&ht, "alice");
    
    // Cleanup
    dsc_hash_table_destroy(&ht, NULL);
}
```

---

## Integer Keys

```c
// Comparison function for integers
int int_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return *(int*)k1 - *(int*)k2;
}

// Hash function for integers (FNV-1a)
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

int main(void) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, sizeof(int), int_hash, int_cmp);
    
    int key = 42;
    char* name = "answer";
    dsc_hash_table_insert(&ht, &key, name);
    
    char* value = (char*)dsc_hash_table_get(&ht, &key);
    printf("%s\n", value);  // "answer"
    
    dsc_hash_table_destroy(&ht, NULL);
}
```

---

## Struct Keys

```c
typedef struct {
    int id;
    char name[32];
} User;

int user_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    const User* u1 = (const User*)k1;
    const User* u2 = (const User*)k2;
    
    int id_diff = u1->id - u2->id;
    if (id_diff != 0) return id_diff;
    
    return strcmp(u1->name, u2->name);
}

uint64_t user_hash(const void* key, size_t len) {
    (void)len;
    const User* u = (const User*)key;
    uint64_t hash = 5381;
    
    // Hash id
    hash = ((hash << 5) + hash) + u->id;
    
    // Hash name
    const char* str = u->name;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}

int main(void) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, sizeof(User), user_hash, user_cmp);
    
    User key = {101, "john"};
    int score = 95;
    dsc_hash_table_insert(&ht, &key, &score);
    
    int* result = (int*)dsc_hash_table_get(&ht, &key);
    printf("Score: %d\n", *result);  // 95
    
    dsc_hash_table_destroy(&ht, NULL);
}
```

---

## Pointer Keys

```c
// Comparison for pointer keys
int ptr_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    void* p1 = *(void**)k1;
    void* p2 = *(void**)k2;
    return (p1 > p2) - (p1 < p2);
}

// Hash for pointer keys (Knuth multiplicative)
uint64_t ptr_hash(const void* key, size_t len) {
    (void)len;
    uintptr_t ptr = (uintptr_t)(*(void**)key);
    return ptr * 2654435761ULL;
}

int main(void) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, sizeof(void*), ptr_hash, ptr_cmp);
    
    int obj = 42;
    void* key = &obj;
    char* metadata = "some_data";
    
    dsc_hash_table_insert(&ht, &key, metadata);
    
    char* result = (char*)dsc_hash_table_get(&ht, &key);
    printf("%s\n", result);  // "some_data"
    
    dsc_hash_table_destroy(&ht, NULL);
}
```

---

## Memory Management - Heap Allocated Values

```c
typedef struct {
    char* name;
    int age;
} Person;

void free_person(void* obj) {
    Person* p = (Person*)obj;
    free(p->name);
    free(p);
}

int main(void) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, 0, str_hash, str_cmp);
    
    // Insert heap-allocated values
    Person* alice = malloc(sizeof(Person));
    alice->name = strdup("Alice");
    alice->age = 25;
    dsc_hash_table_insert(&ht, "alice", alice);
    
    Person* bob = malloc(sizeof(Person));
    bob->name = strdup("Bob");
    bob->age = 30;
    dsc_hash_table_insert(&ht, "bob", bob);
    
    // Destroy with cleanup function
    dsc_hash_table_destroy(&ht, free_person);
}
```

---

## Use Case: Configuration System

```c
// Store application settings
dsc_hash_table config;
dsc_hash_table_init(&config, 32, 0, str_hash, str_cmp);

// Add settings
char* theme = "dark";
int timeout = 30;
bool verbose = true;

dsc_hash_table_insert(&config, "theme", theme);
dsc_hash_table_insert(&config, "timeout", &timeout);
dsc_hash_table_insert(&config, "verbose", &verbose);

// Retrieve settings
char* current_theme = (char*)dsc_hash_table_get(&config, "theme");
int* current_timeout = (int*)dsc_hash_table_get(&config, "timeout");

// Update setting
dsc_hash_table_delete(&config, "timeout");
int new_timeout = 60;
dsc_hash_table_insert(&config, "timeout", &new_timeout);

dsc_hash_table_destroy(&config, NULL);
```

---

## Use Case: Caching

```c
typedef struct {
    char* data;
    time_t timestamp;
} CacheEntry;

void free_cache_entry(void* obj) {
    CacheEntry* entry = (CacheEntry*)obj;
    free(entry->data);
    free(entry);
}

int main(void) {
    dsc_hash_table cache;
    dsc_hash_table_init(&cache, 128, 0, str_hash, str_cmp);
    
    // Add to cache
    CacheEntry* entry = malloc(sizeof(CacheEntry));
    entry->data = strdup("expensive_computation_result");
    entry->timestamp = time(NULL);
    dsc_hash_table_insert(&cache, "user:123:profile", entry);
    
    // Check cache
    CacheEntry* cached = (CacheEntry*)dsc_hash_table_get(&cache, "user:123:profile");
    if (cached) {
        time_t age = time(NULL) - cached->timestamp;
        if (age < 300) {  // 5 minute TTL
            printf("Cache hit: %s\n", cached->data);
        } else {
            // Expired - delete and recompute
            dsc_hash_table_delete(&cache, "user:123:profile");
            free_cache_entry(cached);
        }
    }
    
    dsc_hash_table_destroy(&cache, free_cache_entry);
}
```

---

## Use Case: Symbol Table (Compiler/Interpreter)

```c
typedef struct {
    char* type;
    int scope_level;
    void* value;
} Symbol;

void free_symbol(void* obj) {
    Symbol* sym = (Symbol*)obj;
    free(sym->type);
    free(sym);
}

int main(void) {
    dsc_hash_table symbols;
    dsc_hash_table_init(&symbols, 64, 0, str_hash, str_cmp);
    
    // Define variables
    Symbol* x = malloc(sizeof(Symbol));
    x->type = strdup("int");
    x->scope_level = 0;
    x->value = malloc(sizeof(int));
    *(int*)x->value = 42;
    dsc_hash_table_insert(&symbols, "x", x);
    
    // Lookup variable
    Symbol* var = (Symbol*)dsc_hash_table_get(&symbols, "x");
    if (var) {
        printf("Variable 'x' has type '%s' and value %d\n", 
               var->type, *(int*)var->value);
    }
    
    dsc_hash_table_destroy(&symbols, free_symbol);
}
```

---

## Use Case: Graph Adjacency List

```c
typedef struct {
    int* neighbors;
    size_t count;
    size_t capacity;
} AdjList;

void free_adjlist(void* obj) {
    AdjList* list = (AdjList*)obj;
    free(list->neighbors);
    free(list);
}

int main(void) {
    dsc_hash_table graph;
    dsc_hash_table_init(&graph, 32, sizeof(int), int_hash, int_cmp);
    
    // Add edges: 1 -> 2, 1 -> 3
    int node1 = 1;
    AdjList* adj1 = malloc(sizeof(AdjList));
    adj1->capacity = 4;
    adj1->count = 2;
    adj1->neighbors = malloc(sizeof(int) * adj1->capacity);
    adj1->neighbors[0] = 2;
    adj1->neighbors[1] = 3;
    dsc_hash_table_insert(&graph, &node1, adj1);
    
    // Get neighbors of node 1
    AdjList* neighbors = (AdjList*)dsc_hash_table_get(&graph, &node1);
    printf("Node 1 neighbors: ");
    for (size_t i = 0; i < neighbors->count; i++) {
        printf("%d ", neighbors->neighbors[i]);
    }
    printf("\n");
    
    dsc_hash_table_destroy(&graph, free_adjlist);
}
```

---

## Error Handling

```c
dsc_hash_table ht;
dsc_hash_table_init(&ht, 16, 0, str_hash, str_cmp);

int value = 100;
bool success = dsc_hash_table_insert(&ht, "key", &value);
if (!success) {
    dsc_error_t err = dsc_get_error();
    fprintf(stderr, "Insert failed: %s\n", dsc_strerror(err));
}

// Try to get non-existent key
void* result = dsc_hash_table_get(&ht, "nonexistent");
if (result == NULL) {
    if (dsc_get_error() == DSC_ENOTFOUND) {
        printf("Key not found\n");
    }
}

// Try duplicate insert
if (!dsc_hash_table_insert(&ht, "key", &value)) {
    if (dsc_get_error() == DSC_EEXISTS) {
        printf("Key already exists\n");
    }
}

dsc_hash_table_destroy(&ht, NULL);
```

---

## Advanced: Custom Hash Functions

```c
// Case-insensitive string hash
uint64_t case_insensitive_hash(const void* key, size_t len) {
    const char* str = (const char*)key;
    uint64_t hash = 5381;
    (void)len;
    int c;
    while ((c = *str++)) {
        c = tolower(c);
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}

int case_insensitive_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return strcasecmp((const char*)k1, (const char*)k2);
}

int main(void) {
    dsc_hash_table ht;
    dsc_hash_table_init(&ht, 16, 0, case_insensitive_hash, case_insensitive_cmp);
    
    int v = 42;
    dsc_hash_table_insert(&ht, "Hello", &v);
    
    // Works with any case
    int* result = (int*)dsc_hash_table_get(&ht, "HELLO");  // Found!
    
    dsc_hash_table_destroy(&ht, NULL);
}
```

---

## Performance Tips

```c
// 1. Choose appropriate initial capacity
dsc_hash_table small_ht;
dsc_hash_table_init(&small_ht, 16, 0, str_hash, str_cmp);   // For <12 items

dsc_hash_table large_ht;
dsc_hash_table_init(&large_ht, 1024, 0, str_hash, str_cmp); // For ~768 items

// 2. Use fixed-size keys when possible (faster than variable-length)
dsc_hash_table ht;
dsc_hash_table_init(&ht, 32, sizeof(int), int_hash, int_cmp);

// 3. Reuse hash tables with clear()
dsc_hash_table_clear(&ht, NULL);  // Remove all items but keep capacity

dsc_hash_table_destroy(&ht, NULL);
```

---

## See Also

- [List](list.md) - Growable array for ordered data
- [Set](set.md) - Hash-based set built on hash table
- [Stack](stack.md) - LIFO data structure
- [Utilities](utilities.md) - Extract keys/values, conversions
