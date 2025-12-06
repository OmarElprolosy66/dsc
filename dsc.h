/****************************************************************************
 * dsc.h — Data Structure Collection
 * A lightweight, single-header C library for common data structures.
 * 
 * Version: 1.0.0
 * Repository: https://github.com/OmarElprolosy66/dsc
 * 
 ****************************************************************************
 * 
 * OVERVIEW
 * --------
 * DSC (Data Structure Collection) is an STB-style single-header library
 * providing efficient, type-safe implementations of essential data structures
 * for C projects. Designed for simplicity, portability, and ease of integration.
 * 
 * FEATURES
 * --------
 *   • Hash Table    — O(1) average insert/lookup/delete with automatic resizing with Generic keys (int, string, struct, pointer)
 *   • Dynamic List  — Growable array with map, filter, and foreach operations
 *   • Type-Safe     — Generic macros for compile-time type safety
 *   • Error System  — Thread-local errno-style error handling
 *   • Zero Dependencies — Only requires standard C library
 *   • Cross-Platform — Windows, Linux, macOS, BSD
 *   • Flexible Linking — Header-only, static library, or shared library
 * 
 * QUICK START
 * -----------
 * In ONE source file, define DSC_IMPLEMENTATION before including:
 * 
 *     #define DSC_IMPLEMENTATION
 *     #include "dsc.h"
 * 
 * In all other files, just include normally:
 * 
 *     #include "dsc.h"
 * 
 * SHARED LIBRARY BUILD
 * --------------------
 * Build:   cc -DDSC_SHARED -DDSC_BUILD -fPIC -shared dsc.c -o libdsc.so
 * Link:    cc -DDSC_SHARED main.c -L. -ldsc -o main
 * 
 * CONFIGURATION OPTIONS
 * ---------------------
 *   DSC_IMPLEMENTATION  — Include implementation (define in ONE .c file)
 *   DSC_SHARED          — Build/use as shared library (.dll/.so)
 *   DSC_BUILD           — Define when building shared library
 *   DSC_PREFIX          — Custom function prefix (default: dsc_)
 * 
 * EXAMPLE USAGE
 * -------------
 *     // Hash table with string keys
 *     int str_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
 *         return strcmp((const char*)k1, (const char*)k2);
 *     }
 *     uint64_t str_hash(const void* k, size_t len) {
 *         // djb2 hash...
 *     }
 *     dsc_hash_table* ht = dsc_hash_table_init(64, 0, str_hash, str_cmp);
 *     dsc_hash_table_insert(ht, "key", value_ptr);
 *     void* val = dsc_hash_table_get(ht, "key");
 *     dsc_hash_table_destroy(ht, NULL);
 * 
 *     // Hash table with integer keys
 *     int int_cmp(const void* k1, size_t l1, const void* k2, size_t l2) {
 *         return *(int*)k1 - *(int*)k2;
 *     }
 *     uint64_t int_hash(const void* k, size_t len) {
 *         // FNV-1a hash...
 *     }
 *     dsc_hash_table* ht = dsc_hash_table_init(64, sizeof(int), int_hash, int_cmp);
 *     int key = 42;
 *     dsc_hash_table_insert(ht, &key, value_ptr);
 * 
 *     // Dynamic list
 *     dsc_list list;
 *     dsc_list_init(&list, sizeof(int), 16);
 *     dsc_list_append(&list, &my_int);
 *     int* ptr = dsc_list_get(&list, 0);
 *     dsc_list_destroy(&list);
 * 
 *     // Type-safe wrappers
 *     DSC_DEFINE_LIST(int, int);
 *     int_list nums = int_list_init(16);
 *     int_list_append(&nums, 42);
 * 
 * ERROR HANDLING
 * --------------
 *     if (dsc_hash_table_insert(ht, key, val) == false) {
 *         dsc_error_t err = dsc_get_error();
 *         fprintf(stderr, "Error: %s\n", dsc_strerror(err));
 *     }
 * 
 ****************************************************************************
 * 
 * GNU LESSER GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 * 
 * Copyright (C) 2025 OmarElprolosy66
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 * 
 ****************************************************************************/

#ifndef DSC_H
#define DSC_H

/* ---------------------------------------------------------------
   Export macro (works for header-only AND shared-library builds)
   --------------------------------------------------------------- */

#ifndef DSC_API
#  if defined(DSC_SHARED)        /* building or using a shared lib */
#    if defined(_WIN32)
#      if defined(DSC_BUILD)
#        define DSC_API __declspec(dllexport)
#      else
#        define DSC_API __declspec(dllimport)
#      endif
#    else
#      define DSC_API __attribute__((visibility("default")))
#    endif
#  else
#    define DSC_API              /* static lib or header-only */
#  endif
#endif

#ifndef DSC_PREFIX
#define DSC_PREFIX dsc_
#endif

/* Two-level indirection needed for proper token concatenation */
#define DSC_CONCAT_(a, b) a##b
#define DSC_CONCAT(a, b) DSC_CONCAT_(a, b)
#define DSC_FUNC(name) DSC_CONCAT(DSC_PREFIX, name)

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------
   Error Handling System (using X-Macro pattern)
   --------------------------------------------------------------- */

/*
 * DSC_ERROR_LIST: Define all errors in one place.
 * Format: X(enum_name, error_message)
 * 
 * To add a new error, just add one line here — the enum and
 * string table are generated automatically!
 */
#define DSC_ERROR_LIST                                          \
    X(DSC_EOK,       "Success")                                 \
    X(DSC_ENOMEM,    "Memory allocation failed")                \
    X(DSC_EINVAL,    "Invalid argument")                        \
    X(DSC_ENOTFOUND, "Key or element not found")                \
    X(DSC_EEXISTS,   "Key already exists")                      \
    X(DSC_ERANGE,    "Index out of range")                      \
    X(DSC_EEMPTY,    "Container is empty")                      \
    X(DSC_EHASHFUNC, "Hash function is NULL or invalid")        \
    X(DSC_ECMPFUNC,  "Comparison function is NULL or invalid")

/* Generate the enum */
typedef enum {
    #define X(name, msg) name,
    DSC_ERROR_LIST
    #undef X
    DSC_ECOUNT  /* Number of error codes (for bounds checking) */
} dsc_error_t;

/* Thread-local error storage (C11+), falls back to global */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
    #define DSC_THREAD_LOCAL _Thread_local
#elif defined(_MSC_VER)
    #define DSC_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
    #define DSC_THREAD_LOCAL __thread
#else
    #define DSC_THREAD_LOCAL /* No thread-local support */
#endif

DSC_API dsc_error_t  DSC_FUNC(get_error)(void);
DSC_API const char*  DSC_FUNC(strerror)(dsc_error_t err);
DSC_API void         DSC_FUNC(clear_error)(void);

/* ---------------------------------------------------------------
   Public API declarations
   --------------------------------------------------------------- */

/*
 * +----------------------------------------------------------------+
 * |                         HASHTABLE API                          |
 * +----------------------------------------------------------------+
 */
#include <stdint.h>
#include <stdbool.h>

typedef struct _dsc_kvpair {
    void                 *obj;
    struct _dsc_kvpair   *next;
    void                 *key;
    size_t               key_size;
} dsc_kvpair;

typedef uint64_t dsc_hashfunc(const void*, size_t);
typedef int      dsc_cmpfunc(const void*, size_t, const void*, size_t);

typedef struct _dsc_hash_table {
    size_t          size;
    size_t          capacity;
    size_t          key_size;
    dsc_hashfunc    *hf;
    dsc_cmpfunc     *cf;
    dsc_kvpair      **kvpairs;
} dsc_hash_table;

typedef void dsc_cleanupfunc(void*);

DSC_API void   DSC_FUNC(hash_table_init)(dsc_hash_table *ht, size_t capacity, size_t key_size, dsc_hashfunc *hf, dsc_cmpfunc *cf);
DSC_API bool   DSC_FUNC(hash_table_insert)(dsc_hash_table *ht, const void *key, void *obj);
DSC_API void*  DSC_FUNC(hash_table_get)(dsc_hash_table *ht, const void *key);
DSC_API void*  DSC_FUNC(hash_table_delete)(dsc_hash_table *ht, const void *key);
DSC_API void   DSC_FUNC(hash_table_destroy)(dsc_hash_table *ht, dsc_cleanupfunc *cf);
DSC_API void   DSC_FUNC(hash_table_clear)(dsc_hash_table *ht, dsc_cleanupfunc *cf);

#define DSC_DEFINE_HASH_TABLE(K, T, NAME) \
    typedef struct { dsc_hash_table impl; } NAME##_table; \
    static inline void NAME##_table_init(NAME##_table *t, size_t s, dsc_hashfunc *hf, dsc_cmpfunc *cf) { \
        DSC_FUNC(hash_table_init)(&t->impl, s, sizeof(K), hf, cf); \
    } \
    static inline bool NAME##_table_insert(NAME##_table *t, K *k, T v) { \
        return DSC_FUNC(hash_table_insert)(&t->impl, (const void*)k, (void*)v); \
    } \
    static inline T NAME##_table_get(NAME##_table *t, K *k) { \
        return (T)DSC_FUNC(hash_table_get)(&t->impl, (const void*)k); \
    } \
    static inline T NAME##_table_delete(NAME##_table *t, K *k) { \
        return (T)DSC_FUNC(hash_table_delete)(&t->impl, (const void*)k); \
    } \
    static inline void NAME##_table_destroy(NAME##_table *t, dsc_cleanupfunc *cf) { \
        DSC_FUNC(hash_table_destroy)(&t->impl, cf); \
    }

/*
 * +----------------------------------------------------------------+
 * |                     LIST (DYNAMIC ARRAY) API                   |
 * +----------------------------------------------------------------+
 */

typedef struct _dsc_list {
    void*  items;
    size_t item_size;
    size_t length;
    size_t capacity;
} dsc_list;

typedef void (*dsc_callback)(void*);
typedef int  (*dsc_predicate)(void*);

DSC_API void     DSC_FUNC(list_init)(dsc_list* list, size_t item_size, size_t initial_capacity);
DSC_API void     DSC_FUNC(list_destroy)(dsc_list* list);
DSC_API void     DSC_FUNC(list_append)(dsc_list* list, void* item);
DSC_API void*    DSC_FUNC(list_get)(dsc_list* list, size_t index);
DSC_API void     DSC_FUNC(list_pop)(dsc_list* list);
DSC_API void     DSC_FUNC(list_clear)(dsc_list* list);
DSC_API void     DSC_FUNC(list_resize)(dsc_list* list, size_t new_size);
DSC_API void     DSC_FUNC(list_map)(dsc_list* list, dsc_callback cf);
DSC_API void     DSC_FUNC(list_foreach)(dsc_list* list, dsc_callback cf);
DSC_API dsc_list DSC_FUNC(list_filter)(dsc_list* list, dsc_predicate cf);

#define DSC_DEFINE_LIST(T, NAME) \
    typedef struct { dsc_list impl; } NAME##_list; \
    static inline void NAME##_list_init(NAME##_list *l, size_t initial_capacity) { \
        DSC_FUNC(list_init)(&l->impl, sizeof(T), initial_capacity); \
    } \
    static inline void NAME##_list_destroy(NAME##_list *l) { \
        DSC_FUNC(list_destroy)(&l->impl); \
    } \
    static inline void NAME##_list_append(NAME##_list *l, T item) { \
        DSC_FUNC(list_append)(&l->impl, (void*)&item); \
    } \
    static inline T NAME##_list_get(NAME##_list *l, size_t index) { \
        T* item = (T*)DSC_FUNC(list_get)(&l->impl, index); \
        return (item != NULL) ? *item : (T){0}; \
    } \
    static inline void NAME##_list_pop(NAME##_list *l) { \
        DSC_FUNC(list_pop)(&l->impl); \
    } \
    static inline void NAME##_list_clear(NAME##_list *l) { \
        DSC_FUNC(list_clear)(&l->impl); \
    } \
    static inline void NAME##_list_resize(NAME##_list *l, size_t new_size) { \
        DSC_FUNC(list_resize)(&l->impl, new_size); \
    } \
    static inline void NAME##_list_map(NAME##_list *l, dsc_callback cf) { \
        DSC_FUNC(list_map)(&l->impl, cf); \
    } \
    static inline void NAME##_list_foreach(NAME##_list *l, dsc_callback cf) { \
        DSC_FUNC(list_foreach)(&l->impl, cf); \
    } \
    static inline NAME##_list NAME##_list_filter(NAME##_list *l, dsc_predicate cf) { \
        NAME##_list result; \
        dsc_list filtered = DSC_FUNC(list_filter)(&l->impl, cf); \
        result.impl = filtered; \
        return result; \
    }

/*
 * +----------------------------------------------------------------+
 * |                             Set API                            |
 * +----------------------------------------------------------------+
 */
typedef struct _dsc_set {
    dsc_hash_table* ht;
} dsc_set;

void  DSC_FUNC(set_init)(dsc_set* set, size_t initial_capacity, size_t key_size, dsc_hashfunc* hf, dsc_cmpfunc* cf);
void  DSC_FUNC(set_destroy)(dsc_set* set);
bool  DSC_FUNC(set_add)(dsc_set* set, const void* item);
void  DSC_FUNC(set_remove)(dsc_set* set, const void* item);
void* DSC_FUNC(set_get)(dsc_set* set, const void* item);
void  DSC_FUNC(set_clear)(dsc_set* set);

#define DSC_DEFINE_SET(T, NAME) \
    typedef struct { dsc_set impl; } NAME##_set; \
    static inline void NAME##_set_init(NAME##_set *s, size_t initial_capacity, size_t key_size, dsc_hashfunc* hf, dsc_cmpfunc* cf) { \
        DSC_FUNC(set_init)(&s->impl, initial_capacity, key_size, hf, cf); \
    } \
    static inline void NAME##_set_destroy(NAME##_set *s) { \
        DSC_FUNC(set_destroy)(&s->impl); \
    } \
    static inline bool NAME##_set_add(NAME##_set *s, const void* item) { \
        return DSC_FUNC(set_add)(&s->impl, item); \
    } \
    static inline void NAME##_set_remove(NAME##_set *s, const void* item) { \
        DSC_FUNC(set_remove)(&s->impl, item); \
    } \
    static inline T NAME##_set_get(NAME##_set *s, const void* item) { \
        return (T)DSC_FUNC(set_get)(&s->impl, item); \
    } \
    static inline void NAME##_set_clear(NAME##_set *s) { \
        DSC_FUNC(set_clear)(&s->impl); \
    }

#ifdef __cplusplus
}
#endif

/* ---------------------------------------------------------------
   Implementation (included only when DSC_IMPLEMENTATION is defined)
   --------------------------------------------------------------- */
#ifdef DSC_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* ---------------------------------------------------------------
   Error Handling Implementation
   --------------------------------------------------------------- */

static DSC_THREAD_LOCAL dsc_error_t dsc_last_error = DSC_EOK;

static inline void dsc_set_error(dsc_error_t err) {
    dsc_last_error = err;
}

dsc_error_t DSC_FUNC(get_error)(void) {
    return dsc_last_error;
}

void DSC_FUNC(clear_error)(void) {
    dsc_last_error = DSC_EOK;
}

/* Generate the string table from X-macro */
const char* DSC_FUNC(strerror)(dsc_error_t err) {
    static const char* error_messages[] = {
        #define X(name, msg) msg,
        DSC_ERROR_LIST
        #undef X
    };
    if (err < 0 || err >= DSC_ECOUNT) {
        return "Unknown error";
    }
    return error_messages[err];
}

/*
 * +----------------------------------------------------------------+
 * |                   HASHTABLE Implementation                     |
 * +----------------------------------------------------------------+
 */
void DSC_FUNC(hash_table_init)(dsc_hash_table *ht, size_t capacity, size_t key_size, dsc_hashfunc *hf, dsc_cmpfunc *cf)
{
    dsc_set_error(DSC_EOK);

    if (ht == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }

    if (hf == NULL) {
        dsc_set_error(DSC_EHASHFUNC);
        return;
    }

    /* key_size can be 0 for variable-length keys (e.g., strings) */

    if (cf == NULL) {
        dsc_set_error(DSC_ECMPFUNC);
        return;
    }

    if (capacity == 0) capacity = 1;

    *ht = (dsc_hash_table) {
        .capacity = capacity,
        .hf       = hf,
        .cf       = cf,
        .key_size = key_size,
        .size     = 0,
        .kvpairs  = (dsc_kvpair **)calloc(capacity, sizeof(dsc_kvpair *))
    };
    if (ht->kvpairs == NULL) {
        dsc_set_error(DSC_ENOMEM);
        ht->capacity = 0;
        ht->size = 0;
        ht->kvpairs = NULL;
        return;
    }
}

bool DSC_FUNC(hash_table_insert)(dsc_hash_table *ht, const void *key, void *obj)
{
    dsc_set_error(DSC_EOK);

    if ((ht == NULL) || (key == NULL) || (obj == NULL)) {
        dsc_set_error(DSC_EINVAL);
        return false;
    }

    if ((float)ht->size / ht->capacity > 0.75) {
        size_t old_capacity  = ht->capacity;
        ht->capacity         = ht->capacity * 2;
        dsc_kvpair **new_kvpairs = (dsc_kvpair **)calloc(ht->capacity, sizeof(dsc_kvpair *));
        if (new_kvpairs == NULL) {
            dsc_set_error(DSC_ENOMEM);
            ht->capacity = old_capacity;
            return false;
        }

        // Rehash existing key-value pairs
        for (size_t i = 0; i < old_capacity; i++) {
            dsc_kvpair *tmp = ht->kvpairs[i];
            while (tmp != NULL) {
                dsc_kvpair *next = tmp->next;

                uint32_t new_index = ht->hf(tmp->key, tmp->key_size) % ht->capacity;
                assert(new_index < ht->capacity); // Ensure the index is within bounds

                tmp->next = new_kvpairs[new_index];
                new_kvpairs[new_index] = tmp;

                tmp = next;
            }
        }

        free(ht->kvpairs);
        ht->kvpairs = new_kvpairs;
    }

    if (DSC_FUNC(hash_table_get)(ht, key) != NULL) {
        dsc_set_error(DSC_EEXISTS);
        return false;
    }
    dsc_set_error(DSC_EOK);

    /* Calculate actual key size (for variable-length keys like strings) */
    size_t actual_key_size = ht->key_size;
    if (actual_key_size == 0) {
        /* Variable-length key - assume null-terminated string */
        actual_key_size = strlen((const char*)key) + 1;
    }

    uint32_t index = ht->hf(key, actual_key_size) % ht->capacity;

    dsc_kvpair *kvp = (dsc_kvpair *)malloc(sizeof(dsc_kvpair));
    if (kvp == NULL) {
        dsc_set_error(DSC_ENOMEM);
        return false;
    }

    *kvp = (dsc_kvpair) {
        .key      = malloc(actual_key_size),
        .obj      = obj,
        .key_size = actual_key_size
    };
    if (kvp->key == NULL) {
        dsc_set_error(DSC_ENOMEM);
        free(kvp);
        return false;
    }

    memcpy(kvp->key, key, actual_key_size);

    kvp->next = ht->kvpairs[index];
    ht->kvpairs[index] = kvp;
    ht->size++;

    return true;
}

/**
 * @brief deallocates the hash table
 * @param ht the hash table pointer
 * @param cf function pointer to a custom cleanup function for deallocating dynamically allocated objects defined by user.
 *  it should be NULL if the objects is statically allocated or the user want to handel the deallocation by himself.
 */
void DSC_FUNC(hash_table_destroy)(dsc_hash_table *ht, dsc_cleanupfunc *cf)
{
    if (ht == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);

    for (uint32_t i = 0; i < ht->capacity; i++) {
        while (ht->kvpairs[i] != NULL) {
            dsc_kvpair *tmp = ht->kvpairs[i];
            ht->kvpairs[i]  = ht->kvpairs[i]->next;

            free((void *)tmp->key);
            tmp->key = NULL;

            if (cf != NULL) cf(tmp->obj);

            free(tmp); tmp = NULL;
        }
    }

    free(ht->kvpairs); ht->kvpairs = NULL;
}

void *DSC_FUNC(hash_table_delete)(dsc_hash_table *ht, const void *key)
{
    dsc_set_error(DSC_EOK);

    if ((ht == NULL) || (key == NULL)) {
        dsc_set_error(DSC_EINVAL);
        return NULL;
    }

    /* Calculate key size for variable-length keys */
    size_t key_size = ht->key_size;
    if (key_size == 0) {
        key_size = strlen((const char*)key) + 1;
    }

    uint32_t index = ht->hf(key, key_size) % ht->capacity;

    dsc_kvpair *tmp  = ht->kvpairs[index];
    dsc_kvpair *prev = NULL;
    while (tmp != NULL && ht->cf(tmp->key, tmp->key_size, key, key_size) != 0) {
        prev = tmp;
        tmp  = tmp->next;
    }

    if (tmp  == NULL) {
        dsc_set_error(DSC_ENOTFOUND);
        return NULL;
    }
    if (prev == NULL) ht->kvpairs[index] = tmp->next;
    else prev->next = tmp->next;

    void *result = tmp->obj;
    free((void *)tmp->key);
    free(tmp); tmp = NULL;

    ht->size--;
    return result;
}

void *DSC_FUNC(hash_table_get)(dsc_hash_table *ht, const void *key)
{
    dsc_set_error(DSC_EOK);

    if ((ht == NULL) || (key == NULL)) {
        dsc_set_error(DSC_EINVAL);
        return NULL;
    }
    if (ht->hf == NULL) {
        dsc_set_error(DSC_EHASHFUNC);
        return NULL;
    }

    /* Calculate key size for variable-length keys */
    size_t key_size = ht->key_size;
    if (key_size == 0) {
        key_size = strlen((const char*)key) + 1;
    }

    uint32_t index = ht->hf(key, key_size) % ht->capacity;

    dsc_kvpair *tmp = ht->kvpairs[index];
    while (tmp != NULL && ht->cf(tmp->key, tmp->key_size, key, key_size) != 0) {
        tmp = tmp->next;
    }

    if (tmp == NULL) {
        dsc_set_error(DSC_ENOTFOUND);
        return NULL;
    }

    return tmp->obj;
}

void DSC_FUNC(hash_table_clear)(dsc_hash_table *ht, dsc_cleanupfunc *cf)
{
    if (ht == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);

    for (uint32_t i = 0; i < ht->capacity; i++) {
        while (ht->kvpairs[i] != NULL) {
            dsc_kvpair *tmp = ht->kvpairs[i];
            ht->kvpairs[i]  = ht->kvpairs[i]->next;

            free((void *)tmp->key);
            tmp->key = NULL;

            if (cf != NULL) cf(tmp->obj);

            free(tmp); tmp = NULL;
        }
    }
    ht->size = 0;
}

/*
 * +----------------------------------------------------------------+
 * |              LIST (DYNAMIC ARRAY) Implementation               |
 * +----------------------------------------------------------------+
 */
void DSC_FUNC(list_init)(dsc_list* list, size_t item_size, size_t initial_capacity) {
    dsc_set_error(DSC_EOK);

    if (list == NULL || item_size == 0) {
        dsc_set_error(DSC_EINVAL);
        return;
    }

    if (initial_capacity == 0) initial_capacity = 256;
    list->items = calloc(initial_capacity, item_size);
    if (list->items == NULL) {
        dsc_set_error(DSC_ENOMEM);
        list->item_size = 0;
        list->length    = 0;
        list->capacity  = 0;
        return;
    }
    list->item_size = item_size;
    list->length    = 0;
    list->capacity  = initial_capacity;
}

void DSC_FUNC(list_destroy)(dsc_list* list) {
    if (list == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);

    free(list->items);
    list->items = NULL;
    list->length = 0;
    list->capacity = 0;
}

void DSC_FUNC(list_append)(dsc_list* list, void* item) {
    dsc_set_error(DSC_EOK);

    if (list == NULL || item == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }

    if (list->length >= list->capacity) {
        size_t new_capacity = list->capacity * 2;
        void*  new_items    = realloc(list->items, new_capacity * list->item_size);
        if (new_items == NULL) {
            dsc_set_error(DSC_ENOMEM);
            return;
        }
        list->items    = new_items;
        list->capacity = new_capacity;
    }
    memcpy(
        (char*)list->items + (list->length * list->item_size),
        item,
        list->item_size
    );
    list->length++;
}

void* DSC_FUNC(list_get)(dsc_list* list, size_t index) {
    dsc_set_error(DSC_EOK);

    if (list == NULL) {
        dsc_set_error(DSC_EINVAL);
        return NULL;
    }
    if (index >= list->length) {
        dsc_set_error(DSC_ERANGE);
        return NULL;
    }
    return (char*)list->items + (index * list->item_size);
}

void DSC_FUNC(list_pop)(dsc_list* list) {
    dsc_set_error(DSC_EOK);

    if (list == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    if (list->length == 0) {
        dsc_set_error(DSC_EEMPTY);
        return;
    }
    list->length--;
}

void DSC_FUNC(list_clear)(dsc_list* list) {
    if (list == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);
    list->length = 0;
}

void DSC_FUNC(list_resize)(dsc_list* list, size_t new_size) {
    dsc_set_error(DSC_EOK);

    if (list == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }

    if (new_size > list->capacity) {
        void* new_items = realloc(list->items, new_size * list->item_size);
        if (new_items == NULL) {
            dsc_set_error(DSC_ENOMEM);
            return;
        }
        list->items    = new_items;
        list->capacity = new_size;
    }
    list->length = new_size;
}

void DSC_FUNC(list_map)(dsc_list* list, dsc_callback cf) {
    if (list == NULL || cf == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);

    for (size_t i = 0; i < list->length; i++) {
        void* item = (char*)list->items + (i * list->item_size);
        cf(item);
    }
}

void DSC_FUNC(list_foreach)(dsc_list* list, dsc_callback cf) {
    if (list == NULL || cf == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);

    for (size_t i = 0; i < list->length; i++) {
        void* item = (char*)list->items + (i * list->item_size);
        cf(item);
    }
}

dsc_list DSC_FUNC(list_filter)(dsc_list* list, dsc_predicate cf) {
    dsc_list result = {0};

    if (list == NULL || cf == NULL) {
        dsc_set_error(DSC_EINVAL);
        return result;
    }
    dsc_set_error(DSC_EOK);

    DSC_FUNC(list_init)(&result, list->item_size, list->capacity);
    if (DSC_FUNC(get_error)() != DSC_EOK) {
        return result;
    }

    for (size_t i = 0; i < list->length; i++) {
        void* item = (char*)list->items + (i * list->item_size);
        if (cf(item)) {
            DSC_FUNC(list_append)(&result, item);
        }
    }
    return result;
}

/*
 * +----------------------------------------------------------------+
 * |                     Set Implementation                         |
 * +----------------------------------------------------------------+
 */

 void DSC_FUNC(set_init)(dsc_set* set, size_t initial_capacity, size_t key_size, dsc_hashfunc* hf, dsc_cmpfunc* cf) {
    dsc_set_error(DSC_EOK);

    if (set == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }

    set->ht = (dsc_hash_table*)malloc(sizeof(dsc_hash_table));
    if (set->ht == NULL) {
        dsc_set_error(DSC_ENOMEM);
        return;
    }

    DSC_FUNC(hash_table_init)(set->ht, initial_capacity, key_size, hf, cf);
    if (DSC_FUNC(get_error)() != DSC_EOK) {
        free(set->ht);
        set->ht = NULL;
        return;
    }
}

void DSC_FUNC(set_destroy)(dsc_set* set) {
    if (set == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);

    if (set->ht != NULL) {
        DSC_FUNC(hash_table_destroy)(set->ht, NULL);
        free(set->ht);
        set->ht = NULL;
    }
}

bool DSC_FUNC(set_add)(dsc_set* set, const void* item) {
    dsc_set_error(DSC_EOK);

    if (set == NULL || item == NULL) {
        dsc_set_error(DSC_EINVAL);
        return false;
    }

    bool res = DSC_FUNC(hash_table_insert)(set->ht, item, (void*)item);
    if (!res) {
        // Error already set by hash_table_insert
        return false;
    }
    return true;
}

void DSC_FUNC(set_remove)(dsc_set* set, const void* item) {
    dsc_set_error(DSC_EOK);

    if (set == NULL || item == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }

    void* removed = DSC_FUNC(hash_table_delete)(set->ht, item);
    if (removed == NULL) {
        // Error already set by hash_table_delete
        return;
    }
}

void* DSC_FUNC(set_get)(dsc_set* set, const void* item) {
    dsc_set_error(DSC_EOK);

    if (set == NULL || item == NULL) {
        dsc_set_error(DSC_EINVAL);
        return NULL;
    }

    void* found = DSC_FUNC(hash_table_get)(set->ht, item);
    if (found == NULL) {
        // Error already set by hash_table_get
        return NULL;
    }
    return found;
}

void DSC_FUNC(set_clear)(dsc_set* set) {
    if (set == NULL) {
        dsc_set_error(DSC_EINVAL);
        return;
    }
    dsc_set_error(DSC_EOK);

    DSC_FUNC(hash_table_clear)(set->ht, NULL);
}

#endif /* DSC_IMPLEMENTATION */

#endif /* DSC_H */

