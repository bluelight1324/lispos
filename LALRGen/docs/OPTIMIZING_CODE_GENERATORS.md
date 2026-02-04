# Optimizing Code Generator Options

A comprehensive guide to optimization strategies and available frameworks for
building an optimizing compiler backend.

## Table of Contents

1. [Overview](#overview)
2. [Optimization Levels](#optimization-levels)
3. [Frontend Optimizations](#frontend-optimizations)
4. [Middle-End Optimizations (IR Level)](#middle-end-optimizations-ir-level)
5. [Backend Optimizations](#backend-optimizations)
6. [Available Frameworks](#available-frameworks)
7. [Build vs Buy Decision](#build-vs-buy-decision)
8. [Implementation Recommendations](#implementation-recommendations)

---

## Overview

Code optimization can occur at multiple stages of compilation:

```
┌─────────────────────────────────────────────────────────────────┐
│  Source Code                                                    │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  FRONTEND OPTIMIZATIONS                                         │
│  • Constant folding                                             │
│  • Algebraic simplification                                     │
│  • Dead code elimination (obvious cases)                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  MIDDLE-END OPTIMIZATIONS (IR)                                  │
│  • SSA construction                                             │
│  • Common subexpression elimination                             │
│  • Loop optimizations                                           │
│  • Inlining                                                     │
│  • Interprocedural analysis                                     │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  BACKEND OPTIMIZATIONS                                          │
│  • Instruction selection                                        │
│  • Register allocation                                          │
│  • Instruction scheduling                                       │
│  • Peephole optimization                                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Optimized Machine Code                                         │
└─────────────────────────────────────────────────────────────────┘
```

---

## Optimization Levels

### Standard Optimization Levels

| Level | Description | Typical Use |
|-------|-------------|-------------|
| **-O0** | No optimization, fast compile | Debugging |
| **-O1** | Basic optimizations | Balance of speed/size |
| **-O2** | Full optimizations | Production builds |
| **-O3** | Aggressive optimizations | Maximum performance |
| **-Os** | Optimize for size | Embedded systems |
| **-Oz** | Aggressive size optimization | Very constrained |

### What Each Level Typically Includes

**-O1 (Basic):**
- Constant folding and propagation
- Dead code elimination
- Basic algebraic simplification
- Jump threading

**-O2 (Standard):**
- All of -O1
- Common subexpression elimination (CSE)
- Loop-invariant code motion (LICM)
- Function inlining (small functions)
- Tail call optimization
- Register allocation

**-O3 (Aggressive):**
- All of -O2
- Aggressive inlining
- Loop unrolling
- Vectorization (SIMD)
- Profile-guided optimizations

---

## Frontend Optimizations

These are easy to implement during AST traversal or early IR generation.

### 1. Constant Folding

Evaluate constant expressions at compile time.

```c
// Before
int x = 2 + 3 * 4;

// After constant folding
int x = 14;
```

**Implementation:**
```c
AST_Node *fold_constants(AST_Node *node) {
    if (node->kind != AST_BINARY_OP) return node;

    // Recursively fold children
    node->left = fold_constants(node->left);
    node->right = fold_constants(node->right);

    // If both operands are constants, evaluate
    if (is_constant(node->left) && is_constant(node->right)) {
        int left_val = get_constant_value(node->left);
        int right_val = get_constant_value(node->right);
        int result;

        switch (node->op) {
            case OP_ADD: result = left_val + right_val; break;
            case OP_SUB: result = left_val - right_val; break;
            case OP_MUL: result = left_val * right_val; break;
            case OP_DIV:
                if (right_val == 0) return node;  // Can't fold
                result = left_val / right_val;
                break;
            default: return node;
        }

        return make_constant_node(result);
    }

    return node;
}
```

### 2. Constant Propagation

Replace variables with known constant values.

```c
// Before
int x = 5;
int y = x + 3;

// After constant propagation
int x = 5;
int y = 5 + 3;  // Then fold to: int y = 8;
```

### 3. Algebraic Simplification

Apply mathematical identities.

```c
// Identities to apply:
x + 0  → x
x * 1  → x
x * 0  → 0
x - 0  → x
x / 1  → x
x * 2  → x << 1     // Strength reduction
x / 2  → x >> 1     // For unsigned
x % 2  → x & 1      // For unsigned
x * 2^n → x << n    // Power of 2 multiplication
```

**Implementation:**
```c
AST_Node *simplify_algebraic(AST_Node *node) {
    if (node->kind != AST_BINARY_OP) return node;

    if (node->op == OP_ADD) {
        if (is_zero(node->right)) return node->left;
        if (is_zero(node->left)) return node->right;
    }

    if (node->op == OP_MUL) {
        if (is_zero(node->left) || is_zero(node->right))
            return make_constant_node(0);
        if (is_one(node->right)) return node->left;
        if (is_one(node->left)) return node->right;

        // Strength reduction: x * 2^n → x << n
        if (is_power_of_2(node->right)) {
            int shift = log2_int(get_constant_value(node->right));
            return make_shift_left(node->left, shift);
        }
    }

    return node;
}
```

### 4. Dead Code Elimination (Simple)

Remove unreachable code and unused variables.

```c
// Before
if (0) {
    printf("never executed");  // Dead code
}

int x = 5;  // x never used → dead assignment

// After
// (both removed)
```

---

## Middle-End Optimizations (IR Level)

These require a proper intermediate representation, typically SSA form.

### 1. Static Single Assignment (SSA)

Each variable is assigned exactly once. Enables many optimizations.

```c
// Original
x = 1;
x = 2;
y = x;

// SSA form
x1 = 1;
x2 = 2;
y1 = x2;
```

**Phi functions** at control flow join points:
```c
// Original
if (cond) x = 1; else x = 2;
y = x;

// SSA form
if (cond)
    x1 = 1;
else
    x2 = 2;
x3 = φ(x1, x2);  // Phi function
y1 = x3;
```

### 2. Common Subexpression Elimination (CSE)

Avoid recomputing the same expression.

```c
// Before
a = b + c;
d = b + c;

// After CSE
t = b + c;
a = t;
d = t;
```

**Implementation using value numbering:**
```c
typedef struct {
    TacOp op;
    int arg1_vn;   // Value number of arg1
    int arg2_vn;   // Value number of arg2
    int result_vn; // Value number of result
} ValueEntry;

int find_or_create_value_number(TacInstr *instr, HashTable *table) {
    // Create a key from the operation and operand value numbers
    ValueEntry key = {
        .op = instr->op,
        .arg1_vn = get_value_number(instr->arg1),
        .arg2_vn = get_value_number(instr->arg2)
    };

    // Look up in hash table
    ValueEntry *existing = hash_lookup(table, &key);
    if (existing) {
        // Replace with copy from existing result
        return existing->result_vn;
    }

    // New expression, create new value number
    int new_vn = next_value_number++;
    key.result_vn = new_vn;
    hash_insert(table, &key);
    return new_vn;
}
```

### 3. Loop-Invariant Code Motion (LICM)

Move computations that don't change inside loops to outside.

```c
// Before
for (int i = 0; i < n; i++) {
    int t = a * b;  // Loop-invariant
    arr[i] = t + i;
}

// After LICM
int t = a * b;  // Moved outside loop
for (int i = 0; i < n; i++) {
    arr[i] = t + i;
}
```

### 4. Loop Unrolling

Reduce loop overhead by duplicating the body.

```c
// Before
for (int i = 0; i < 4; i++) {
    a[i] = 0;
}

// After unrolling (factor 4)
a[0] = 0;
a[1] = 0;
a[2] = 0;
a[3] = 0;

// Partial unrolling (factor 2)
for (int i = 0; i < 4; i += 2) {
    a[i] = 0;
    a[i+1] = 0;
}
```

### 5. Function Inlining

Replace function calls with the function body.

```c
// Before
inline int square(int x) { return x * x; }
int y = square(5);

// After inlining
int y = 5 * 5;  // Then fold to: int y = 25;
```

**Inlining heuristics:**
- Function is small (< 20 instructions)
- Function is called only once (always inline)
- Function is hot (profile-guided)
- Function is marked `inline`

### 6. Tail Call Optimization

Convert tail-recursive calls to jumps.

```c
// Before
int factorial(int n, int acc) {
    if (n <= 1) return acc;
    return factorial(n - 1, n * acc);  // Tail call
}

// After TCO (pseudo-assembly)
factorial:
    cmp n, 1
    jle .done
    imul acc, n
    dec n
    jmp factorial    ; Jump, not call!
.done:
    mov eax, acc
    ret
```

### 7. Strength Reduction

Replace expensive operations with cheaper ones.

```c
// Loop strength reduction
// Before
for (int i = 0; i < n; i++) {
    a[i * 4] = 0;  // Multiply each iteration
}

// After
int t = 0;
for (int i = 0; i < n; i++) {
    a[t] = 0;
    t += 4;  // Add instead of multiply
}
```

---

## Backend Optimizations

These generate efficient machine code from the IR.

### 1. Instruction Selection

Choose the best machine instructions for IR operations.

```c
// IR
t1 = a + b * c

// Naive selection (3 instructions)
mov eax, b
imul eax, c
add eax, a

// Better: use LEA for address calculations
// t1 = a + b (when b is small constant)
lea eax, [a + b]

// Even better: use fused multiply-add if available
// FMA: a + b * c in single instruction (AVX)
vfmadd231ss xmm0, xmm1, xmm2
```

**Pattern matching approach:**
```c
typedef struct {
    IrPattern pattern;
    const char *asm_template;
    int cost;
} Rule;

Rule rules[] = {
    // Pattern               Assembly              Cost
    { IR_ADD_REG_REG,       "add %d, %s",         1 },
    { IR_ADD_REG_CONST,     "add %d, %i",         1 },
    { IR_MUL_REG_POW2,      "shl %d, %i",         1 },  // Cheaper than mul
    { IR_ADD_MUL_REG_REG,   "lea %d, [%s+%s*%i]", 1 },  // Fused
};
```

### 2. Register Allocation

Map virtual registers/temporaries to physical registers.

**Algorithms by complexity:**

| Algorithm | Complexity | Quality | Use Case |
|-----------|------------|---------|----------|
| Local (per basic block) | O(n) | Poor | Fast compilation |
| Linear Scan | O(n log n) | Good | JIT compilers |
| Graph Coloring | O(n²) | Excellent | Production compilers |
| PBQP | O(n²) | Excellent | Complex ISAs |

**Linear Scan Algorithm:**
```c
// Simplified linear scan
void linear_scan_allocate(Function *func) {
    // 1. Compute live intervals
    Interval *intervals = compute_live_intervals(func);

    // 2. Sort by start point
    sort_by_start(intervals);

    // 3. Allocate registers
    List *active = list_new();
    int regs[NUM_REGS] = {0};

    for (Interval *i = intervals; i; i = i->next) {
        // Expire old intervals
        expire_old_intervals(active, i->start, regs);

        // Try to allocate a register
        int reg = find_free_register(regs);
        if (reg >= 0) {
            i->reg = reg;
            regs[reg] = 1;
            list_add(active, i);
        } else {
            // Spill: find interval to spill
            spill_at_interval(active, i, regs);
        }
    }
}
```

**Graph Coloring (Chaitin's algorithm):**
```c
void graph_coloring_allocate(Function *func) {
    // 1. Build interference graph
    Graph *ig = build_interference_graph(func);

    // 2. Simplify: remove low-degree nodes
    Stack *stack = stack_new();
    while (!graph_empty(ig)) {
        Node *n = find_low_degree_node(ig, NUM_REGS);
        if (n) {
            stack_push(stack, n);
            graph_remove(ig, n);
        } else {
            // Potential spill: pick node to spill
            n = pick_spill_candidate(ig);
            n->potential_spill = true;
            stack_push(stack, n);
            graph_remove(ig, n);
        }
    }

    // 3. Select: assign colors (registers)
    while (!stack_empty(stack)) {
        Node *n = stack_pop(stack);
        int color = find_available_color(n, NUM_REGS);
        if (color >= 0) {
            n->color = color;
        } else {
            // Actual spill required
            insert_spill_code(func, n);
            // Restart allocation
            return graph_coloring_allocate(func);
        }
    }
}
```

### 3. Instruction Scheduling

Reorder instructions to hide latencies and utilize pipelines.

```asm
; Before scheduling
load r1, [mem1]    ; Latency 3 cycles
add r2, r1, r3     ; Stalls waiting for r1!
load r4, [mem2]
add r5, r4, r6     ; Stalls waiting for r4!

; After scheduling (interleaved loads)
load r1, [mem1]    ; Start load
load r4, [mem2]    ; Start load (parallel with first)
; ... other independent work ...
add r2, r1, r3     ; r1 now ready
add r5, r4, r6     ; r4 now ready
```

**List scheduling algorithm:**
```c
void list_schedule(BasicBlock *bb) {
    // 1. Build dependence DAG
    DAG *dag = build_dependence_dag(bb);

    // 2. Compute priorities (e.g., critical path length)
    compute_priorities(dag);

    // 3. Schedule instructions
    List *ready = list_new();
    int cycle = 0;

    // Add instructions with no predecessors
    for (Node *n : dag->nodes) {
        if (n->num_preds == 0)
            list_add(ready, n);
    }

    while (!list_empty(ready)) {
        // Pick highest priority ready instruction
        Node *best = pick_best(ready, cycle);

        // Schedule it
        best->scheduled_cycle = cycle;
        emit(best->instr);

        // Update ready list
        for (Node *succ : best->successors) {
            succ->num_preds--;
            if (succ->num_preds == 0 &&
                cycle >= succ->earliest_start)
                list_add(ready, succ);
        }

        cycle++;
    }
}
```

### 4. Peephole Optimization

Pattern-match and optimize small instruction sequences.

```asm
; Pattern: redundant move
mov eax, ebx
mov ebx, eax  ; Remove this

; Pattern: redundant load after store
mov [mem], eax
mov eax, [mem]  ; Remove this (value already in eax)

; Pattern: strength reduction
imul eax, 2     ; Replace with: add eax, eax
                ; Or: shl eax, 1

; Pattern: compare optimization
sub eax, 0      ; Replace with: test eax, eax
cmp eax, 0      ; Replace with: test eax, eax
```

**Implementation:**
```c
typedef struct {
    Pattern pattern;
    Instruction *replacement;
} PeepholeRule;

void peephole_optimize(InstrList *code) {
    for (Instr *i = code->head; i; ) {
        bool matched = false;

        for (PeepholeRule *rule : peephole_rules) {
            if (matches(i, rule->pattern)) {
                i = apply_replacement(i, rule->replacement);
                matched = true;
                break;
            }
        }

        if (!matched) i = i->next;
    }
}
```

---

## Available Frameworks

### Option 1: LLVM (Recommended for Production)

LLVM provides a complete optimizing backend infrastructure.

**Advantages:**
- Industry-standard optimization passes
- Multiple target architectures (x86, ARM, RISC-V, etc.)
- Active development and community
- Used by Clang, Rust, Swift, Julia

**Integration approach:**
```c
// Generate LLVM IR from your AST
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>

LLVMModuleRef module = LLVMModuleCreateWithName("my_module");
LLVMBuilderRef builder = LLVMCreateBuilder();

// Create a function
LLVMTypeRef param_types[] = {LLVMInt32Type(), LLVMInt32Type()};
LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0);
LLVMValueRef add_func = LLVMAddFunction(module, "add", func_type);

// Create basic block
LLVMBasicBlockRef entry = LLVMAppendBasicBlock(add_func, "entry");
LLVMPositionBuilderAtEnd(builder, entry);

// Generate IR: return a + b
LLVMValueRef a = LLVMGetParam(add_func, 0);
LLVMValueRef b = LLVMGetParam(add_func, 1);
LLVMValueRef sum = LLVMBuildAdd(builder, a, b, "sum");
LLVMBuildRet(builder, sum);

// Run optimization passes
LLVMPassManagerRef pm = LLVMCreatePassManager();
LLVMAddPromoteMemoryToRegisterPass(pm);
LLVMAddInstructionCombiningPass(pm);
LLVMAddReassociatePass(pm);
LLVMAddGVNPass(pm);
LLVMAddCFGSimplificationPass(pm);
LLVMRunPassManager(pm, module);

// Generate object code
LLVMInitializeX86TargetInfo();
LLVMInitializeX86Target();
LLVMInitializeX86TargetMC();
LLVMInitializeX86AsmPrinter();

char *error = NULL;
LLVMTargetMachineRef tm = LLVMCreateTargetMachine(...);
LLVMTargetMachineEmitToFile(tm, module, "output.o",
                             LLVMObjectFile, &error);
```

### Option 2: QBE (Quick Backend)

Lightweight alternative to LLVM for simpler compilers.

**Advantages:**
- Much simpler than LLVM (~15K lines of code)
- Fast compilation
- Good enough optimization for many use cases
- Easy to integrate

**QBE IR example:**
```
function w $add(w %a, w %b) {
@start
    %sum =w add %a, %b
    ret %sum
}
```

### Option 3: libFirm

Academic-quality SSA-based optimizer.

**Advantages:**
- Clean SSA-based design
- Good optimization quality
- Simpler than LLVM

### Option 4: Cranelift

Designed for fast compilation (JIT use cases).

**Advantages:**
- Fast compile times
- Reasonable optimization
- Written in Rust (memory safe)
- Used by Wasmtime

### Comparison Table

| Framework | Compile Speed | Optimization | Complexity | Best For |
|-----------|---------------|--------------|------------|----------|
| LLVM | Slow | Excellent | Very High | Production compilers |
| QBE | Fast | Good | Low | Simple compilers |
| libFirm | Medium | Good | Medium | Learning, research |
| Cranelift | Very Fast | Moderate | Medium | JIT compilers |
| Custom | Varies | Basic-Good | Varies | Full control |

---

## Build vs Buy Decision

### Build Your Own When:

1. **Learning:** You want to understand compiler optimization
2. **Control:** You need complete control over generated code
3. **Simplicity:** Your language is simple and doesn't need advanced optimization
4. **Special requirements:** Unusual target or optimization constraints

### Use LLVM/QBE When:

1. **Production quality:** You need competitive performance
2. **Multiple targets:** You want to support x86, ARM, etc.
3. **Time constraints:** You want to focus on frontend features
4. **Maintenance:** You want ongoing improvements from the community

### Hybrid Approach

Many successful compilers use a hybrid:
- Custom frontend optimizations (constant folding, etc.)
- LLVM for middle-end and backend

Example: Rust, Swift, Julia all use LLVM backends.

---

## Implementation Recommendations

### Minimal Optimizer (Easy)

Implement these for noticeable improvement with low effort:

1. **Constant folding** - 1 day
2. **Algebraic simplification** - 1 day
3. **Dead code elimination** - 2 days
4. **Peephole optimization** - 2 days

Total: ~1 week for basic optimizations

### Good Optimizer (Medium)

Add these for good optimization quality:

1. **SSA construction** - 1 week
2. **Common subexpression elimination** - 3 days
3. **Copy propagation** - 2 days
4. **Linear scan register allocation** - 1 week
5. **Basic instruction scheduling** - 3 days

Total: ~1 month

### Production Optimizer (Hard)

For production-quality optimization:

1. Use LLVM as backend
2. Implement custom frontend optimizations
3. Profile-guided optimization infrastructure
4. Target-specific tuning

Total: Use LLVM to save years of work

### Quick Start: Using LLVM

```bash
# Install LLVM (Windows)
winget install LLVM.LLVM

# Install LLVM (Linux)
apt install llvm-dev

# Compile with LLVM
clang -c mycompiler.c `llvm-config --cflags`
clang -o mycompiler mycompiler.o `llvm-config --ldflags --libs core analysis native`
```

---

## Summary

| Approach | Effort | Result | Recommendation |
|----------|--------|--------|----------------|
| No optimization | Minimal | Slow code | Prototyping only |
| Basic (fold/DCE) | 1 week | 2-3x faster | Educational projects |
| Good (SSA/CSE) | 1 month | 5-10x faster | Simple production |
| LLVM backend | 2 weeks | 50-100x faster | Production compilers |

### Key Takeaways

1. **Start simple:** Constant folding and dead code elimination give good ROI
2. **SSA is fundamental:** Required for most advanced optimizations
3. **Register allocation matters:** Poor allocation can negate other optimizations
4. **Consider LLVM:** For production, LLVM saves years of work
5. **Measure everything:** Profile to find optimization opportunities

### Further Reading

1. "Engineering a Compiler" - Cooper & Torczon
2. "Modern Compiler Implementation" - Andrew Appel
3. "SSA-based Compiler Design" - Springer
4. LLVM documentation: https://llvm.org/docs/
5. QBE documentation: https://c9x.me/compile/
