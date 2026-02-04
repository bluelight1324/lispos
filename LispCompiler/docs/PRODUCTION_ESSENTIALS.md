# Production Essentials

This document identifies the **absolute minimum requirements** for deploying the LispCompiler and SchemeEdit in a production real-world environment. These are non-negotiable items that must be addressed before any production use.

## Executive Summary

| Category | Essential Item | Current Status | Risk if Missing |
|----------|----------------|----------------|-----------------|
| **Stability** | Crash-free operation | ⚠️ 85% | System unusable |
| **Errors** | Graceful error handling | ⚠️ 70% | Data loss, crashes |
| **Memory** | No memory leaks | ⚠️ 80% | System degradation |
| **Security** | Input validation | ⚠️ 60% | Exploitable vulnerabilities |
| **Testing** | Core functionality verified | ⚠️ 75% | Hidden bugs |

---

## The 5 Absolute Essentials

### 1. Crash-Free Core Operations

**What**: The interpreter must not crash on valid Scheme code.

**Current Issues**:
- Some edge cases in recursive functions can cause stack overflow
- Malformed input can crash the parser
- Certain GC timing issues under heavy load

**Minimum Fix Required**:
```c
// Must implement: Stack overflow protection
#define MAX_RECURSION_DEPTH 10000

int eval_with_depth_check(LispObject *expr, Environment *env, int depth) {
    if (depth > MAX_RECURSION_DEPTH) {
        return lisp_error("Maximum recursion depth exceeded");
    }
    // ... rest of evaluation
}
```

**Acceptance Criteria**:
- [ ] No crash on any valid R7RS-small program
- [ ] Graceful error on stack overflow
- [ ] Graceful error on out-of-memory
- [ ] Can run for 24+ hours under load without crashing

---

### 2. Proper Error Handling

**What**: All errors must be caught and reported, never cause undefined behavior.

**Current Issues**:
- Division by zero may crash
- Null pointer dereferences possible
- Some errors don't report line numbers

**Minimum Fix Required**:
```c
// Must implement: Error codes and handling
typedef enum {
    SCHEME_OK = 0,
    SCHEME_ERROR_SYNTAX,
    SCHEME_ERROR_RUNTIME,
    SCHEME_ERROR_TYPE,
    SCHEME_ERROR_MEMORY,
    SCHEME_ERROR_IO
} SchemeResult;

SchemeResult safe_divide(LispObject *a, LispObject *b, LispObject **result) {
    if (b->number == 0) {
        return lisp_error_at(SCHEME_ERROR_RUNTIME,
                            "Division by zero",
                            current_line, current_col);
    }
    *result = make_number(a->number / b->number);
    return SCHEME_OK;
}
```

**Acceptance Criteria**:
- [ ] All runtime errors caught and reported with location
- [ ] No undefined behavior on any input
- [ ] Clear, actionable error messages
- [ ] Error recovery (REPL continues after error)

---

### 3. Memory Safety

**What**: No memory leaks, no use-after-free, no buffer overflows.

**Current Issues**:
- Long-running programs may slowly leak memory
- Some string operations lack bounds checking
- GC may miss some objects in certain scenarios

**Minimum Fix Required**:
```c
// Must implement: Safe string operations
char* safe_string_copy(const char *src, size_t max_len) {
    if (!src) return NULL;
    size_t len = strnlen(src, max_len);
    char *dst = gc_malloc(len + 1);
    if (!dst) {
        lisp_error("Out of memory");
        return NULL;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

// Must verify: GC correctness
void verify_gc_roots(void) {
    // Ensure all live objects are reachable from roots
}
```

**Acceptance Criteria**:
- [ ] Pass Valgrind/AddressSanitizer with no errors
- [ ] Stable memory usage over 24-hour run
- [ ] No buffer overflows (verified by fuzzing)
- [ ] GC correctly frees all unreachable objects

---

### 4. Input Validation

**What**: All external inputs must be validated before use.

**Current Issues**:
- File paths not sanitized
- User input can contain malicious code
- No limits on input size

**Minimum Fix Required**:
```c
// Must implement: Input validation
#define MAX_INPUT_SIZE (10 * 1024 * 1024)  // 10 MB max
#define MAX_STRING_LENGTH 1000000
#define MAX_LIST_LENGTH 1000000
#define MAX_FILE_PATH 4096

SchemeResult validate_file_path(const char *path) {
    if (!path) return SCHEME_ERROR_IO;
    if (strlen(path) > MAX_FILE_PATH) return SCHEME_ERROR_IO;

    // Prevent directory traversal
    if (strstr(path, "..")) return SCHEME_ERROR_IO;

    // Validate characters
    for (const char *p = path; *p; p++) {
        if (!isprint(*p)) return SCHEME_ERROR_IO;
    }
    return SCHEME_OK;
}

SchemeResult validate_input_size(size_t size) {
    if (size > MAX_INPUT_SIZE) {
        return lisp_error("Input exceeds maximum size");
    }
    return SCHEME_OK;
}
```

**Acceptance Criteria**:
- [ ] All file operations validate paths
- [ ] Input size limits enforced
- [ ] No code injection vulnerabilities
- [ ] Resource limits prevent DoS

---

### 5. Core Functionality Verification

**What**: All documented features must work correctly.

**Current Issues**:
- Some R7RS functions have edge case bugs
- Test coverage is incomplete
- No automated regression testing

**Minimum Fix Required**:
```scheme
;; Must pass: Core test suite (minimum 100 tests)

; Arithmetic
(assert (= (+ 1 2) 3))
(assert (= (- 10 5) 5))
(assert (= (* 3 4) 12))
(assert (= (/ 20 4) 5))
(assert (= (modulo 17 5) 2))

; Lists
(assert (equal? (car '(1 2 3)) 1))
(assert (equal? (cdr '(1 2 3)) '(2 3)))
(assert (equal? (cons 1 '(2 3)) '(1 2 3)))
(assert (equal? (length '(1 2 3)) 3))
(assert (equal? (reverse '(1 2 3)) '(3 2 1)))

; Higher-order functions
(assert (equal? (map (lambda (x) (* x 2)) '(1 2 3)) '(2 4 6)))
(assert (equal? (filter (lambda (x) (> x 2)) '(1 2 3 4)) '(3 4)))

; Recursion (verified deep)
(assert (= (factorial 10) 3628800))
(assert (= (fibonacci 20) 6765))

; Closures
(define counter (make-counter))
(assert (= (counter) 1))
(assert (= (counter) 2))
```

**Acceptance Criteria**:
- [ ] 100% pass rate on core test suite
- [ ] All documented functions work as specified
- [ ] Edge cases tested (empty lists, zero, negative numbers)
- [ ] Performance meets baseline (fib(30) < 5 seconds)

---

## Production Checklist

### Before Going Live

#### Critical (Must Do)

- [ ] **Run full test suite** - All tests pass
- [ ] **Memory check** - Run with sanitizers, no errors
- [ ] **Error handling audit** - Verify all error paths
- [ ] **Input fuzzing** - Run fuzzer for 24 hours, no crashes
- [ ] **Load test** - Run under heavy load for 24 hours
- [ ] **Review security** - Audit file operations and eval

#### Important (Should Do)

- [ ] **Documentation review** - Verify accuracy of all docs
- [ ] **Install testing** - Fresh install on clean machine
- [ ] **User acceptance** - Get feedback from beta users
- [ ] **Backup plan** - Document rollback procedure

### Minimum Test Coverage

| Component | Required Tests | Current |
|-----------|---------------|---------|
| Lexer | 30 | ~10 |
| Parser | 50 | ~15 |
| Evaluator | 100 | ~30 |
| Built-ins | 200 | ~50 |
| GC | 30 | ~10 |
| Debugger | 50 | ~20 |
| **Total** | **460** | **~135** |

**Gap**: ~325 more tests needed for minimum coverage.

---

## What Can Be Deferred

These items from PRODUCTION_READINESS.md are **not essential** for initial production:

| Item | Why It Can Wait |
|------|-----------------|
| Full R7RS compliance | Core features work, advanced features can be added later |
| Bytecode compiler | Interpreter is fast enough for most use cases |
| Generational GC | Current GC works, optimization is not critical |
| Cross-platform | Windows works, macOS/Linux can come later |
| VS Code Marketplace | Users can install manually |
| Multiple themes | Default theme is usable |
| REPL | Run file mode is sufficient initially |
| Refactoring tools | Users can edit manually |
| Plugin system | Core functionality is enough |

---

## Implementation Order

### Week 1: Stability
1. Implement stack overflow protection
2. Add null checks to all pointer dereferences
3. Fix known crash-causing edge cases

### Week 2: Error Handling
1. Add error codes to all functions
2. Implement line/column tracking for errors
3. Add try/catch around all entry points

### Week 3: Memory Safety
1. Run AddressSanitizer, fix all issues
2. Audit all string operations
3. Verify GC correctness with test suite

### Week 4: Security & Testing
1. Implement input validation
2. Add size limits everywhere
3. Write remaining core tests
4. Run fuzzer and fix findings

---

## Success Metrics

### Minimum Viable Production

| Metric | Target | How to Measure |
|--------|--------|----------------|
| Crash rate | < 0.01% | Crashes per 10,000 runs |
| Error handling | 100% | All errors produce message |
| Memory leaks | 0 | Valgrind clean |
| Security vulns | 0 critical | Security audit |
| Test pass rate | 100% | Automated test suite |
| Uptime | 99.9% | 24-hour continuous run |

### Ready for Production When:

1. ✅ All critical tests pass
2. ✅ No crashes on valid input
3. ✅ All errors handled gracefully
4. ✅ Memory is stable over time
5. ✅ Security audit completed
6. ✅ Documentation is accurate

---

## Conclusion

**The absolute essentials for production are:**

1. **Don't crash** - Handle all edge cases
2. **Don't lose data** - Proper error handling
3. **Don't leak** - Memory safety
4. **Don't be exploitable** - Input validation
5. **Don't break** - Core functionality verified

Everything else (performance, features, polish) can be improved over time. These five essentials are non-negotiable for any production deployment.

**Estimated effort**: 4 weeks of focused work
**Risk without**: System failures, data loss, security breaches

Once these essentials are in place, the LispCompiler can be confidently deployed in production environments.
