# Set

**Hash table-based set with automatic duplicate prevention**

## Quick Reference

```c
void  dsc_set_init(dsc_set* set, size_t initial_capacity, size_t key_size, dsc_hashfunc* hf, dsc_cmpfunc* cf);
void  dsc_set_destroy(dsc_set* set);
bool  dsc_set_add(dsc_set* set, const void* item);
void  dsc_set_remove(dsc_set* set, const void* item);
void* dsc_set_get(dsc_set* set, const void* item);
void  dsc_set_clear(dsc_set* set);
```

---

## Basic Example - String Set

```c
#include "dsc.h"

// Use same hash/compare functions as hash table
int str_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return strcmp((const char*)k1, (const char*)k2);
}

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
    dsc_set tags;
    dsc_set_init(&tags, 16, 0, str_hash, str_cmp);
    
    // Add items
    dsc_set_add(&tags, "python");
    dsc_set_add(&tags, "c");
    dsc_set_add(&tags, "rust");
    
    // Try to add duplicate (returns false)
    bool added = dsc_set_add(&tags, "python");
    printf("Added duplicate: %s\n", added ? "yes" : "no");  // no
    
    // Check if item exists
    void* found = dsc_set_get(&tags, "c");
    if (found) {
        printf("Found: %s\n", (char*)found);  // "c"
    }
    
    // Remove item
    dsc_set_remove(&tags, "rust");
    
    // Cleanup
    dsc_set_destroy(&tags);
}
```

---

## Integer Set

```c
int int_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return *(int*)k1 - *(int*)k2;
}

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
    dsc_set numbers;
    dsc_set_init(&numbers, 16, sizeof(int), int_hash, int_cmp);
    
    // Add unique numbers
    int nums[] = {5, 2, 8, 2, 9, 5, 1};  // Has duplicates
    for (int i = 0; i < 7; i++) {
        dsc_set_add(&numbers, &nums[i]);
    }
    // Set now contains: {1, 2, 5, 8, 9}
    
    // Check membership
    int check = 5;
    if (dsc_set_get(&numbers, &check)) {
        printf("5 is in the set\n");
    }
    
    dsc_set_destroy(&numbers);
}
```

---

## Use Case: Unique Word Counter

```c
int main(void) {
    dsc_set unique_words;
    dsc_set_init(&unique_words, 32, 0, str_hash, str_cmp);
    
    const char* text = "the quick brown fox jumps over the lazy dog the fox";
    char* copy = strdup(text);
    
    // Tokenize and add to set
    char* token = strtok(copy, " ");
    while (token != NULL) {
        dsc_set_add(&unique_words, token);
        token = strtok(NULL, " ");
    }
    
    printf("Unique words: %zu\n", unique_words.ht->size);  // 8
    
    free(copy);
    dsc_set_destroy(&unique_words);
}
```

---

## Use Case: Visited Nodes (Graph Traversal)

```c
typedef struct {
    int id;
} Node;

int node_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return ((Node*)k1)->id - ((Node*)k2)->id;
}

uint64_t node_hash(const void* key, size_t len) {
    (void)len;
    return ((Node*)key)->id * 2654435761ULL;
}

void dfs(Node* node, dsc_set* visited) {
    // Check if already visited
    if (dsc_set_get(visited, node)) {
        return;
    }
    
    // Mark as visited
    dsc_set_add(visited, node);
    printf("Visiting node %d\n", node->id);
    
    // Visit neighbors...
}

int main(void) {
    dsc_set visited;
    dsc_set_init(&visited, 32, sizeof(Node), node_hash, node_cmp);
    
    Node n1 = {1}, n2 = {2}, n3 = {3};
    
    dfs(&n1, &visited);
    dfs(&n2, &visited);
    dfs(&n1, &visited);  // Already visited - skipped
    
    dsc_set_destroy(&visited);
}
```

---

## Use Case: Email Deduplication

```c
typedef struct {
    char email[128];
    char name[64];
} Contact;

// Only compare email for set uniqueness
int contact_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return strcmp(((Contact*)k1)->email, ((Contact*)k2)->email);
}

uint64_t contact_hash(const void* key, size_t len) {
    (void)len;
    const char* email = ((Contact*)key)->email;
    uint64_t hash = 5381;
    int c;
    while ((c = *email++)) {
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}

int main(void) {
    dsc_set contacts;
    dsc_set_init(&contacts, 32, sizeof(Contact), contact_hash, contact_cmp);
    
    Contact c1 = {.email = "alice@example.com", .name = "Alice"};
    Contact c2 = {.email = "bob@example.com", .name = "Bob"};
    Contact c3 = {.email = "alice@example.com", .name = "Alice Smith"};
    
    dsc_set_add(&contacts, &c1);
    dsc_set_add(&contacts, &c2);
    dsc_set_add(&contacts, &c3);  // Duplicate email - not added
    
    printf("Unique contacts: %zu\n", contacts.ht->size);  // 2
    
    dsc_set_destroy(&contacts);
}
```

---

## Use Case: IP Address Blacklist

```c
typedef struct {
    uint8_t octets[4];
} IP;

int ip_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return memcmp(k1, k2, sizeof(IP));
}

uint64_t ip_hash(const void* key, size_t len) {
    (void)len;
    const IP* ip = (const IP*)key;
    return ip->octets[0] + (ip->octets[1] << 8) + 
           (ip->octets[2] << 16) + (ip->octets[3] << 24);
}

int main(void) {
    dsc_set blacklist;
    dsc_set_init(&blacklist, 64, sizeof(IP), ip_hash, ip_cmp);
    
    // Add blocked IPs
    IP blocked1 = {{192, 168, 1, 100}};
    IP blocked2 = {{10, 0, 0, 50}};
    
    dsc_set_add(&blacklist, &blocked1);
    dsc_set_add(&blacklist, &blocked2);
    
    // Check if IP is blocked
    IP check = {{192, 168, 1, 100}};
    if (dsc_set_get(&blacklist, &check)) {
        printf("IP is blacklisted\n");
    }
    
    dsc_set_destroy(&blacklist);
}
```

---

## Use Case: Permission System

```c
typedef enum {
    PERM_READ = 1,
    PERM_WRITE = 2,
    PERM_EXECUTE = 4,
    PERM_DELETE = 8
} Permission;

int perm_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return *(Permission*)k1 - *(Permission*)k2;
}

uint64_t perm_hash(const void* key, size_t len) {
    (void)len;
    return *(Permission*)key * 2654435761ULL;
}

int main(void) {
    dsc_set user_perms;
    dsc_set_init(&user_perms, 8, sizeof(Permission), perm_hash, perm_cmp);
    
    // Grant permissions
    Permission read = PERM_READ;
    Permission write = PERM_WRITE;
    
    dsc_set_add(&user_perms, &read);
    dsc_set_add(&user_perms, &write);
    
    // Check permission
    Permission check = PERM_WRITE;
    if (dsc_set_get(&user_perms, &check)) {
        printf("Write permission granted\n");
    }
    
    // Revoke permission
    dsc_set_remove(&user_perms, &write);
    
    dsc_set_destroy(&user_perms);
}
```

---

## Use Case: Session Management

```c
typedef struct {
    char session_id[64];
    time_t created;
} Session;

int session_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
    (void)l1; (void)l2;
    return strcmp(((Session*)k1)->session_id, ((Session*)k2)->session_id);
}

uint64_t session_hash(const void* key, size_t len) {
    (void)len;
    const char* id = ((Session*)key)->session_id;
    uint64_t hash = 5381;
    int c;
    while ((c = *id++)) {
        hash = ((hash << 5) + hash) + (uint64_t)c;
    }
    return hash;
}

int main(void) {
    dsc_set active_sessions;
    dsc_set_init(&active_sessions, 64, sizeof(Session), session_hash, session_cmp);
    
    // Create session
    Session s1;
    snprintf(s1.session_id, sizeof(s1.session_id), "sess_%ld", time(NULL));
    s1.created = time(NULL);
    
    dsc_set_add(&active_sessions, &s1);
    
    // Validate session
    Session check = s1;
    Session* found = (Session*)dsc_set_get(&active_sessions, &check);
    if (found) {
        time_t age = time(NULL) - found->created;
        if (age > 3600) {  // 1 hour timeout
            printf("Session expired\n");
            dsc_set_remove(&active_sessions, &check);
        }
    }
    
    dsc_set_destroy(&active_sessions);
}
```

---

## Clear and Reuse

```c
dsc_set tags;
dsc_set_init(&tags, 16, 0, str_hash, str_cmp);

// Add items
dsc_set_add(&tags, "tag1");
dsc_set_add(&tags, "tag2");

// Clear all items
dsc_set_clear(&tags);  // size = 0, capacity unchanged

// Reuse
dsc_set_add(&tags, "new_tag");

dsc_set_destroy(&tags);
```

---

## Error Handling

```c
dsc_set nums;
dsc_set_init(&nums, 16, sizeof(int), int_hash, int_cmp);

int x = 42;
dsc_set_add(&nums, &x);

// Try to add duplicate
if (!dsc_set_add(&nums, &x)) {
    if (dsc_get_error() == DSC_EEXISTS) {
        printf("Item already in set\n");
    }
}

// Try to get non-existent item
int y = 99;
if (dsc_set_get(&nums, &y) == NULL) {
    if (dsc_get_error() == DSC_ENOTFOUND) {
        printf("Item not in set\n");
    }
}

dsc_set_destroy(&nums);
```

---

## Performance Tips

```c
// 1. Pre-allocate for known size
dsc_set big_set;
dsc_set_init(&big_set, 1024, sizeof(int), int_hash, int_cmp);

// 2. Access underlying hash table for size
printf("Set contains %zu items\n", big_set.ht->size);

// 3. Use clear() instead of destroy() + init() for reuse
dsc_set_clear(&big_set);

dsc_set_destroy(&big_set);
```

---

## Comparison with Hash Table

```c
// Set: Stores items as both key AND value (duplicate prevention)
dsc_set unique_ids;
dsc_set_init(&unique_ids, 16, sizeof(int), int_hash, int_cmp);
int id = 123;
dsc_set_add(&unique_ids, &id);  // Stores id as key and value

// Hash Table: Stores key -> value mapping
dsc_hash_table map;
dsc_hash_table_init(&map, 16, sizeof(int), int_hash, int_cmp);
int key = 123;
char* value = "some_data";
dsc_hash_table_insert(&map, &key, value);  // Stores key -> value

dsc_set_destroy(&unique_ids);
dsc_hash_table_destroy(&map, NULL);
```

---

## See Also

- [Hash Table](hash_table.md) - Set is built on hash table
- [List](list.md) - Ordered collection with duplicates allowed
- [Stack](stack.md) - LIFO data structure
- [Utilities](utilities.md) - Convert arrays/lists to sets, duplicate detection
