# Dynamic List (Array)

**Growable array with map, filter, and foreach operations**

## Quick Reference

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

---

## Basic Example

```c
#include "dsc.h"

int main(void) {
    // Initialize list for integers
    dsc_list numbers;
    dsc_list_init(&numbers, sizeof(int), 10);
    
    // Append items
    int a = 10, b = 20, c = 30;
    dsc_list_append(&numbers, &a);
    dsc_list_append(&numbers, &b);
    dsc_list_append(&numbers, &c);
    
    // Get item
    int* first = (int*)dsc_list_get(&numbers, 0);
    printf("%d\n", *first);  // 10
    
    // Pop last item
    dsc_list_pop(&numbers);
    
    // Cleanup
    dsc_list_destroy(&numbers);
}
```

---

## Type-Safe Wrapper

```c
// Define type-safe list
DSC_DEFINE_LIST(int, int)

int main(void) {
    // Initialize
    int_list nums;
    int_list_init(&nums, 10);
    
    // Append (pass by value!)
    int_list_append(&nums, 10);
    int_list_append(&nums, 20);
    int_list_append(&nums, 30);
    
    // Get (returns by value)
    int first = int_list_get(&nums, 0);
    printf("%d\n", first);  // 10
    
    // Pop
    int_list_pop(&nums);
    
    // Cleanup
    int_list_destroy(&nums);
}
```

---

## Structs in Lists

```c
typedef struct {
    char name[32];
    int age;
} Person;

int main(void) {
    dsc_list people;
    dsc_list_init(&people, sizeof(Person), 5);
    
    Person p1 = {"Alice", 25};
    Person p2 = {"Bob", 30};
    
    dsc_list_append(&people, &p1);
    dsc_list_append(&people, &p2);
    
    // Get person
    Person* person = (Person*)dsc_list_get(&people, 0);
    printf("%s is %d years old\n", person->name, person->age);
    
    dsc_list_destroy(&people);
}
```

---

## Map Operation

```c
void double_value(void* item) {
    int* num = (int*)item;
    *num *= 2;
}

int main(void) {
    dsc_list nums;
    dsc_list_init(&nums, sizeof(int), 5);
    
    int values[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        dsc_list_append(&nums, &values[i]);
    }
    
    // Double all values
    dsc_list_map(&nums, double_value);
    
    // Now: [2, 4, 6, 8, 10]
    for (size_t i = 0; i < nums.length; i++) {
        int* val = (int*)dsc_list_get(&nums, i);
        printf("%d ", *val);
    }
    printf("\n");
    
    dsc_list_destroy(&nums);
}
```

---

## Foreach Operation

```c
void print_int(void* item) {
    printf("%d ", *(int*)item);
}

int main(void) {
    dsc_list nums;
    dsc_list_init(&nums, sizeof(int), 5);
    
    int values[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        dsc_list_append(&nums, &values[i]);
    }
    
    // Print all
    dsc_list_foreach(&nums, print_int);  // 1 2 3 4 5
    printf("\n");
    
    dsc_list_destroy(&nums);
}
```

---

## Filter Operation

```c
int is_even(void* item) {
    return *(int*)item % 2 == 0;
}

int main(void) {
    dsc_list nums;
    dsc_list_init(&nums, sizeof(int), 10);
    
    // Add 1-10
    for (int i = 1; i <= 10; i++) {
        dsc_list_append(&nums, &i);
    }
    
    // Filter evens
    dsc_list evens = dsc_list_filter(&nums, is_even);
    
    // evens: [2, 4, 6, 8, 10]
    printf("Even numbers: ");
    for (size_t i = 0; i < evens.length; i++) {
        int* val = (int*)dsc_list_get(&evens, i);
        printf("%d ", *val);
    }
    printf("\n");
    
    dsc_list_destroy(&evens);
    dsc_list_destroy(&nums);
}
```

---

## Use Case: Dynamic String Array

```c
int main(void) {
    dsc_list lines;
    dsc_list_init(&lines, sizeof(char*), 10);
    
    // Add lines
    char* line1 = strdup("First line");
    char* line2 = strdup("Second line");
    char* line3 = strdup("Third line");
    
    dsc_list_append(&lines, &line1);
    dsc_list_append(&lines, &line2);
    dsc_list_append(&lines, &line3);
    
    // Print all lines
    for (size_t i = 0; i < lines.length; i++) {
        char** line_ptr = (char**)dsc_list_get(&lines, i);
        printf("%zu: %s\n", i, *line_ptr);
    }
    
    // Free strings
    for (size_t i = 0; i < lines.length; i++) {
        char** line_ptr = (char**)dsc_list_get(&lines, i);
        free(*line_ptr);
    }
    
    dsc_list_destroy(&lines);
}
```

---

## Use Case: Command Queue

```c
typedef enum {
    CMD_MOVE,
    CMD_ATTACK,
    CMD_DEFEND
} CommandType;

typedef struct {
    CommandType type;
    int target_x;
    int target_y;
} Command;

int main(void) {
    dsc_list cmd_queue;
    dsc_list_init(&cmd_queue, sizeof(Command), 20);
    
    // Queue commands
    Command move = {CMD_MOVE, 10, 20};
    Command attack = {CMD_ATTACK, 15, 25};
    
    dsc_list_append(&cmd_queue, &move);
    dsc_list_append(&cmd_queue, &attack);
    
    // Process commands
    while (cmd_queue.length > 0) {
        Command* cmd = (Command*)dsc_list_get(&cmd_queue, 0);
        printf("Executing command type %d at (%d, %d)\n", 
               cmd->type, cmd->target_x, cmd->target_y);
        
        // Remove processed command (shift array)
        // Note: For efficient queue, use a dedicated queue structure
        for (size_t i = 1; i < cmd_queue.length; i++) {
            Command* src = (Command*)dsc_list_get(&cmd_queue, i);
            Command* dst = (Command*)dsc_list_get(&cmd_queue, i-1);
            *dst = *src;
        }
        dsc_list_pop(&cmd_queue);
    }
    
    dsc_list_destroy(&cmd_queue);
}
```

---

## Use Case: Point Cloud / Vertices

```c
typedef struct {
    float x, y, z;
} Vec3;

int main(void) {
    dsc_list vertices;
    dsc_list_init(&vertices, sizeof(Vec3), 100);
    
    // Add vertices
    Vec3 v1 = {0.0f, 0.0f, 0.0f};
    Vec3 v2 = {1.0f, 0.0f, 0.0f};
    Vec3 v3 = {0.0f, 1.0f, 0.0f};
    
    dsc_list_append(&vertices, &v1);
    dsc_list_append(&vertices, &v2);
    dsc_list_append(&vertices, &v3);
    
    // Transform all vertices
    void translate(void* item) {
        Vec3* v = (Vec3*)item;
        v->x += 5.0f;
        v->y += 3.0f;
    }
    
    dsc_list_map(&vertices, translate);
    
    dsc_list_destroy(&vertices);
}
```

---

## Use Case: Log Buffer

```c
typedef struct {
    time_t timestamp;
    char message[256];
} LogEntry;

int main(void) {
    dsc_list logs;
    dsc_list_init(&logs, sizeof(LogEntry), 100);
    
    // Add log entries
    LogEntry entry;
    entry.timestamp = time(NULL);
    snprintf(entry.message, sizeof(entry.message), "Application started");
    dsc_list_append(&logs, &entry);
    
    entry.timestamp = time(NULL);
    snprintf(entry.message, sizeof(entry.message), "User logged in");
    dsc_list_append(&logs, &entry);
    
    // Print logs
    for (size_t i = 0; i < logs.length; i++) {
        LogEntry* log = (LogEntry*)dsc_list_get(&logs, i);
        printf("[%ld] %s\n", log->timestamp, log->message);
    }
    
    dsc_list_destroy(&logs);
}
```

---

## Use Case: Data Processing Pipeline

```c
typedef struct {
    int id;
    double value;
    bool processed;
} DataPoint;

void normalize(void* item) {
    DataPoint* dp = (DataPoint*)item;
    dp->value = dp->value / 100.0;  // Scale to 0-1
}

void mark_processed(void* item) {
    DataPoint* dp = (DataPoint*)item;
    dp->processed = true;
}

int is_valid(void* item) {
    DataPoint* dp = (DataPoint*)item;
    return dp->value > 0.5;  // Keep only values > 0.5
}

int main(void) {
    dsc_list data;
    dsc_list_init(&data, sizeof(DataPoint), 50);
    
    // Load data
    for (int i = 0; i < 10; i++) {
        DataPoint dp = {i, (double)(rand() % 100), false};
        dsc_list_append(&data, &dp);
    }
    
    // Processing pipeline
    dsc_list_map(&data, normalize);         // Step 1: Normalize
    dsc_list filtered = dsc_list_filter(&data, is_valid);  // Step 2: Filter
    dsc_list_map(&filtered, mark_processed); // Step 3: Mark as processed
    
    printf("Processed %zu out of %zu data points\n", 
           filtered.length, data.length);
    
    dsc_list_destroy(&filtered);
    dsc_list_destroy(&data);
}
```

---

## Resize and Clear

```c
dsc_list nums;
dsc_list_init(&nums, sizeof(int), 5);

// Add items
for (int i = 0; i < 10; i++) {
    dsc_list_append(&nums, &i);
}

// Resize (grow)
dsc_list_resize(&nums, 20);  // Capacity now 20, length still 10

// Clear all items (but keep capacity)
dsc_list_clear(&nums);  // length = 0, capacity unchanged

// Reuse
int x = 99;
dsc_list_append(&nums, &x);

dsc_list_destroy(&nums);
```

---

## Error Handling

```c
dsc_list nums;
dsc_list_init(&nums, sizeof(int), 5);

// Out of range access
int* val = (int*)dsc_list_get(&nums, 100);
if (val == NULL) {
    if (dsc_get_error() == DSC_ERANGE) {
        printf("Index out of range\n");
    }
}

// Pop empty list
dsc_list_pop(&nums);
if (dsc_get_error() == DSC_EEMPTY) {
    printf("Cannot pop from empty list\n");
}

dsc_list_destroy(&nums);
```

---

## Performance Tips

```c
// 1. Pre-allocate if you know the size
dsc_list big_list;
dsc_list_init(&big_list, sizeof(int), 10000);  // Avoid reallocations

// 2. Access length directly (no function call)
printf("List has %zu items\n", big_list.length);

// 3. Clear instead of destroy+init for reuse
dsc_list_clear(&big_list);  // Keeps capacity

// 4. Manual iteration is faster than foreach for simple cases
for (size_t i = 0; i < big_list.length; i++) {
    int* val = (int*)dsc_list_get(&big_list, i);
    // Process val
}

dsc_list_destroy(&big_list);
```
