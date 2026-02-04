# First-Pass Memory Manager Implementation

## Implementation Status: ✅ COMPLETE

The mark-and-sweep garbage collector has been successfully implemented and tested.

**Changes made:**
- Added GC root registry in `lisp.c` (`gc_add_root`, `gc_remove_root`, `gc_add_env_root`, `gc_remove_env_root`)
- Implemented mark phase (`gc_mark_object`, `gc_mark_env`, `gc_mark_binding`)
- Implemented sweep phase (`gc_sweep`)
- Added `gc_collect()` entry point with statistics
- Modified `lisp_alloc()` to trigger GC at 75% capacity threshold
- Protected temporaries in `eval.c` during function application
- Registered global environment as root in `main.c`

**Test Results:**
- GC successfully collects garbage objects
- Live objects survive collection
- Iterative tests with 200K+ allocations pass without out-of-memory errors

---

## Overview

This document describes how to implement a simple mark-and-sweep garbage collector for the LispOS Scheme interpreter. This is a "first-pass" implementation focused on correctness over performance.

## Current State

The current memory system in [lisp.c](../src/lisp.c) uses a fixed object pool:

```c
#define MAX_OBJECTS 262144
static LispObject *all_objects[MAX_OBJECTS];
static int num_objects = 0;
```

**Problems:**
1. Objects are never freed during execution
2. Fixed limit causes "out of memory" errors
3. Long-running programs eventually crash

## Design Goals

1. **Correctness first** - Never free reachable objects
2. **Simplicity** - Easy to understand and debug
3. **Incremental** - Minimal changes to existing code
4. **Non-moving** - Objects stay at same address (simplifies implementation)

## Architecture

### Mark-and-Sweep Algorithm

```
┌─────────────────────────────────────────────────────────────────┐
│                    GARBAGE COLLECTION CYCLE                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. MARK PHASE                     2. SWEEP PHASE               │
│  ┌──────────────────┐              ┌──────────────────┐         │
│  │ Start from roots │              │ Scan all objects │         │
│  │    ↓             │              │    ↓             │         │
│  │ Mark reachable   │     →        │ Free unmarked    │         │
│  │ objects          │              │ objects          │         │
│  │    ↓             │              │    ↓             │         │
│  │ Follow pointers  │              │ Compact pool     │         │
│  └──────────────────┘              └──────────────────┘         │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

## Implementation Steps

### Step 1: Add Mark Bit to Objects

Modify `lisp.h`:

```c
typedef struct LispObject {
    LispType type;
    unsigned char marked;    // ADD THIS FIELD
    union {
        // ... existing fields
    };
} LispObject;
```

### Step 2: Create GC Root Registry

The GC needs to know where to start marking. Roots include:
- Global environment
- Current evaluation stack
- Symbol table

Add to `lisp.c`:

```c
/* GC Root Registry */
#define MAX_GC_ROOTS 1024
static LispObject **gc_roots[MAX_GC_ROOTS];
static int num_gc_roots = 0;

/* Register a root pointer */
void gc_add_root(LispObject **root) {
    if (num_gc_roots < MAX_GC_ROOTS) {
        gc_roots[num_gc_roots++] = root;
    }
}

/* Remove a root pointer */
void gc_remove_root(LispObject **root) {
    for (int i = 0; i < num_gc_roots; i++) {
        if (gc_roots[i] == root) {
            gc_roots[i] = gc_roots[--num_gc_roots];
            return;
        }
    }
}
```

### Step 3: Implement Mark Phase

```c
/* Mark a single object and its children */
static void gc_mark_object(LispObject *obj) {
    if (obj == NULL || obj->marked) {
        return;  // Already marked or null
    }

    obj->marked = 1;

    // Recursively mark children based on type
    switch (obj->type) {
        case LISP_CONS:
            gc_mark_object(obj->cons.car);
            gc_mark_object(obj->cons.cdr);
            break;

        case LISP_LAMBDA:
            gc_mark_object(obj->lambda.params);
            gc_mark_object(obj->lambda.body);
            gc_mark_env(obj->lambda.env);  // Mark environment
            break;

        case LISP_MACRO:
            gc_mark_object(obj->macro.params);
            gc_mark_object(obj->macro.body);
            gc_mark_env(obj->macro.env);
            break;

        case LISP_VECTOR:
            for (size_t i = 0; i < obj->vector.length; i++) {
                gc_mark_object(obj->vector.elements[i]);
            }
            break;

        case LISP_HASHTABLE:
            for (size_t i = 0; i < obj->hashtable.capacity; i++) {
                if (obj->hashtable.keys[i]) {
                    gc_mark_object(obj->hashtable.keys[i]);
                    gc_mark_object(obj->hashtable.values[i]);
                }
            }
            break;

        case LISP_RECORD:
            gc_mark_object(obj->record.rtd);
            // Mark all fields
            if (obj->record.rtd && is_record_type(obj->record.rtd)) {
                int count = obj->record.rtd->record_type.field_count;
                for (int i = 0; i < count; i++) {
                    gc_mark_object(obj->record.fields[i]);
                }
            }
            break;

        case LISP_RECORD_TYPE:
            gc_mark_object(obj->record_type.name);
            gc_mark_object(obj->record_type.parent);
            gc_mark_object(obj->record_type.fields);
            break;

        case LISP_CONDITION:
            gc_mark_object(obj->condition.type);
            gc_mark_object(obj->condition.message);
            gc_mark_object(obj->condition.irritants);
            gc_mark_object(obj->condition.who);
            break;

        case LISP_VALUES:
            for (int i = 0; i < obj->values.count; i++) {
                gc_mark_object(obj->values.vals[i]);
            }
            break;

        // Atomic types - no children to mark
        case LISP_NIL:
        case LISP_BOOLEAN:
        case LISP_NUMBER:
        case LISP_CHARACTER:
        case LISP_STRING:
        case LISP_SYMBOL:
        case LISP_PRIMITIVE:
        case LISP_PORT:
            break;
    }
}

/* Mark environment frames */
static void gc_mark_env(Environment *env) {
    while (env != NULL) {
        gc_mark_object(env->bindings);  // Mark binding list
        env = env->parent;
    }
}

/* Mark all roots */
static void gc_mark_roots(void) {
    // Mark registered roots
    for (int i = 0; i < num_gc_roots; i++) {
        if (gc_roots[i] && *gc_roots[i]) {
            gc_mark_object(*gc_roots[i]);
        }
    }

    // Mark symbol table (symbols are permanent)
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        if (symbol_table[i]) {
            gc_mark_object(symbol_table[i]);
        }
    }

    // Mark global singletons
    gc_mark_object(LISP_NIL_OBJ);
    gc_mark_object(LISP_TRUE);
    gc_mark_object(LISP_FALSE);
}
```

### Step 4: Implement Sweep Phase

```c
/* Free an unmarked object */
static void gc_free_object(LispObject *obj) {
    switch (obj->type) {
        case LISP_STRING:
            free(obj->string.data);
            break;
        case LISP_SYMBOL:
            // Don't free symbol names - they're interned
            break;
        case LISP_LAMBDA:
            if (obj->lambda.name) free(obj->lambda.name);
            break;
        case LISP_VECTOR:
            free(obj->vector.elements);
            break;
        case LISP_BYTEVECTOR:
            free(obj->bytevector.bytes);
            break;
        case LISP_HASHTABLE:
            free(obj->hashtable.keys);
            free(obj->hashtable.values);
            break;
        case LISP_RECORD:
            free(obj->record.fields);
            break;
        case LISP_VALUES:
            free(obj->values.vals);
            break;
        case LISP_PORT:
            if (obj->port.name) free(obj->port.name);
            break;
        default:
            break;
    }
    free(obj);
}

/* Sweep phase - free unmarked objects */
static void gc_sweep(void) {
    int new_count = 0;

    for (int i = 0; i < num_objects; i++) {
        LispObject *obj = all_objects[i];

        if (obj->marked) {
            // Object is reachable - keep it
            obj->marked = 0;  // Reset for next GC cycle
            all_objects[new_count++] = obj;
        } else {
            // Object is garbage - free it
            gc_free_object(obj);
        }
    }

    num_objects = new_count;
}
```

### Step 5: Main GC Entry Point

```c
/* Statistics */
static int gc_collections = 0;
static int gc_objects_freed = 0;

/* Run garbage collection */
void gc_collect(void) {
    int before = num_objects;

    // Mark phase
    gc_mark_roots();

    // Sweep phase
    gc_sweep();

    // Statistics
    gc_collections++;
    gc_objects_freed += (before - num_objects);

    #ifdef GC_DEBUG
    printf("[GC] Collection #%d: %d -> %d objects (%d freed)\n",
           gc_collections, before, num_objects, before - num_objects);
    #endif
}

/* Get GC statistics */
void gc_stats(int *collections, int *freed, int *current) {
    if (collections) *collections = gc_collections;
    if (freed) *freed = gc_objects_freed;
    if (current) *current = num_objects;
}
```

### Step 6: Trigger GC in Allocator

Modify `lisp_alloc()`:

```c
#define GC_THRESHOLD 0.75  // Trigger GC at 75% capacity

LispObject *lisp_alloc(void) {
    // Check if GC needed
    if (num_objects >= MAX_OBJECTS * GC_THRESHOLD) {
        gc_collect();
    }

    // Check if still out of memory after GC
    if (num_objects >= MAX_OBJECTS) {
        lisp_error("Out of memory: %d objects allocated", num_objects);
        return NULL;
    }

    // Allocate new object
    LispObject *obj = (LispObject *)calloc(1, sizeof(LispObject));
    if (!obj) {
        lisp_error("Out of memory");
        return NULL;
    }

    obj->marked = 0;
    all_objects[num_objects++] = obj;
    return obj;
}
```

### Step 7: Protect Temporaries During Evaluation

When evaluating expressions, temporary results must be protected from GC:

```c
/* In eval.c - protect temporary values */
LispObject *eval_application(LispObject *expr, Environment *env) {
    // Protect the function while evaluating arguments
    LispObject *func = eval(car(expr), env);
    gc_add_root(&func);  // PROTECT

    // Evaluate arguments
    LispObject *args = eval_list(cdr(expr), env);
    gc_add_root(&args);  // PROTECT

    // Apply function
    LispObject *result = apply(func, args);

    gc_remove_root(&args);  // UNPROTECT
    gc_remove_root(&func);  // UNPROTECT

    return result;
}
```

### Step 8: Register Global Environment

In `main.c` or initialization:

```c
void lisp_init(void) {
    // ... existing init code ...

    // Register global environment as GC root
    gc_add_root((LispObject **)&global_env);
}
```

## File Changes Summary

| File | Changes |
|------|---------|
| `lisp.h` | Add `marked` field to `LispObject` |
| `lisp.c` | Add GC functions: `gc_mark_object`, `gc_sweep`, `gc_collect` |
| `lisp.c` | Add root registry: `gc_add_root`, `gc_remove_root` |
| `lisp.c` | Modify `lisp_alloc` to trigger GC |
| `eval.c` | Protect temporaries with `gc_add_root`/`gc_remove_root` |
| `env.c` | Ensure environments are properly marked |

## Testing the GC

### Test 1: Verify Objects Are Freed

```scheme
; Create lots of garbage
(define (make-garbage n)
  (if (> n 0)
      (begin
        (cons 1 2)  ; Creates garbage - not saved
        (make-garbage (- n 1)))
      'done))

(make-garbage 100000)  ; Should not run out of memory
```

### Test 2: Verify Live Objects Survive

```scheme
(define my-list '(1 2 3 4 5))

; Force GC by allocating
(define (force-gc n)
  (if (> n 0)
      (begin (cons 1 2) (force-gc (- n 1)))
      'done))

(force-gc 100000)

; my-list should still be intact
(display my-list)  ; Should print (1 2 3 4 5)
```

### Test 3: Deep Recursion

```scheme
; This should now work without running out of memory
(define (fib n)
  (if (< n 2) n
      (+ (fib (- n 1)) (fib (- n 2)))))

(fib 30)  ; Should complete (was impossible before)
```

## Performance Considerations

### Current Approach: Stop-the-World

- Simple to implement
- Pauses execution during GC
- Acceptable for interactive use

### Future Improvements

1. **Generational GC** - Track object age, collect young objects more often
2. **Incremental GC** - Spread work across allocations
3. **Concurrent GC** - Run GC in separate thread

## Memory Layout After Implementation

```
┌─────────────────────────────────────────────────────────────────┐
│                        MEMORY LAYOUT                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  GC Roots                    Object Pool                        │
│  ┌──────────┐               ┌──────────────────────────────┐   │
│  │ global_  │──────────────→│ [obj1] [obj2] [obj3] ... [objN]│   │
│  │ env      │               │   ↑      ↑                     │   │
│  ├──────────┤               │   │      │                     │   │
│  │ symbol_  │───────────────┼───┘      │                     │   │
│  │ table    │               │          │                     │   │
│  ├──────────┤               │          │                     │   │
│  │ eval     │───────────────┼──────────┘                     │   │
│  │ stack    │               │                                │   │
│  └──────────┘               └──────────────────────────────────┘   │
│                                                                  │
│  During GC:                                                      │
│  - Mark phase: Follow pointers from roots, set marked=1         │
│  - Sweep phase: Free objects with marked=0, reset marked        │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

## Implementation Checklist

- [x] Add `marked` field to `LispObject` struct (already existed as `gc_mark`)
- [x] Implement `gc_mark_object()` for all types
- [x] Implement `gc_mark_env()` for environments
- [x] Implement `gc_sweep()` to free unmarked objects
- [x] Implement `gc_collect()` main entry point
- [x] Add root registry (`gc_add_root`, `gc_remove_root`, `gc_add_env_root`, `gc_remove_env_root`)
- [x] Modify `lisp_alloc()` to trigger GC
- [x] Protect temporaries in `eval.c`
- [x] Register global environment as root
- [x] Add GC statistics functions (`gc_stats`)
- [x] Test with garbage-intensive programs
- [x] Test that live objects survive GC

## Estimated Effort

| Task | Time |
|------|------|
| Add marked field | 15 min |
| Mark phase | 2 hours |
| Sweep phase | 1 hour |
| Root registry | 30 min |
| Protect temporaries | 2 hours |
| Testing | 2 hours |
| **Total** | **~8 hours** |

## References

- [The Garbage Collection Handbook](https://gchandbook.org/)
- [SICP Section 5.3](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book-Z-H-33.html)
- [Chibi-Scheme GC Implementation](https://github.com/ashinn/chibi-scheme)
- [Baby's First Garbage Collector](https://journal.stuffwithstuff.com/2013/12/08/babys-first-garbage-collector/)
