# Stack - LIFO Data Structure

**Last-In-First-Out (LIFO) stack with O(1) push/pop/peek operations.**

---

## Quick Reference

```c
// Core Operations
void      dsc_stack_init(dsc_stack* stack, size_t item_size, size_t initial_capacity);
bool      dsc_stack_push(dsc_stack* stack, const void* item);
void*     dsc_stack_pop(dsc_stack* stack, void* out_item);
void*     dsc_stack_peek(dsc_stack* stack);
size_t    dsc_stack_size(dsc_stack* stack);
bool      dsc_stack_is_empty(dsc_stack* stack);
void      dsc_stack_clear(dsc_stack* stack);
void      dsc_stack_destroy(dsc_stack* stack);

// Type-Safe Wrapper
DSC_DEFINE_STACK(T, NAME)
```

---

## Basic Example

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>

int main(void) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 16);
    
    // Push elements
    int values[] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++) {
        dsc_stack_push(&stack, &values[i]);
    }
    
    // Pop elements (LIFO order: 4, 3, 2, 1)
    while (!dsc_stack_is_empty(&stack)) {
        int val;
        dsc_stack_pop(&stack, &val);
        printf("%d ", val);
    }
    printf("\n");  // Output: 4 3 2 1
    
    dsc_stack_destroy(&stack);
    return 0;
}
```

---

## Type-Safe Stack

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>

DSC_DEFINE_STACK(int, int)

int main(void) {
    int_stack stack;
    int_stack_init(&stack, 16);
    
    // Type-safe push
    int_stack_push(&stack, 10);
    int_stack_push(&stack, 20);
    int_stack_push(&stack, 30);
    
    // Peek at top without removing
    int* top = int_stack_peek(&stack);
    printf("Top: %d\n", *top);  // 30
    
    // Pop elements
    int val;
    while (int_stack_pop(&stack, &val)) {
        printf("%d ", val);
    }
    printf("\n");  // Output: 30 20 10
    
    int_stack_destroy(&stack);
    return 0;
}
```

---

## Stack with Structs

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    int id;
    char name[32];
    double score;
} Student;

DSC_DEFINE_STACK(Student, student)

int main(void) {
    student_stack stack;
    student_stack_init(&stack, 8);
    
    // Push students
    Student s1 = {1, "Alice", 95.5};
    Student s2 = {2, "Bob", 87.3};
    Student s3 = {3, "Charlie", 92.1};
    
    student_stack_push(&stack, s1);
    student_stack_push(&stack, s2);
    student_stack_push(&stack, s3);
    
    printf("Stack size: %zu\n", student_stack_size(&stack));  // 3
    
    // Process students in LIFO order
    Student student;
    while (student_stack_pop(&stack, &student)) {
        printf("ID: %d, Name: %s, Score: %.1f\n", 
               student.id, student.name, student.score);
    }
    // Output:
    // ID: 3, Name: Charlie, Score: 92.1
    // ID: 2, Name: Bob, Score: 87.3
    // ID: 1, Name: Alice, Score: 95.5
    
    student_stack_destroy(&stack);
    return 0;
}
```

---

## Use Case: Undo/Redo System

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    char action[64];
    char data[128];
} Command;

DSC_DEFINE_STACK(Command, command)

typedef struct {
    command_stack undo_stack;
    command_stack redo_stack;
} Editor;

void editor_init(Editor* editor) {
    command_stack_init(&editor->undo_stack, 16);
    command_stack_init(&editor->redo_stack, 16);
}

void editor_execute(Editor* editor, const char* action, const char* data) {
    Command cmd;
    strncpy(cmd.action, action, sizeof(cmd.action) - 1);
    strncpy(cmd.data, data, sizeof(cmd.data) - 1);
    
    command_stack_push(&editor->undo_stack, cmd);
    command_stack_clear(&editor->redo_stack);  // Clear redo on new action
    
    printf("Executed: %s\n", action);
}

void editor_undo(Editor* editor) {
    Command cmd;
    if (command_stack_pop(&editor->undo_stack, &cmd)) {
        command_stack_push(&editor->redo_stack, cmd);
        printf("Undid: %s\n", cmd.action);
    } else {
        printf("Nothing to undo\n");
    }
}

void editor_redo(Editor* editor) {
    Command cmd;
    if (command_stack_pop(&editor->redo_stack, &cmd)) {
        command_stack_push(&editor->undo_stack, cmd);
        printf("Redid: %s\n", cmd.action);
    } else {
        printf("Nothing to redo\n");
    }
}

void editor_destroy(Editor* editor) {
    command_stack_destroy(&editor->undo_stack);
    command_stack_destroy(&editor->redo_stack);
}

int main(void) {
    Editor editor;
    editor_init(&editor);
    
    // Execute commands
    editor_execute(&editor, "INSERT", "Hello");
    editor_execute(&editor, "INSERT", " World");
    editor_execute(&editor, "DELETE", "World");
    
    // Undo/Redo
    editor_undo(&editor);  // Undo DELETE
    editor_undo(&editor);  // Undo INSERT " World"
    editor_redo(&editor);  // Redo INSERT " World"
    
    editor_destroy(&editor);
    return 0;
}
```

---

## Use Case: Expression Evaluation

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

DSC_DEFINE_STACK(int, int)

int evaluate_rpn(const char* expression) {
    int_stack stack;
    int_stack_init(&stack, 16);
    
    const char* p = expression;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        
        if (isdigit(*p)) {
            int num = 0;
            while (isdigit(*p)) {
                num = num * 10 + (*p - '0');
                p++;
            }
            int_stack_push(&stack, num);
        } else {
            int b, a;
            int_stack_pop(&stack, &b);
            int_stack_pop(&stack, &a);
            
            int result;
            switch (*p) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': result = a / b; break;
                default: result = 0;
            }
            int_stack_push(&stack, result);
            p++;
        }
    }
    
    int result;
    int_stack_pop(&stack, &result);
    int_stack_destroy(&stack);
    
    return result;
}

int main(void) {
    // Reverse Polish Notation: 5 3 + 2 *
    // Equivalent to: (5 + 3) * 2 = 16
    int result = evaluate_rpn("5 3 + 2 *");
    printf("Result: %d\n", result);  // 16
    
    // Another example: 15 7 1 1 + - / 3 * 2 1 1 + + -
    // Equivalent to: ((15 / (7 - (1 + 1))) * 3) - (2 + (1 + 1)) = 5
    result = evaluate_rpn("15 7 1 1 + - / 3 * 2 1 1 + + -");
    printf("Result: %d\n", result);  // 5
    
    return 0;
}
```

---

## Use Case: Backtracking (Path Finding)

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int x, y;
} Point;

DSC_DEFINE_STACK(Point, point)

bool find_path(int maze[5][5], int start_x, int start_y, int end_x, int end_y) {
    point_stack path;
    point_stack_init(&path, 25);
    
    bool visited[5][5] = {false};
    int dx[] = {0, 1, 0, -1};  // Right, Down, Left, Up
    int dy[] = {1, 0, -1, 0};
    
    Point start = {start_x, start_y};
    point_stack_push(&path, start);
    visited[start_x][start_y] = true;
    
    while (!point_stack_is_empty(&path)) {
        Point* current = point_stack_peek(&path);
        
        // Check if we reached the destination
        if (current->x == end_x && current->y == end_y) {
            printf("Path found with %zu steps:\n", point_stack_size(&path));
            
            // Print path (would need to pop and store separately in real code)
            Point temp;
            point_stack_pop(&path, &temp);
            printf("(%d, %d)\n", temp.x, temp.y);
            
            point_stack_destroy(&path);
            return true;
        }
        
        // Try all 4 directions
        bool moved = false;
        for (int i = 0; i < 4; i++) {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];
            
            // Check bounds and if cell is valid
            if (nx >= 0 && nx < 5 && ny >= 0 && ny < 5 && 
                maze[nx][ny] == 0 && !visited[nx][ny]) {
                Point next = {nx, ny};
                point_stack_push(&path, next);
                visited[nx][ny] = true;
                moved = true;
                break;
            }
        }
        
        // Backtrack if no valid move
        if (!moved) {
            Point backtrack;
            point_stack_pop(&path, &backtrack);
        }
    }
    
    printf("No path found\n");
    point_stack_destroy(&path);
    return false;
}

int main(void) {
    // 0 = walkable, 1 = wall
    int maze[5][5] = {
        {0, 1, 0, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {1, 1, 0, 0, 0},
        {0, 0, 0, 1, 0}
    };
    
    find_path(maze, 0, 0, 4, 4);
    return 0;
}
```

---

## Use Case: Function Call Stack Simulator

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    char function_name[32];
    int line_number;
    char locals[64];
} StackFrame;

DSC_DEFINE_STACK(StackFrame, frame)

void print_stack_trace(frame_stack* callstack) {
    printf("\n=== Stack Trace ===\n");
    
    // Create temporary stack to preserve original
    frame_stack temp;
    frame_stack_init(&temp, frame_stack_size(callstack));
    
    StackFrame frame;
    while (frame_stack_pop(callstack, &frame)) {
        printf("  at %s (line %d): %s\n", 
               frame.function_name, frame.line_number, frame.locals);
        frame_stack_push(&temp, frame);
    }
    
    // Restore original stack
    while (frame_stack_pop(&temp, &frame)) {
        frame_stack_push(callstack, frame);
    }
    
    frame_stack_destroy(&temp);
    printf("==================\n\n");
}

void function_c(frame_stack* callstack) {
    StackFrame frame = {"function_c", 85, "result=42"};
    frame_stack_push(callstack, frame);
    
    printf("In function_c\n");
    print_stack_trace(callstack);
    
    StackFrame popped;
    frame_stack_pop(callstack, &popped);
}

void function_b(frame_stack* callstack) {
    StackFrame frame = {"function_b", 70, "x=10, y=20"};
    frame_stack_push(callstack, frame);
    
    printf("In function_b\n");
    function_c(callstack);
    
    StackFrame popped;
    frame_stack_pop(callstack, &popped);
}

void function_a(frame_stack* callstack) {
    StackFrame frame = {"function_a", 55, "count=5"};
    frame_stack_push(callstack, frame);
    
    printf("In function_a\n");
    function_b(callstack);
    
    StackFrame popped;
    frame_stack_pop(callstack, &popped);
}

int main(void) {
    frame_stack callstack;
    frame_stack_init(&callstack, 16);
    
    StackFrame main_frame = {"main", 10, "argc=1"};
    frame_stack_push(&callstack, main_frame);
    
    function_a(&callstack);
    
    frame_stack_destroy(&callstack);
    return 0;
}
```

---

## Performance Characteristics

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| Push      | O(1) amortized | O(n)            |
| Pop       | O(1)           | O(n)            |
| Peek      | O(1)           | O(n)            |
| Size      | O(1)           | O(n)            |
| IsEmpty   | O(1)           | O(n)            |
| Clear     | O(1)           | O(n)            |
| Destroy   | O(1)           | -               |

**Note:** Push is O(1) amortized due to automatic resizing when capacity is exceeded.

---

## Error Handling

```c
#define DSC_IMPLEMENTATION
#include "dsc.h"
#include <stdio.h>

int main(void) {
    dsc_stack stack;
    dsc_stack_init(&stack, sizeof(int), 4);
    
    // Pop from empty stack
    int val;
    if (dsc_stack_pop(&stack, &val) == NULL) {
        fprintf(stderr, "Error: %s\n", dsc_strerror(dsc_get_error()));
        // Output: Error: Container is empty
    }
    
    // Push with NULL
    if (!dsc_stack_push(&stack, NULL)) {
        fprintf(stderr, "Error: %s\n", dsc_strerror(dsc_get_error()));
        // Output: Error: Invalid argument
    }
    
    // Peek empty stack
    if (dsc_stack_peek(&stack) == NULL) {
        fprintf(stderr, "Error: %s\n", dsc_strerror(dsc_get_error()));
        // Output: Error: Container is empty
    }
    
    dsc_stack_destroy(&stack);
    return 0;
}
```

---

## Common Patterns

### Clear and Reuse

```c
dsc_stack stack;
dsc_stack_init(&stack, sizeof(int), 16);

// First usage
for (int i = 0; i < 10; i++) {
    dsc_stack_push(&stack, &i);
}

// Clear for reuse
dsc_stack_clear(&stack);

// Reuse the same stack
for (int i = 100; i < 110; i++) {
    dsc_stack_push(&stack, &i);
}

dsc_stack_destroy(&stack);
```

### Peek Before Pop

```c
// Check what's on top before deciding to pop
if (!dsc_stack_is_empty(&stack)) {
    int* top = (int*)dsc_stack_peek(&stack);
    
    if (*top > 50) {
        int val;
        dsc_stack_pop(&stack, &val);
        printf("Removed large value: %d\n", val);
    }
}
```

### Reversing Data

```c
// Push elements in order
int data[] = {1, 2, 3, 4, 5};
for (int i = 0; i < 5; i++) {
    dsc_stack_push(&stack, &data[i]);
}

// Pop gets them in reverse order
int reversed[5];
int idx = 0;
while (!dsc_stack_is_empty(&stack)) {
    dsc_stack_pop(&stack, &reversed[idx++]);
}
// reversed[] = {5, 4, 3, 2, 1}
```

---

## Best Practices

1. **Always check `dsc_stack_is_empty()` before pop/peek**
   ```c
   if (!dsc_stack_is_empty(&stack)) {
       dsc_stack_pop(&stack, &val);
   }
   ```

2. **Use type-safe wrappers for better type checking**
   ```c
   DSC_DEFINE_STACK(MyType, mytype)
   mytype_stack stack;  // Compile-time type safety
   ```

3. **Prefer peek for non-destructive access**
   ```c
   int* top = int_stack_peek(&stack);  // Don't modify stack
   ```

4. **Pre-allocate capacity for known sizes**
   ```c
   dsc_stack_init(&stack, sizeof(int), 1000);  // Avoid resizes
   ```

5. **Clear instead of destroy/init for reuse**
   ```c
   dsc_stack_clear(&stack);  // Keeps allocated memory
   ```

---

## Common Use Cases

- **Undo/Redo Systems** - Track operation history
- **Expression Evaluation** - Parse RPN, validate parentheses
- **Backtracking Algorithms** - DFS, maze solving, puzzle solving
- **Function Call Management** - Debug stack traces, recursion simulation
- **Syntax Parsing** - Compiler design, bracket matching
- **Browser History** - Back/forward navigation
- **Text Editors** - Cursor position history

---

## When NOT to Use Stack

- Random access needed → Use **dsc_list**
- FIFO order needed → Implement queue with dsc_list
- Key-value lookups → Use **dsc_hash_table**
- Unique elements → Use **dsc_set**
- Priority ordering → Implement heap/priority queue

---

## API Reference

### Initialization

```c
void dsc_stack_init(dsc_stack* stack, size_t item_size, size_t initial_capacity);
```
Initialize a stack. Pass `0` for `initial_capacity` to use default (256).

### Push

```c
bool dsc_stack_push(dsc_stack* stack, const void* item);
```
Push an item onto the stack. Returns `false` on error (NULL params, allocation failure).

### Pop

```c
void* dsc_stack_pop(dsc_stack* stack, void* out_item);
```
Pop top item into `out_item`. Returns `out_item` on success, `NULL` if empty or invalid params.

### Peek

```c
void* dsc_stack_peek(dsc_stack* stack);
```
View top item without removing. Returns `NULL` if empty or invalid params.

### Size

```c
size_t dsc_stack_size(dsc_stack* stack);
```
Get number of elements. Returns `0` for NULL stack.

### IsEmpty

```c
bool dsc_stack_is_empty(dsc_stack* stack);
```
Check if stack has no elements. Returns `true` for NULL stack (safe).

### Clear

```c
void dsc_stack_clear(dsc_stack* stack);
```
Remove all elements but keep allocated memory for reuse.

### Destroy

```c
void dsc_stack_destroy(dsc_stack* stack);
```
Free all resources. Stack must not be used after this without re-initialization.

---

## See Also

- [List](list.md) - Stack is built on top of dsc_list
- [Hash Table](hash_table.md) - For key-value storage
- [Set](set.md) - For unique element collections
- [Utilities](utilities.md) - Conversion functions
