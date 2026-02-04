# Overcoming the Memory/Recursion Limitation

## The Problem

The LispOS Scheme interpreter runs out of memory during deep recursion:

```
> (tak 18 12 6)
Error: Out of memory: too many objects

> (fib 20)
Error: Out of memory: too many objects
```

## Root Cause Analysis

The issue is in [lisp.c:20-48](../src/lisp.c#L20-L48):

```c
/* Simple memory tracking for GC */
#define MAX_OBJECTS 65536
static LispObject *all_objects[MAX_OBJECTS];
static int num_objects = 0;

/* Allocate a new object */
LispObject *lisp_alloc(void) {
    if (num_objects >= MAX_OBJECTS) {
        lisp_error("Out of memory: too many objects");
        return NULL;
    }
    LispObject *obj = (LispObject *)calloc(1, sizeof(LispObject));
    all_objects[num_objects++] = obj;
    return obj;
}
```

**Problems identified:**

1. **Fixed object limit**: Maximum 65,536 objects
2. **No garbage collection**: Objects are never reclaimed during execution
3. **Linear growth**: Each function call creates new objects (numbers, cons cells, closures)
4. **No object reuse**: Even temporary results stay allocated

### Memory Usage Example

For `(fib 20)`:
- Recursive calls: ~21,891 calls
- Objects per call: ~5-10 (arguments, results, stack frames)
- Total objects: ~100,000+ (exceeds 65,536 limit)

## Solutions

### Solution 1: Implement Mark-and-Sweep Garbage Collection (Recommended)

Add a simple mark-and-sweep GC that runs when memory is low.

#### Implementation Steps:

1. **Add mark bit to objects**:
```c
typedef struct LispObject {
    LispType type;
    unsigned char marked;  // Add this field
    union { ... };
} LispObject;
```

2. **Mark phase** - trace all reachable objects:
```c
void gc_mark(LispObject *obj) {
    if (!obj || obj->marked) return;
    obj->marked = 1;

    switch (obj->type) {
        case LISP_CONS:
            gc_mark(obj->cons.car);
            gc_mark(obj->cons.cdr);
            break;
        case LISP_LAMBDA:
            gc_mark(obj->lambda.params);
            gc_mark(obj->lambda.body);
            gc_mark_env(obj->lambda.env);
            break;
        // ... other types
    }
}
```

3. **Sweep phase** - free unmarked objects:
```c
void gc_sweep(void) {
    int new_count = 0;
    for (int i = 0; i < num_objects; i++) {
        if (all_objects[i]->marked) {
            all_objects[i]->marked = 0;
            all_objects[new_count++] = all_objects[i];
        } else {
            lisp_free(all_objects[i]);
        }
    }
    num_objects = new_count;
}
```

4. **Trigger GC when needed**:
```c
LispObject *lisp_alloc(void) {
    if (num_objects >= MAX_OBJECTS * 0.75) {
        gc_collect();  // Run GC at 75% capacity
    }
    if (num_objects >= MAX_OBJECTS) {
        lisp_error("Out of memory");
        return NULL;
    }
    // ... allocate
}
```

**Pros**: Complete solution, enables unlimited recursion
**Cons**: Requires careful implementation, pause times during GC

---

### Solution 2: Increase Object Pool Size

Simple fix: increase `MAX_OBJECTS` in lisp.c.

```c
// Change from:
#define MAX_OBJECTS 65536

// To:
#define MAX_OBJECTS 1048576  // 1 million objects
```

**Pros**: Trivial change, immediate improvement
**Cons**: Still finite, uses more memory, doesn't solve root cause

**Memory impact**: ~64MB for 1M objects (64 bytes/object average)

---

### Solution 3: Implement Tail Call Optimization (TCO)

For tail-recursive functions, reuse the current stack frame instead of creating a new one.

#### Current problem:
```scheme
(define (sum-iter n acc)
  (if (= n 0)
      acc
      (sum-iter (- n 1) (+ n acc))))  ; Tail call - should reuse frame
```

Each call creates new objects. With TCO, we reuse them.

#### Implementation in eval.c:

```c
// Before evaluating a tail call, check if it's a self-call
// and if so, rebind parameters instead of creating new frame

LispObject *eval_tail(LispObject *expr, Environment *env) {
    while (1) {
        if (is_self_tail_call(expr, env)) {
            // Rebind parameters in current environment
            rebind_params(expr, env);
            expr = get_body(current_lambda);
            continue;  // Loop instead of recurse
        }
        return eval(expr, env);
    }
}
```

**Pros**: Enables infinite tail recursion, standard Scheme requirement
**Cons**: Only helps tail-recursive code, complex to implement fully

---

### Solution 4: Object Pooling for Common Types

Reuse number objects for common values instead of allocating new ones.

```c
// Cache small integers
#define SMALL_INT_MIN -128
#define SMALL_INT_MAX 256
static LispObject *small_ints[SMALL_INT_MAX - SMALL_INT_MIN + 1];

LispObject *make_number(double value) {
    // Check if it's a small integer we can cache
    if (value == (int)value) {
        int i = (int)value;
        if (i >= SMALL_INT_MIN && i <= SMALL_INT_MAX) {
            int idx = i - SMALL_INT_MIN;
            if (!small_ints[idx]) {
                small_ints[idx] = lisp_alloc_real();
                small_ints[idx]->type = LISP_NUMBER;
                small_ints[idx]->number = value;
            }
            return small_ints[idx];
        }
    }
    // Allocate normally for other numbers
    return lisp_alloc_number(value);
}
```

**Pros**: Significant reduction for numeric code, simple to implement
**Cons**: Partial solution, only helps with numbers

---

### Solution 5: Trampoline-Based Evaluation

Convert recursive evaluation to iterative using a trampoline.

```c
typedef struct {
    enum { EVAL_DONE, EVAL_CONTINUE } status;
    LispObject *result;
    LispObject *expr;
    Environment *env;
} Thunk;

LispObject *trampoline_eval(LispObject *expr, Environment *env) {
    Thunk thunk = { EVAL_CONTINUE, NULL, expr, env };

    while (thunk.status == EVAL_CONTINUE) {
        thunk = eval_step(thunk.expr, thunk.env);
    }

    return thunk.result;
}
```

**Pros**: Eliminates C stack overflow, enables deep recursion
**Cons**: Major refactoring required, slower for simple cases

---

## Recommended Implementation Order

1. **Quick fix** (5 minutes): Increase `MAX_OBJECTS` to 1M
2. **Medium term** (2-4 hours): Implement small integer caching
3. **Full solution** (1-2 days): Add mark-and-sweep GC
4. **Optimization** (2-4 hours): Add tail call optimization

## Quick Fix Implementation

To immediately improve the situation, edit [lisp.c](../src/lisp.c):

```c
// Line 21: Change
#define MAX_OBJECTS 65536
// To
#define MAX_OBJECTS 1048576
```

Then rebuild:
```bash
cd LispCompiler
cmake --build build --config Release
```

## Testing the Fix

After implementing any solution, test with:

```scheme
; Should complete without error
(define (fib n)
  (if (< n 2) n
      (+ (fib (- n 1)) (fib (- n 2)))))

(fib 25)  ; Returns 75025

; Tail-recursive version (works better with TCO)
(define (fib-iter n a b)
  (if (= n 0) a
      (fib-iter (- n 1) b (+ a b))))

(fib-iter 100 0 1)  ; Returns very large number
```

## References

- [Garbage Collection Handbook](https://gchandbook.org/) - Comprehensive GC reference
- [R7RS Specification](https://small.r7rs.org/) - Tail call requirements
- [Chibi-Scheme GC](https://github.com/ashinn/chibi-scheme) - Reference implementation
- [SICP Chapter 5.3](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book-Z-H-33.html) - Storage allocation and GC
