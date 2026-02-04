# LispOS Project Configuration

## Project Overview
This project contains a Lisp/Scheme compiler that compiles to x86-64 MASM assembly.

## Directory Structure
- `LispCompiler/` - R7RS Scheme compiler implementation
- `LALRGen/` - LALR parser generator

## Build System
The project uses CMake with Visual Studio on Windows.

## Allowed Commands

The following commands are allowed for building and testing:

```
cmake -B build -S .
cmake --build build --config Release
cmake --build build --config Debug
msbuild /p:Configuration=Release
msbuild /p:Configuration=Debug
ctest --test-dir build
ml64.exe
link.exe
```

## Build Instructions

### Build the Lisp Compiler
```bash
cd LispCompiler
cmake -B build -S .
cmake --build build --config Release
```

### Run Tests
```bash
cd LispCompiler/build
ctest
```

### Compile and Assemble Scheme Files
```bash
# Generate assembly from Scheme source
LispCompiler/build/Release/lisp.exe <input.scm> -o <output.asm>

# Assemble with MASM
ml64.exe /c /Fo<output.obj> <output.asm>

# Link
link.exe /SUBSYSTEM:CONSOLE /OUT:<output.exe> <output.obj>
```
