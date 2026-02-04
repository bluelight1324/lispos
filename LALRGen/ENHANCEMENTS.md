# LALRGen Modernization & Enhancement Opportunities

## Current State
LALRGen is an LALR(1) parser generator based on the "occs" tool from "Compiler Design in C" by Allen Holub (circa 1990). It generates C code with parse tables.

---

## 1. Code Modernization

### 1.1 C++ Migration
- **Current:** C with some C++ features mixed in
- **Enhancement:** Full C++17/20 migration
  - Replace raw pointers with `std::unique_ptr`/`std::shared_ptr`
  - Use `std::string` instead of `TCHAR*` buffers
  - Replace manual memory management (`AllocMem`/`FreeMem`) with RAII
  - Use `std::vector` instead of manual arrays
  - Replace function pointers with `std::function` or templates

### 1.2 Remove Windows Dependencies
- **Current:** Heavy use of Windows-specific types (`TCHAR`, `_TINT`, `_ftprintf`)
- **Enhancement:**
  - Use standard C++ types (`char`, `int`, `std::cout`)
  - Make cross-platform (Linux, macOS)
  - Use CMake for build system instead of Visual Studio projects

### 1.3 Remove Deprecated Pragmas
- **Current:** Many `#pragma warning(disable:xxxx)` to suppress warnings
- **Enhancement:** Fix underlying issues causing warnings instead of suppressing

---

## 2. Architecture Improvements

### 2.1 Modular Design
- **Current:** Monolithic with tight coupling between modules
- **Enhancement:**
  - Separate lexer, parser, and code generator into distinct libraries
  - Define clean interfaces between components
  - Support dependency injection for testing

### 2.2 Error Handling
- **Current:** Uses `error(FATAL, ...)` with `exit()` calls
- **Enhancement:**
  - Use C++ exceptions or `std::expected` (C++23)
  - Provide error recovery mechanisms
  - Better error messages with source location tracking

### 2.3 Symbol Table
- **Current:** Custom hash table implementation (`HASH_TAB`)
- **Enhancement:**
  - Use `std::unordered_map` or `std::map`
  - Support scoped symbol tables
  - Add type information for semantic analysis

---

## 3. Feature Enhancements

### 3.1 GLR Parsing Support
- **Current:** Only LALR(1)
- **Enhancement:** Add GLR (Generalized LR) for ambiguous grammars

### 3.2 Error Recovery
- **Current:** Basic error reporting
- **Enhancement:**
  - Panic mode recovery
  - Phrase-level recovery
  - Error productions support

### 3.3 Semantic Actions
- **Current:** Basic `$$` and `$N` syntax
- **Enhancement:**
  - Named attributes (`$name` instead of `$1`)
  - Mid-rule actions with proper scoping
  - Type checking for attributes

### 3.4 Multiple Output Formats
- **Current:** Only generates C code
- **Enhancement:**
  - C++ output with classes
  - JSON parse tables for other languages
  - LLVM IR generation
  - Rust output

### 3.5 Unicode Support
- **Current:** ASCII/TCHAR only
- **Enhancement:** Full UTF-8/Unicode support in lexer and parser

---

## 4. Developer Experience

### 4.1 Better Diagnostics
- Add colored terminal output for errors/warnings
- Show grammar rule context in error messages
- Generate conflict resolution suggestions

### 4.2 Debug Tools
- Visualize parse tables (DOT/Graphviz output)
- Step-through parser execution
- Grammar analysis reports (unused rules, unreachable symbols)

### 4.3 IDE Integration
- LSP (Language Server Protocol) for grammar files
- Syntax highlighting definitions
- Auto-completion for grammar constructs

---

## 5. Performance Optimizations

### 5.1 Table Compression
- **Current:** Basic pair compression
- **Enhancement:**
  - Row/column displacement compression
  - Graph coloring for table minimization
  - Sparse matrix representations

### 5.2 Code Generation
- Generate switch statements instead of table lookups (for small grammars)
- SIMD-optimized lexer generation
- Branch prediction hints

---

## 6. Testing & Quality

### 6.1 Unit Tests
- Add comprehensive test suite
- Test coverage for all grammar constructs
- Regression tests for known grammars (C, Pascal, etc.)

### 6.2 Fuzzing
- Fuzz test the grammar parser
- Fuzz generated parsers

### 6.3 Static Analysis
- Add clang-tidy configuration
- Address all compiler warnings at highest level

---

## Priority Recommendations

| Priority | Enhancement | Effort | Impact |
|----------|-------------|--------|--------|
| 1 | Cross-platform CMake build | Medium | High |
| 2 | C++17 migration | High | High |
| 3 | Better error messages | Low | High |
| 4 | Unit test suite | Medium | High |
| 5 | Multiple output formats | Medium | Medium |
| 6 | GLR support | High | Medium |
