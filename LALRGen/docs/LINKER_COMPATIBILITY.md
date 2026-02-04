# Compiler Output Compatibility with Linkers

This document describes how to generate compiler output that is compatible
with the Microsoft linker (LINK.exe), GNU linker (ld), and LLVM linker (lld).

## Table of Contents

1. [Overview](#overview)
2. [Object File Formats](#object-file-formats)
3. [Calling Conventions](#calling-conventions)
4. [Assembly Syntax](#assembly-syntax)
5. [Symbol Naming and Decoration](#symbol-naming-and-decoration)
6. [Runtime Library Linking](#runtime-library-linking)
7. [Complete Examples](#complete-examples)
8. [Build Toolchain Integration](#build-toolchain-integration)

---

## Overview

A compiler must generate output in a format that linkers can process. The
two main approaches are:

1. **Generate Assembly** → Assemble with MASM/NASM/GAS → Link
2. **Generate Object Files Directly** → Link

For simplicity and portability, generating assembly is recommended for
educational compilers. Production compilers typically emit object files
directly for performance.

### Supported Toolchains

| Platform | Assembler | Linker | Object Format |
|----------|-----------|--------|---------------|
| Windows x64 | MASM (ml64) | LINK.exe | COFF/PE |
| Windows x64 | NASM | LINK.exe | COFF/PE |
| Linux x64 | GAS (as) | ld / gold | ELF |
| macOS x64 | as | ld | Mach-O |
| Cross-platform | LLVM | lld | All formats |

---

## Object File Formats

### Windows: COFF/PE Format

Microsoft's Common Object File Format (COFF) is used for `.obj` files.

**Key sections:**
```
.text    - Executable code
.data    - Initialized data
.bss     - Uninitialized data (zero-filled)
.rdata   - Read-only data (constants, strings)
```

**COFF Header Structure (simplified):**
```c
typedef struct {
    uint16_t Machine;           // IMAGE_FILE_MACHINE_AMD64 = 0x8664
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} COFF_Header;

typedef struct {
    char     Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLineNumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLineNumbers;
    uint32_t Characteristics;
} COFF_Section;
```

### Linux: ELF Format

Executable and Linkable Format (ELF) is the standard on Unix-like systems.

**Key sections:**
```
.text    - Executable code
.data    - Initialized data
.bss     - Uninitialized data
.rodata  - Read-only data
```

### Generating Assembly Instead of Object Files

For simplicity, generate assembly and use the platform's assembler:

```c
// codegen.c - Generate assembly for target platform

#ifdef _WIN32
    #define ASM_COMMENT ";"
    #define ASM_SYNTAX  SYNTAX_MASM
#else
    #define ASM_COMMENT "#"
    #define ASM_SYNTAX  SYNTAX_GAS
#endif

void emit_prologue(FILE *out, const char *func_name) {
#ifdef _WIN32
    // MASM syntax
    fprintf(out, "%s PROC\n", func_name);
    fprintf(out, "    push rbp\n");
    fprintf(out, "    mov rbp, rsp\n");
#else
    // GAS syntax
    fprintf(out, ".globl %s\n", func_name);
    fprintf(out, "%s:\n", func_name);
    fprintf(out, "    pushq %%rbp\n");
    fprintf(out, "    movq %%rsp, %%rbp\n");
#endif
}
```

---

## Calling Conventions

### Windows x64 Calling Convention

Microsoft x64 ABI:

| Parameter | Register |
|-----------|----------|
| 1st integer/pointer | RCX |
| 2nd integer/pointer | RDX |
| 3rd integer/pointer | R8 |
| 4th integer/pointer | R9 |
| 5th+ | Stack (right to left) |

| Parameter | Register |
|-----------|----------|
| 1st float/double | XMM0 |
| 2nd float/double | XMM1 |
| 3rd float/double | XMM2 |
| 4th float/double | XMM3 |

**Return value:** RAX (integer), XMM0 (float/double)

**Shadow space:** Caller must allocate 32 bytes on stack before CALL.

**Preserved registers (callee-saved):** RBX, RBP, RDI, RSI, R12-R15, XMM6-XMM15

```asm
; Windows x64 example: int add(int a, int b)
; a in ECX, b in EDX
add PROC
    mov eax, ecx      ; First param
    add eax, edx      ; Second param
    ret
add ENDP
```

### System V AMD64 ABI (Linux/macOS)

| Parameter | Register |
|-----------|----------|
| 1st integer/pointer | RDI |
| 2nd integer/pointer | RSI |
| 3rd integer/pointer | RDX |
| 4th integer/pointer | RCX |
| 5th integer/pointer | R8 |
| 6th integer/pointer | R9 |
| 7th+ | Stack (right to left) |

| Parameter | Register |
|-----------|----------|
| 1st-8th float/double | XMM0-XMM7 |

**Return value:** RAX (integer), XMM0 (float/double)

**Red zone:** 128 bytes below RSP that leaf functions can use.

**Preserved registers (callee-saved):** RBX, RBP, R12-R15

```asm
# System V example: int add(int a, int b)
# a in EDI, b in ESI
.globl add
add:
    movl %edi, %eax   # First param
    addl %esi, %eax   # Second param
    ret
```

### Code Generator for Both Conventions

```c
typedef enum {
    CC_MSVC_X64,
    CC_SYSV_X64
} CallingConvention;

const char *int_param_regs_msvc[] = {"rcx", "rdx", "r8", "r9"};
const char *int_param_regs_sysv[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void emit_function_call(FILE *out, CallingConvention cc,
                        const char *func, Operand **args, int nargs) {
    const char **regs = (cc == CC_MSVC_X64) ? int_param_regs_msvc : int_param_regs_sysv;
    int max_reg_args = (cc == CC_MSVC_X64) ? 4 : 6;

    // Push stack arguments (right to left)
    for (int i = nargs - 1; i >= max_reg_args; i--) {
        emit_push(out, args[i]);
    }

    // Load register arguments
    for (int i = 0; i < nargs && i < max_reg_args; i++) {
        emit_mov(out, regs[i], args[i]);
    }

    // Shadow space for Windows
    if (cc == CC_MSVC_X64) {
        fprintf(out, "    sub rsp, 32\n");
    }

    fprintf(out, "    call %s\n", func);

    // Clean up
    if (cc == CC_MSVC_X64) {
        int cleanup = 32 + 8 * max(0, nargs - 4);
        fprintf(out, "    add rsp, %d\n", cleanup);
    } else if (nargs > 6) {
        fprintf(out, "    add rsp, %d\n", 8 * (nargs - 6));
    }
}
```

---

## Assembly Syntax

### MASM (Microsoft Macro Assembler) - Windows

```asm
; MASM syntax for Windows x64
.code

; Function: int factorial(int n)
factorial PROC
    push rbp
    mov rbp, rsp
    sub rsp, 16              ; Local variables

    mov DWORD PTR [rbp-4], ecx  ; Save n

    cmp ecx, 1
    jle base_case

    ; Recursive case: n * factorial(n-1)
    dec ecx
    sub rsp, 32              ; Shadow space
    call factorial
    add rsp, 32

    imul eax, DWORD PTR [rbp-4]
    jmp done

base_case:
    mov eax, 1

done:
    add rsp, 16
    pop rbp
    ret
factorial ENDP

END
```

### GAS (GNU Assembler) - Linux/macOS

```asm
# GAS syntax for System V x64
    .text
    .globl factorial

# Function: int factorial(int n)
factorial:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp           # Local variables

    movl %edi, -4(%rbp)      # Save n

    cmpl $1, %edi
    jle .base_case

    # Recursive case: n * factorial(n-1)
    decl %edi
    call factorial

    imull -4(%rbp), %eax
    jmp .done

.base_case:
    movl $1, %eax

.done:
    addq $16, %rsp
    popq %rbp
    ret
```

### NASM (Netwide Assembler) - Cross-platform

```asm
; NASM syntax (works on Windows/Linux)
bits 64

section .text
global factorial

; Function: int factorial(int n)
; Windows: n in ecx, Linux: n in edi
factorial:
    push rbp
    mov rbp, rsp
    sub rsp, 16

%ifdef WIN64
    mov dword [rbp-4], ecx
%else
    mov dword [rbp-4], edi
%endif

    cmp dword [rbp-4], 1
    jle .base_case

    mov eax, [rbp-4]
    dec eax
%ifdef WIN64
    mov ecx, eax
    sub rsp, 32
%else
    mov edi, eax
%endif
    call factorial
%ifdef WIN64
    add rsp, 32
%endif

    imul eax, dword [rbp-4]
    jmp .done

.base_case:
    mov eax, 1

.done:
    add rsp, 16
    pop rbp
    ret
```

### Syntax Comparison Table

| Feature | MASM | GAS | NASM |
|---------|------|-----|------|
| Register prefix | none | % | none |
| Immediate prefix | none | $ | none |
| Memory operand | [rbp-4] | -4(%rbp) | [rbp-4] |
| Operand order | Intel (dest, src) | AT&T (src, dest) | Intel |
| Size suffix | DWORD PTR | l/q suffix | dword |
| Comment | ; | # | ; |
| Label | name: or name PROC | name: | name: |
| Section | .code | .text | section .text |

---

## Symbol Naming and Decoration

### C Name Decoration

For C functions, names are generally undecorated on x64:

| Platform | C function `foo` | Exported as |
|----------|------------------|-------------|
| Windows x64 | `foo` | `foo` |
| Linux x64 | `foo` | `foo` |
| Windows x86 (cdecl) | `foo` | `_foo` |

### Exporting Symbols

**MASM:**
```asm
PUBLIC factorial       ; Export symbol

factorial PROC
    ; ...
factorial ENDP
```

**GAS:**
```asm
    .globl factorial   # Export symbol

factorial:
    # ...
```

### External References

**MASM:**
```asm
EXTERN printf:PROC     ; Import external function

    sub rsp, 32
    lea rcx, [message]
    call printf
    add rsp, 32
```

**GAS:**
```asm
    .extern printf     # Import external function

    leaq message(%rip), %rdi
    xorl %eax, %eax    # No floating point args
    call printf
```

---

## Runtime Library Linking

### Windows: Microsoft C Runtime (MSVCRT)

Link with the Visual C++ runtime:

```batch
REM Compile assembly to object file
ml64 /c program.asm

REM Link with C runtime
link program.obj /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup ^
     /DEFAULTLIB:libcmt.lib /DEFAULTLIB:kernel32.lib
```

**Required runtime libraries:**
- `libcmt.lib` - Static C runtime (release)
- `libcmtd.lib` - Static C runtime (debug)
- `msvcrt.lib` - Dynamic C runtime import library
- `kernel32.lib` - Windows kernel functions

**Startup code:**
Your `main` function is called by the CRT startup code (`mainCRTStartup`).
The CRT initializes:
- Global constructors (C++)
- Command line parsing (`argc`, `argv`)
- Environment variables
- Floating point unit
- Heap

### Linux: glibc

```bash
# Compile assembly to object file
as -o program.o program.s

# Link with C runtime
ld -o program program.o \
   -dynamic-linker /lib64/ld-linux-x86-64.so.2 \
   -lc \
   /usr/lib/x86_64-linux-gnu/crt1.o \
   /usr/lib/x86_64-linux-gnu/crti.o \
   /usr/lib/x86_64-linux-gnu/crtn.o
```

Or use GCC as the linker driver (easier):
```bash
gcc -o program program.o
```

### Minimal Programs (No Runtime)

For freestanding programs without C runtime:

**Windows:**
```asm
.code

mainCRTStartup PROC
    ; Your code here
    xor ecx, ecx           ; Exit code 0
    call ExitProcess
mainCRTStartup ENDP

EXTERN ExitProcess:PROC

END
```

```batch
ml64 /c minimal.asm
link minimal.obj /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup kernel32.lib
```

**Linux:**
```asm
    .text
    .globl _start

_start:
    # Your code here

    # Exit syscall
    movl $60, %eax         # sys_exit
    xorl %edi, %edi        # exit code 0
    syscall
```

```bash
as -o minimal.o minimal.s
ld -o minimal minimal.o
```

---

## Complete Examples

### Example 1: Hello World (Windows x64)

**hello.asm (MASM):**
```asm
.data
    message db "Hello, World!", 13, 10, 0

.code
    EXTERN printf:PROC
    EXTERN ExitProcess:PROC

main PROC
    push rbp
    mov rbp, rsp
    sub rsp, 32              ; Shadow space + alignment

    lea rcx, [message]
    call printf

    xor ecx, ecx
    call ExitProcess
main ENDP

END
```

**Build commands:**
```batch
ml64 /c hello.asm
link hello.obj /SUBSYSTEM:CONSOLE /DEFAULTLIB:legacy_stdio_definitions.lib ^
     /DEFAULTLIB:msvcrt.lib /DEFAULTLIB:kernel32.lib
```

### Example 2: Hello World (Linux x64)

**hello.s (GAS):**
```asm
    .data
message:
    .string "Hello, World!\n"

    .text
    .globl main

main:
    pushq %rbp
    movq %rsp, %rbp

    leaq message(%rip), %rdi
    xorl %eax, %eax          # No vector args
    call printf

    xorl %eax, %eax          # Return 0
    popq %rbp
    ret
```

**Build commands:**
```bash
as -o hello.o hello.s
gcc -o hello hello.o -no-pie
```

### Example 3: Platform-Agnostic Code Generator

```c
// codegen.c - Platform-independent code generator

#include <stdio.h>
#include <stdbool.h>

typedef enum { PLATFORM_WIN64, PLATFORM_LINUX64 } Platform;

typedef struct {
    FILE *out;
    Platform platform;
    int label_counter;
    int stack_offset;
} CodeGen;

void cg_init(CodeGen *cg, FILE *out, Platform platform) {
    cg->out = out;
    cg->platform = platform;
    cg->label_counter = 0;
    cg->stack_offset = 0;
}

void cg_emit_file_header(CodeGen *cg) {
    if (cg->platform == PLATFORM_WIN64) {
        fprintf(cg->out, "; Generated for Windows x64\n");
        fprintf(cg->out, ".code\n\n");
    } else {
        fprintf(cg->out, "# Generated for Linux x64\n");
        fprintf(cg->out, "    .text\n\n");
    }
}

void cg_emit_function_start(CodeGen *cg, const char *name) {
    if (cg->platform == PLATFORM_WIN64) {
        fprintf(cg->out, "PUBLIC %s\n", name);
        fprintf(cg->out, "%s PROC\n", name);
        fprintf(cg->out, "    push rbp\n");
        fprintf(cg->out, "    mov rbp, rsp\n");
    } else {
        fprintf(cg->out, "    .globl %s\n", name);
        fprintf(cg->out, "%s:\n", name);
        fprintf(cg->out, "    pushq %%rbp\n");
        fprintf(cg->out, "    movq %%rsp, %%rbp\n");
    }
}

void cg_emit_function_end(CodeGen *cg, const char *name) {
    if (cg->platform == PLATFORM_WIN64) {
        fprintf(cg->out, "    pop rbp\n");
        fprintf(cg->out, "    ret\n");
        fprintf(cg->out, "%s ENDP\n\n", name);
    } else {
        fprintf(cg->out, "    popq %%rbp\n");
        fprintf(cg->out, "    ret\n\n");
    }
}

void cg_emit_mov_reg_imm(CodeGen *cg, const char *reg, int value) {
    if (cg->platform == PLATFORM_WIN64) {
        fprintf(cg->out, "    mov %s, %d\n", reg, value);
    } else {
        fprintf(cg->out, "    movl $%d, %%%s\n", value, reg);
    }
}

void cg_emit_call(CodeGen *cg, const char *func) {
    if (cg->platform == PLATFORM_WIN64) {
        fprintf(cg->out, "    sub rsp, 32\n");  // Shadow space
        fprintf(cg->out, "    call %s\n", func);
        fprintf(cg->out, "    add rsp, 32\n");
    } else {
        fprintf(cg->out, "    call %s\n", func);
    }
}

void cg_emit_file_footer(CodeGen *cg) {
    if (cg->platform == PLATFORM_WIN64) {
        fprintf(cg->out, "END\n");
    }
}

// Example usage
int main() {
    FILE *out = fopen("output.asm", "w");
    CodeGen cg;

#ifdef _WIN32
    cg_init(&cg, out, PLATFORM_WIN64);
#else
    cg_init(&cg, out, PLATFORM_LINUX64);
#endif

    cg_emit_file_header(&cg);
    cg_emit_function_start(&cg, "main");
    cg_emit_mov_reg_imm(&cg, "eax", 42);
    cg_emit_function_end(&cg, "main");
    cg_emit_file_footer(&cg);

    fclose(out);
    return 0;
}
```

---

## Build Toolchain Integration

### Windows Build Script

**build.bat:**
```batch
@echo off
setlocal

REM Configuration
set COMPILER=cc.exe
set ASSEMBLER=ml64.exe
set LINKER=link.exe

REM Paths (adjust for your VS installation)
set VCPATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC
set KITPATH=C:\Program Files (x86)\Windows Kits\10

REM Set up environment
call "%VCPATH%\Auxiliary\Build\vcvars64.bat"

REM Compile C source to assembly
%COMPILER% -S %1.c -o %1.asm
if errorlevel 1 goto error

REM Assemble to object file
%ASSEMBLER% /c /Fo%1.obj %1.asm
if errorlevel 1 goto error

REM Link to executable
%LINKER% %1.obj /OUT:%1.exe /SUBSYSTEM:CONSOLE ^
    /DEFAULTLIB:libcmt.lib ^
    /DEFAULTLIB:kernel32.lib ^
    /DEFAULTLIB:legacy_stdio_definitions.lib
if errorlevel 1 goto error

echo Build successful: %1.exe
goto end

:error
echo Build failed!
exit /b 1

:end
endlocal
```

### Linux Build Script

**build.sh:**
```bash
#!/bin/bash

COMPILER=./cc
SOURCE=$1
BASENAME=${SOURCE%.c}

# Compile C source to assembly
$COMPILER -S $SOURCE -o $BASENAME.s
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

# Assemble and link using GCC
gcc -o $BASENAME $BASENAME.s -no-pie
if [ $? -ne 0 ]; then
    echo "Assembly/linking failed"
    exit 1
fi

echo "Build successful: $BASENAME"
```

### CMake Integration

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.16)
project(MyCompiler)

# Build the compiler
add_executable(cc
    src/main.c
    src/lexer.c
    src/parser.c
    src/codegen.c
)

# Custom command to compile a test file
add_custom_target(test_compile
    COMMAND $<TARGET_FILE:cc> ${CMAKE_SOURCE_DIR}/test/hello.c -o hello.asm
    DEPENDS cc
    COMMENT "Compiling test/hello.c"
)

if(WIN32)
    # Windows: use MASM
    add_custom_target(test_build
        COMMAND ml64 /c hello.asm
        COMMAND link hello.obj /SUBSYSTEM:CONSOLE
                /DEFAULTLIB:libcmt.lib /DEFAULTLIB:kernel32.lib
        DEPENDS test_compile
        COMMENT "Building test executable"
    )
else()
    # Linux: use GAS
    add_custom_target(test_build
        COMMAND as -o hello.o hello.s
        COMMAND gcc -o hello hello.o -no-pie
        DEPENDS test_compile
        COMMENT "Building test executable"
    )
endif()
```

---

## Summary

| Aspect | Windows x64 | Linux x64 |
|--------|-------------|-----------|
| Object format | COFF (.obj) | ELF (.o) |
| Assembler | ml64 (MASM) | as (GAS) |
| Linker | link.exe | ld or gcc |
| Syntax | Intel | AT&T |
| 1st int param | RCX | RDI |
| Shadow space | 32 bytes required | Not used |
| Red zone | Not available | 128 bytes |
| C runtime | MSVCRT | glibc |
| Entry point | mainCRTStartup | _start |

### Key Recommendations

1. **Use a portable assembler** (NASM) if targeting both platforms
2. **Generate platform-specific assembly** with conditional code
3. **Use GCC/Clang as linker driver** for simpler linking
4. **Test with both toolchains** during development
5. **Handle calling convention differences** in the code generator

### Quick Reference

**Windows build:**
```batch
ml64 /c program.asm
link program.obj /SUBSYSTEM:CONSOLE kernel32.lib msvcrt.lib
```

**Linux build:**
```bash
as -o program.o program.s
gcc -o program program.o
```
